// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/MoveHandler.h"  // 包含处理移动的头文件
#include "carla/Time.h"         // 包含时间相关的头文件
#include "carla/rpc/Metadata.h" // 包含元数据相关的头文件
#include "carla/rpc/Response.h" // 包含响应相关的头文件

#include <boost/asio/io_context.hpp> // 包含Boost.Asio的io_context类
#include <boost/asio/post.hpp>        // 包含Boost.Asio的post函数

#include <rpc/server.h>               // 包含RPC服务器的头文件

#include <future>                     // 包含future库，用于异步编程

namespace carla {
namespace rpc {

  // ===========================================================================
  // -- Server -----------------------------------------------------------------
  // ===========================================================================

  /// 一个RPC服务器，可以将功能绑定为同步或异步运行。
  ///
  /// 使用`AsyncRun`启动工作线程，使用`SyncRunFor`在调用者线程中运行一段工作。
  ///
  /// 使用`BindAsync`绑定的函数将在工作线程中异步运行。使用`BindSync`绑定的函数将在
  /// `SyncRunFor`函数中运行。
  class Server {
  public:

    // 构造函数，接受变长参数
    template <typename... Args>
    explicit Server(Args &&... args);

    // 绑定一个同步函数
    template <typename FunctorT>
    void BindSync(const std::string &name, FunctorT &&functor);

    // 绑定一个异步函数
    template <typename FunctorT>
    void BindAsync(const std::string &name, FunctorT &&functor);

    // 异步运行指定数量的工作线程
    void AsyncRun(size_t worker_threads) {
      _server.async_run(worker_threads);
    }

    // 在当前线程中运行指定持续时间的同步工作
    void SyncRunFor(time_duration duration) {
      #ifdef LIBCARLA_INCLUDED_FROM_UE4
      #include <compiler/enable-ue4-macros.h> // 包含UE4宏
      TRACE_CPUPROFILER_EVENT_SCOPE_STR(__FUNCTION__); // CPU性能分析
      #include <compiler/disable-ue4-macros.h> // 禁用UE4宏
      #endif // LIBCARLA_INCLUDED_FROM_UE4
      _sync_io_context.reset(); // 重置同步IO上下文
      _sync_io_context.run_for(duration.to_chrono()); // 运行指定持续时间
    }

    /// @warning 不会停止游戏线程。
    void Stop() {
      _server.stop(); // 停止服务器
    }

  private:

    boost::asio::io_context _sync_io_context; // 同步IO上下文

    ::rpc::server _server; // RPC服务器实例
  };

  // ===========================================================================
  // -- Server implementation --------------------------------------------------
  // ===========================================================================

namespace detail {

  // 函数包装器的结构体模板
  template <typename T>
  struct FunctionWrapper : FunctionWrapper<decltype(&T::operator())> {};

  // 特化：用于成员函数
  template <typename C, typename R, typename... Args>
  struct FunctionWrapper<R (C::*)(Args...)> : FunctionWrapper<R (*)(Args...)> {};

  // 特化：用于常量成员函数
  template <typename C, typename R, typename... Args>
  struct FunctionWrapper<R (C::*)(Args...) const> : FunctionWrapper<R (*)(Args...)> {};

  // 处理引用类型
  template<class T>
  struct FunctionWrapper<T &> : public FunctionWrapper<T> {};

  // 处理右值引用类型
  template<class T>
  struct FunctionWrapper<T &&> : public FunctionWrapper<T> {};

  // 定义函数指针包装器
  template <typename R, typename... Args>
  struct FunctionWrapper<R (*)(Args...)> {

    /// 将@a functor包装为具有等效签名的函数类型。返回的包装函数。
    /// 当调用时，将@a functor放入io_context；如果客户端同步调用此方法，则等待
    /// 已发布任务完成，否则立即返回。
    ///
    /// 这样，无论从哪个线程调用包装函数，提供的@a functor始终在
    /// io_context的上下文中调用。即，我们可以使用io_context在特定线程（例如
    /// 游戏线程）上运行任务。
    template <typename FuncT>
    static auto WrapSyncCall(boost::asio::io_context &io, FuncT &&functor) {
      return [&io, functor=std::forward<FuncT>(functor)](Metadata metadata, Args... args) -> R {
        auto task = std::packaged_task<R()>([functor=std::move(functor), args...]() {
          return functor(args...); // 调用传入的可调用对象
        });
        if (metadata.IsResponseIgnored()) { // 如果响应被忽略
          // 发布任务并忽略结果。
          boost::asio::post(io, MoveHandler(task)); // 将任务发布到IO上下文
          return R(); // 返回默认构造的R
        } else {
          // 发布任务并等待结果。
          auto result = task.get_future(); // 获取未来对象以获取结果
          boost::asio::post(io, MoveHandler(task)); // 将任务发布到IO上下文
          return result.get(); // 等待并返回结果
        }
      };
    }


   /// 将@a functor包装为具有等效签名的函数类型，
/// 处理客户端发送的元数据。如果客户端异步调用此方法，结果将被忽略。
template <typename FuncT>
static auto WrapAsyncCall(FuncT &&functor) {
  return [functor=std::forward<FuncT>(functor)](::carla::rpc::Metadata metadata, Args... args) -> R {
    if (metadata.IsResponseIgnored()) { // 检查响应是否被忽略
      functor(args...); // 调用传入的可调用对象
      return R(); // 返回默认构造的R
    } else {
      return functor(args...); // 正常返回调用结果
    }
  };
}
};

} // namespace detail

// 构造函数，接受变长参数
template <typename ... Args>
inline Server::Server(Args && ... args)
  : _server(std::forward<Args>(args) ...) { // 初始化服务器
  _server.suppress_exceptions(true); // 抑制异常
}

// 绑定一个同步函数
template <typename FunctorT>
inline void Server::BindSync(const std::string &name, FunctorT &&functor) {
  using Wrapper = detail::FunctionWrapper<FunctorT>; // 使用函数包装器
  _server.bind(
      name,
      Wrapper::WrapSyncCall(_sync_io_context, std::forward<FunctorT>(functor))); // 绑定同步函数
}

// 绑定一个异步函数
template <typename FunctorT>
inline void Server::BindAsync(const std::string &name, FunctorT &&functor) {
  using Wrapper = detail::FunctionWrapper<FunctorT>; // 使用函数包装器
  _server.bind(
      name,
      Wrapper::WrapAsyncCall(std::forward<FunctorT>(functor))); // 绑定异步函数
}

} // namespace rpc
} // namespace carla
