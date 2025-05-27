// This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
// If a copy of the MPL was not distributed with this file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is a modified version of code from:
// https://github.com/AuthorityFX/afx-nuke-plugins
// Originally authored by Ryan P. Wilson, Authority FX, Inc.
//
// Modifications for DespillAP plugin:
// - Renamed variables for improved readability and consistency
// - Removed CUDA-specific sections
// - Adjusted to integrate with the DespillAP CPU-based plugin architecture

#include "include/threading.h"

#include <algorithm>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/thread.hpp>

namespace imgcore
{
  Threader::Threader() : running(false)
  {
    InitializeThreads(0);
  }

  Threader::Threader(unsigned int num_threads) : running(false)
  {
    InitializeThreads(num_threads);
  }

  Threader::~Threader()
  {
    StopAndJoin();
  }

  /* Handles adding new worker threads
  to the existing thread group (thread_pool),
  simply increases the number of threads */
  void Threader::AddThreads(unsigned int num_threads)
  {
    for(unsigned int t = 0; t < num_threads; ++t) {
      thread_pool.create_thread(boost::bind(&Threader::Worker, this));
    }
  }

  void Threader::InitializeThreads(unsigned int requested_threads)
  {
    // Check if threads are running
    if(running) {
      StopAndJoin();  // Stop and wait for current task to finish
    }

    // Create a work object
    // This object prevents the service from stopping when there are no pending tasks
    work_ptr.reset(new boost::asio::io_service::work(io_service));

    // If the service is stopped, restart it with .reset()
    if(io_service.stopped()) {
      io_service.reset();
    }

    // Mark the state as "running"
    running = true;

    // Get the number of available CPU cores/threads
    unsigned int available_threads = boost::thread::hardware_concurrency();

    // If not specified, use all available threads
    num_threads = requested_threads > 0
                      ? std::min<unsigned int>(requested_threads, available_threads)
                      : available_threads;

    // Start a loop to create threads
    for(unsigned int t = 0; t < num_threads; ++t) {
      // For each thread, use 'thread_pool.create_thread()' to add it to the pool
      // Each thread will execute the 'Worker()' method of the Threader class
      thread_pool.create_thread(boost::bind(&Threader::Worker, this));
    }
  }

  /* This function is executed by each thread created in the thread pool
  defines the behavior of threads and their lifecycle */
  void Threader::Worker()
  {
    // Keep the thread active while the 'running' variable is true
    while(running) {
      io_service.run();  // Execute pending tasks in the io_service

      // When 'run()' finishes, the thread notifies with this event
      // Useful for other threads to know that this thread has finished its task
      exited_run.notify_one();

      // Acquire an exclusive lock on the mutex
      // to avoid race conditions
      boost::unique_lock<boost::mutex> lock(mutex);

      // Check if the thread should continue running
      // If the service is stopped 'io_service.stopped()'
      while(running && io_service.stopped()) {
        // If the service is stopped, wait for new tasks to be added
        io_service_ready.wait(lock);
      }
    }
  }

  /* This function allows waiting for all pending tasks
  in the thread pool to complete */
  void Threader::Wait()
  {
    // Remove the work object that keeps the io_service active
    // When there is no work object, 'io_service.run()' returns
    // once all pending tasks are completed
    work_ptr.reset();

    // Create a temporary mutex and lock to use them
    // with the condition variable
    boost::mutex dummy_mutex;
    boost::unique_lock<boost::mutex> dummy_lock(dummy_mutex);

    // Wait until the service stops
    while(!io_service.stopped()) {
      // Used to suspend execution of the calling thread
      // until a worker thread notifies that it has finished its task
      // This step is crucial; the thread calling 'Wait()'
      // blocks until all threads have processed
      // their pending tasks
      exited_run.wait(dummy_lock);
    }

    // When all threads have finished their work
    // Check if the service should continue running
    if(running) {
      // If it should continue
      // Protect the service modification
      boost::lock_guard<boost::mutex> lock(mutex);
      // Create a new work object to keep the service active
      work_ptr.reset(new boost::asio::io_service::work(io_service));
      // Restart the service
      // to allow new tasks
      io_service.reset();
    }
    io_service_ready.notify_all();
  }

  /* Handles terminating all operations
  and freeing resources associated with threads */
  void Threader::StopAndJoin()
  {
    // Scope block
    // that controls the duration of the mutex
    // lock
    {
      // Protect modification of the 'running' variable
      boost::lock_guard<boost::mutex> lock(mutex);
      running = false;  // Tell all threads they should terminate
    }
    Wait();                  // Remove the work object
    thread_pool.join_all();  // Wait for all threads to finish
  }

  /* This function is the entry point for adding new tasks to the pool,
  allows adding work that will be executed by one of the threads.
  For any type of function that matches the specified signature*/
  void Threader::AddWork(boost::function<void()> function)
  {
    /* The post() method of Boost.Asio guarantees that the function will be executed 
    asynchronously by one of the threads running io_service.run()
    This operation is non-blocking: 
    the AddWork function returns immediately 
    without waiting for the task to be executed */
    io_service.post(function);
  }

  bool Threader::IsRunning() const
  {
    return running;
  }

  unsigned int Threader::Threads() const
  {
    return num_threads;
  }
}  // namespace imgcore