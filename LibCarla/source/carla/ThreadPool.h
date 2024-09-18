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

    /// 返回底层 io_context�� 
    auto &io_context() {
      return _io_context;
    }

    /// 向Pool发布一个任务 
    template <typename FunctorT, typename ResultT = typename std::result_of<FunctorT()>::type>
    std::future<ResultT> Post(FunctorT &&functor) {
      auto task = std::packaged_task<ResultT()>(std::forward<FunctorT>(functor));
      auto future = task.get_future();
      boost::asio::post(_io_context, carla::MoveHandler(task));
      return future;
    }

    /// Launch threads to run tasks asynchronously. Launch specific number of
    /// threads if @a worker_threads is provided, otherwise use all available
    /// hardware concurrency.
    void AsyncRun(size_t worker_threads) {
      _workers.CreateThreads(worker_threads, [this]() { Run(); });
    }

    /// @copydoc AsyncRun(size_t)
    void AsyncRun() {
      AsyncRun(std::thread::hardware_concurrency());
    }

    /// Run tasks in this thread.
    ///
    /// @warning This function blocks until the ThreadPool has been stopped.
    void Run() {
      _io_context.run();
    }

    /// Run tasks in this thread for an specific @a duration.
    ///
    /// @warning This function blocks until the ThreadPool has been stopped, or
    /// until the specified time duration has elapsed.
    void RunFor(time_duration duration) {
      _io_context.run_for(duration.to_chrono());
    }

    /// 停止线程池ThreadPool并合并所有线程
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
