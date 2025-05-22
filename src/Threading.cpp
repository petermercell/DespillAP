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

  /* Se encarga de añadir nuevos hilos de trabajo
  al grupo de hilos existente (thread_pool),
  simplemente aumenta la cantidad de hilos */
  void Threader::AddThreads(unsigned int num_threads)
  {
    for(unsigned int t = 0; t < num_threads; ++t) {
      thread_pool.create_thread(boost::bind(&Threader::Worker, this));
    }
  }

  void Threader::InitializeThreads(unsigned int requested_threads)
  {
    // Verifica si hay hilos en ejecución
    if(running) {
      StopAndJoin();  // Se detiene y espera a que termine la tarea actual
    }

    // Crea un objeto work
    // Este objeto impide que el servicio se detenga cuando no hay tareas pendientes
    work_ptr.reset(new boost::asio::io_service::work(io_service));

    // Si el servicio está detenido, lo reinicia con .reset()
    if(io_service.stopped()) {
      io_service.reset();
    }

    // Se marca el estado en "ejecucion"
    running = true;

    // obtiene el numero de nucleos/hilos disponibles del CPU
    unsigned int available_threads = boost::thread::hardware_concurrency();

    // si no, se utiliza todos los hilos disponibles
    num_threads =
        requested_threads > 0
            ? std::min<unsigned int>(requested_threads, available_threads)
            : available_threads;

    // inicia un bucle para crear hilos
    for(unsigned int t = 0; t < num_threads; ++t) {
      // para cada hilo, utiliza 'thread_pool.create_thread()' para añadirlo a la pool
      // cada hilo ejecutará el método 'Worker()' de la clase Threader
      thread_pool.create_thread(boost::bind(&Threader::Worker, this));
    }
  }

  /* Esta funcion es la que ejecuta cada hilo creado en el pool de hilos
  define el comportamiento de los hilos y su ciclo de vida */
  void Threader::Worker()
  {
    // Mantiene el hilo activo mientras la variable 'running' sea verdadera
    while(running) {
      io_service.run();  // ejecutará tareas pendientes en el io_service

      // Cuando 'run()' termina, el hilo notifica con este evento
      // Util para que los otros hilos sepan que el hilo ha terminado su tarea
      exited_run.notify_one();

      // Adquiere un bloqueo exclusivo del mutex
      // para evitar condiciones de carrera
      boost::unique_lock<boost::mutex> lock(mutex);

      // Verifica si el hilo deberia seguir ejecutandose
      // Si el servicio esta detendido 'io_service.stoppped()'
      while(running && io_service.stopped()) {
        // Si el servicio está detenido, espera a que se añadan nuevas tareas
        io_service_ready.wait(lock);
      }
    }
  }

  /* Esta funcion permite esperar a que se completen todas las tareas
  pendientes en el pool de hilos */
  void Threader::Wait()
  {
    // Elimina el objeto work que mantiene el io_service activo
    // Cuando no hay un objeto work, 'io_service.run()' retorna
    // una vez que se completen todas las tareas pendientes
    work_ptr.reset();

    // Crea un mutex temporal y un bloqueo para utilizarlos
    // con la variable de condicion
    boost::mutex dummy_mutex;
    boost::unique_lock<boost::mutex> dummy_lock(dummy_mutex);

    // Espera hasta que el servicio se detenga
    while(!io_service.stopped()) {
      // Utiliza para suspender la ejecucion del hilo llamante
      // hasta que un hilo trabajador notifique que ha terminado su tarea
      // Este paso es crucial; el hilo que llama a 'Wait()'
      // se bloque hasta que todos los hilos hayan procesado
      // sus tareas pendientes
      exited_run.wait(dummy_lock);
    }

    // Cuando todos los hilos han terminado su trabajo
    // Verifica si el servicio debe de seguir ejecutandose
    if(running) {
      // Si debe continuar
      // Protege la modificacion del servicio
      boost::lock_guard<boost::mutex> lock(mutex);
      // Crea un nuevo objeto work para mantener el servicio activo
      work_ptr.reset(new boost::asio::io_service::work(io_service));
      // Reinicia el servicio
      // Para permitir nuevas tareas
      io_service.reset();
    }
    io_service_ready.notify_all();
  }

  /* Se encarga de finalizar todas las operaciones
  y liberar los recursos asociados a los hilos */
  void Threader::StopAndJoin()
  {
    // bloque de ambito
    // que controla la duracion del bloqueo
    // del mutex
    {
      // protege la modificacion de la variable 'running'
      boost::lock_guard<boost::mutex> lock(mutex);
      running = false;  // indica a todos los hilos que deben de terminar
    }
    Wait();                  // elimina el objeto work
    thread_pool.join_all();  // espera a que todos los hilos terminen
  }

  /* Esta funcion es el punto de entrada para agregar nuevas tareas al pool,
  permite añadir trabajo que será ejecutado por alguno de los hilos.
  Para cualquier tipo de funcion que cumpla con la firma especificada*/
  void Threader::AddWork(boost::function<void()> function)
  {
    /* El método post() de Boost.Asio garantiza que la función será ejecutada 
    de forma asíncrona por uno de los hilos que estén ejecutando io_service.run()
    Esta operación es no bloqueante: 
    la función AddWork retorna inmediatamente 
    sin esperar a que la tarea sea ejecutada */
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