// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.
//这段代码定义了一个名为 ThreadPool 的类，它实现了基于 Boost.Asio 的线程池。ThreadPool 类的主要功能是通过 Boost.Asio 提供的 io_context 来管理任务的异步执行。
#pragma once

#include "carla/MoveHandler.h"   // 引入 MoveHandler，用于在 Boost.Asio 中包装任务
#include "carla/NonCopyable.h"   // 引入 NonCopyable 类，确保 ThreadPool 不可拷贝
#include "carla/ThreadGroup.h"   // 引入 ThreadGroup，用于管理工作线程
#include "carla/Time.h"          // 引入 Time 类，用于时间相关操作

#include <boost/asio/io_context.hpp>   // 引入 Boost.Asio 的 io_context 类，用于调度异步任务
#include <boost/asio/post.hpp>    // 引入 Boost.Asio 的 post 函数，用于将任务发布到 io_context

#include <future>    // 引入 future，用于支持异步任务的结果获取
#include <thread>    // 引入 thread，用于获取硬件并发线程数
#include <type_traits>   // 引入 type_traits，用于类型推断和 SFINAE

namespace carla {

  /// 基于Boost.Asio的上下文 
  class ThreadPool : private NonCopyable {
  public:

    ThreadPool() : _work_to_do(_io_context) {}

    /// 停止线程池ThreadPool并合并所有线程
    ~ThreadPool() {
      Stop();
    }

    // 返回底层的 Boost.Asio io_context 对象
    auto &io_context() {
      return _io_context;
    }
    // 模板函数，向线程池中发布一个任务
    // FunctorT 是可调用对象类型，ResultT 是该可调用对象的返回值类型
    template <typename FunctorT, typename ResultT = typename std::result_of<FunctorT()>::type>
    std::future<ResultT> Post(FunctorT &&functor) {
// 定义一个模板函数Post，它接受一个可调用对象FunctorT作为参数。
// ResultT是默认模板参数，通过std::result_of<FunctorT()>::type推导得到，表示FunctorT()调用的返回类型。
// 注意：C++17及以后版本推荐使用std::invoke_result<FunctorT()>代替std::result_of<FunctorT()>，因为std::result_of已被弃用。
      auto task = std::packaged_task<ResultT()>(std::forward<FunctorT>(functor)); 
  // 创建一个std::packaged_task对象，该对象封装了传入的可调用对象functor。
  // std::packaged_task<ResultT()>表示这是一个不接受参数并返回ResultT类型结果的任务。
      auto future = task.get_future();
  // 通过task.get_future()获取一个与task关联的std::future对象。
  // 这个future对象将用于获取task执行的结果。
      boost::asio::post(_io_context, carla::MoveHandler(task));
  // 使用boost::asio的post函数将task（封装在carla::MoveHandler中）异步地提交到_io_context中执行。
  // _io_context是一个事件循环，用于管理异步任务的执行。
  // carla::MoveHandler可能是一个适配器，用于确保task可以被boost::asio正确处理。
  // 注意：_io_context是Post函数外部定义的一个成员变量或全局变量，这里假设它已经被正确初始化。
      return future;
  // 返回与task关联的future对象，允许调用者在未来某个时间点获取执行结果。
    }

    // 启动线程以异步运行任务，可以指定线程数量
    // 如果没有指定数量，则使用硬件的并发线程数
    void AsyncRun(size_t worker_threads) {
      // 定义一个成员函数AsyncRun，它接受一个size_t类型的参数worker_threads。
      // 这个参数指定了要创建并用于异步运行Run方法的工作线程数量。
      _workers.CreateThreads(worker_threads, [this]() { Run(); });
   // 使用_workers成员变量（可能是一个管理线程池的对象）的CreateThreads方法来创建指定数量的工作线程。
  // CreateThreads方法接受两个参数：一个是线程数量，另一个是每个线程要执行的函数或可调用对象。
  // 在这里，使用了一个lambda表达式作为第二个参数，这个lambda表达式捕获了当前对象（通过this指针），并调用了Run方法。
    }
  // 注意：这个注释假设_workers是一个已经定义并初始化的成员变量，它提供了创建和管理线程的功能。
  // 同时，也假设Run是当前类的一个成员函数，它将被异步地在工作线程中执行。
  // 此外，由于使用了[this]捕获列表，这个lambda表达式能够访问当前对象的成员变量和方法。
  
  // 当CreateThreads方法被调用时，它会启动指定数量的线程，并且每个线程都会执行提供的lambda表达式，
  // 进而调用Run方法。这样，Run方法就会在多个线程中异步地执行。
   // 调用 AsyncRun 函数，不指定线程数量，默认使用硬件并发线程数
    void AsyncRun() {
 // 定义一个无参成员函数AsyncRun。
// 这个函数的目的是启动异步操作，但不需要调用者指定线程数量。
// 相反，它会使用std::thread::hardware_concurrency()来获取系统的硬件并发级别（通常是CPU核心的数量），
// 并以此作为线程数量来调用另一个AsyncRun重载版本。
      AsyncRun(std::thread::hardware_concurrency());
    }  
  // 调用重载的AsyncRun函数，传入std::thread::hardware_concurrency()作为线程数量参数。
  // std::thread::hardware_concurrency()返回一个unsigned int值，表示系统建议的并发线程数。
  // 这个值通常等于CPU的物理核心数，但在某些情况下可能不同（例如，在具有超线程技术的CPU上）。
 // 注意：这里的注释假设已经定义了一个接受size_t类型参数的AsyncRun重载版本，
  // 并且该版本函数会根据提供的线程数量来异步执行某些操作。
  // 此外，还假设std::thread::hardware_concurrency()的返回值是合理的，
  // 即不为0，因为传入0可能导致未定义的行为（取决于AsyncRun重载版本的具体实现）。
  
