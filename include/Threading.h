#ifndef THREADING_H
#define THREADING_H

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/thread/thread.hpp>

#ifdef _WIN32
#ifdef _MSC_VER
#pragma warning(disable : 4251)
#endif
#define WINLIB_EXPORT __declspec(dllexport)
#else
#define WINLIB_EXPORT
#endif

namespace imgcore
{

  class WINLIB_EXPORT Threader
  {
   private:
    boost::asio::io_service io_service;  // manage threads asynchronously
    boost::scoped_ptr<boost::asio::io_service::work>
        work_ptr;                     // keep io_service busy
    boost::thread_group thread_pool;  // pool of threads
    unsigned int num_threads;

    boost::condition_variable exited_run;  // condition variable for thread exit
    boost::condition_variable
        io_service_ready;  // condition variable for io_service
    bool running;

    boost::mutex mutex;  // mutex for thread safety

    void Worker();

   public:
    Threader();
    explicit Threader(unsigned int num_threads);
    ~Threader();

    void AddThreads(unsigned int num_threads);
    void InitializeThreads(unsigned int requested_threads = 0);
    void Wait();
    void StopAndJoin();
    void AddWork(boost::function<void()> function);
    bool IsRunning() const;
    unsigned int Threads() const;
  };
}  // namespace imgcore

#endif  // THREADING_H