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
      auto task = std::packaged_task<ResultT()>(std::forward<FunctorT>(functor));
      auto future = task.get_future();
      boost::asio::post(_io_context, carla::MoveHandler(task));
      return future;
    }

    // 启动线程以异步运行任务，可以指定线程数量
    // 如果没有指定数量，则使用硬件的并发线程数
    void AsyncRun(size_t worker_threads) {
      _workers.CreateThreads(worker_threads, [this]() { Run(); });
    }

    // 调用 AsyncRun 函数，不指定线程数量，默认使用硬件并发线程数
    void AsyncRun() {
      AsyncRun(std::thread::hardware_concurrency());
    }

    // 在当前线程中运行任务，会阻塞当前线程直到线程池停止
    void Run() {
      _io_context.run();
    }

    // 在当前线程中运行任务一段时间，如果在指定时间内没有停止，则退出
    void RunFor(time_duration duration) {
      _io_context.run_for(duration.to_chrono());
    }

    // 停止线程池并合并所有线程
    void Stop() {
      _io_context.stop();
      _workers.JoinAll();
    }

  private:

    boost::asio::io_context _io_context;

    boost::asio::io_context::work _work_to_do;

    ThreadGroup _workers;
  };

} // namespace carla