  // 在实际应用中，使用std::thread::hardware_concurrency()作为线程数量是一种常见做法，
  // 因为它可以根据系统的硬件能力来优化并行任务的执行。
  // 然而，开发者也应该注意，这个值只是一个建议，并且可能不适合所有情况。
  // 在某些情况下，可能需要手动调整线程数量以达到最佳性能。
  // 在当前线程中运行任务，会阻塞当前线程直到线程池停止
    void Run() {
// 定义一个成员函数Run，它不接受任何参数。
// 这个函数的目的是启动_io_context的事件循环或I/O服务，并让它一直运行，直到它被显式停止（例如，通过调用某个停止方法）或所有已提交的任务都已完成。
      _io_context.run();
  // 调用_io_context的run方法。
  // run方法会启动事件循环，允许异步任务在这个循环中执行。
  // 一旦run方法被调用，它将一直阻塞当前线程，直到事件循环被显式停止或所有任务都已完成。
    }
  // 注意：这里的注释基于一些合理的假设，因为_io_context的具体类型没有在代码片段中给出。
  // 在实际代码中，_io_context可能是一个封装了事件循环或I/O服务功能的对象，它提供了运行、停止和管理异步任务的能力。
  // 例如，在Boost.Asio库中，_io_context可能是一个boost::asio::io_context对象，它的run方法用于启动异步操作的事件循环。
  
  // 当run方法被调用时，它会启动_io_context的事件循环，并允许异步任务（如网络I/O操作、定时器回调等）在这个循环中执行。
  // 事件循环会一直运行，直到它被显式停止（例如，通过调用_io_context的stop方法）或所有已提交的任务都已完成。
  // 这使得Run函数成为启动和管理异步操作的关键部分。

  // 在当前线程中运行任务一段时间，如果在指定时间内没有停止，则退出
    void RunFor(time_duration duration) {
// 定义一个成员函数RunFor，它接受一个time_duration类型的参数duration。
// 这个参数指定了_io_context应该运行的时间长度。
// 假设time_duration是一个封装了时间长度信息的自定义类型，它提供了to_chrono()方法以转换为标准库的时间点或时间间隔类型。
      _io_context.run_for(duration.to_chrono());
   // 调用_io_context的run_for方法，并传入duration.to_chrono()作为参数。
  // run_for方法会让_io_context运行，直到指定的时间间隔结束或所有任务都已完成。
  // duration.to_chrono()应该返回一个std::chrono库中的时间间隔类型，如std::chrono::milliseconds、std::chrono::seconds等。
    }
  // 注意：这里的注释基于一些合理的假设，因为time_duration和_io_context的具体类型没有在代码片段中给出。
  // 在实际代码中，_io_context可能是一个封装了事件循环或I/O服务功能的对象，它提供了运行、停止和管理异步任务的能力。
  // time_duration可能是一个自定义类型，用于表示时间长度，它应该提供了与std::chrono库互操作的能力。
  
  // 当run_for方法被调用时，它会启动_io_context的事件循环，并允许异步任务在这个循环中执行。
  // 事件循环会一直运行，直到指定的时间间隔结束，或者所有已提交的任务都已完成（以先到者为准）。
  // 这可以用于限制异步操作的最大执行时间，或者在需要定期执行某些操作时提供一个时间窗口。

  // 停止线程池并合并所有线程
    void Stop() {
    // 构造函数可能在这里被省略了，但它应该初始化_io_context、_work_to_do和_workers成员。
    // 特别是，_work_to_do可能需要与_io_context关联，以保持事件循环的运行，
    // 而_workers可能需要启动一定数量的工作线程。
 
    // 停止异步I/O操作并等待所有工作线程完成。
      _io_context.stop();
        // 调用_io_context的stop方法，请求停止事件循环。
        // 这通常会导致run方法返回，但已经提交的任务可能仍然会执行完成。
      _workers.JoinAll();
        // 调用_workers的JoinAll方法，等待所有工作线程完成。
        // 这确保了所有线程在Stop方法返回之前都已经结束执行。
        // 注意：JoinAll方法的具体实现和行为取决于ThreadGroup类的定义。
    }
        // 可能还有其他成员函数，如启动异步操作、添加任务等，但在这里没有给出。

  private:

    boost::asio::io_context _io_context;
// Boost.Asio库中的io_context对象，用于管理异步I/O操作和事件循环。
    boost::asio::io_context::work _work_to_do;
// 与_io_context关联的工作对象，用于保持事件循环的运行。
// 当_work_to_do存在时，即使没有其他显式任务，_io_context的run方法也会继续运行。
// 这通常用于确保在异步操作完成之前事件循环不会退出。
    ThreadGroup _workers;
    // 自定义的ThreadGroup类型，用于管理一组工作线程。
    // 这些线程可能用于执行与_io_context关联的异步任务。
    // ThreadGroup的具体实现和行为取决于其定义，但通常它会提供启动、停止和等待线程完成的方法。
  };
// 注意：这里的代码片段是一个类的部分定义，并且假设了一些外部类型和成员函数的存在。
// 特别是，ThreadGroup类型没有在代码片段中定义，因此它的具体实现和行为是未知的。
// 同样，构造函数和其他可能的成员函数也没有在代码片段中给出。

} // namespace carla
