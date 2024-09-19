// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/MoveHandler.h"
#include "carla/NonCopyable.h"
#include "carla/ThreadGroup.h"
#include "carla/Time.h"

#include <boost/asio/io_context.hpp>
#include <boost/asio/post.hpp>

#include <future>
#include <thread>
#include <type_traits>

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
