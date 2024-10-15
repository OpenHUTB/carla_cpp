// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/NonCopyable.h" // 引入carla命名空间下的NonCopyable类，该类用于防止对象被复制
#include "carla/Time.h"
#include "carla/streaming/detail/tcp/ServerSession.h"

#include <boost/asio/io_context.hpp> // 引入Boost库的asio模块中的io_context类，用于事件处理和I/O操作
#include <boost/asio/ip/tcp.hpp> // 引入Boost库的asio模块中的ip/tcp协议支持类
#include <boost/asio/post.hpp> // 引入Boost库的asio模块中的post函数，用于在io_context上安排函数执行

#include <atomic> // 引入C++标准库中的原子操作模板，用于线程安全的共享变量操作

namespace carla {
namespace streaming {
namespace detail {
namespace tcp {

  ///警告：在io_context停止之前，不能销毁这个服务器实例
  class Server : private NonCopyable {
  public:
    // 使用using声明简化类型名称，方便后续代码中使用
    using endpoint = boost::asio::ip::tcp::endpoint;
    using protocol_type = endpoint::protocol_type;

    explicit Server(boost::asio::io_context &io_context, endpoint ep); // 构造函数，接受一个io_context引用和一个endpoint作为参数

    endpoint GetLocalEndpoint() const { // 获取服务器监听的本地端点
      return _acceptor.local_endpoint();
    }

    /// 设置会话超时时间，仅对新创建的会话有效，默认为10秒
    ///
    void SetTimeout(time_duration timeout) {
      _timeout = timeout;
    }

    // 开始监听连接，为每个新连接设置打开和关闭时的回调函数
    //
    template <typename FunctorT1, typename FunctorT2>
    void Listen(FunctorT1 on_session_opened, FunctorT2 on_session_closed) {
      boost::asio::post(_io_context, [=]() {
        OpenSession(
            _timeout,
            std::move(on_session_opened),
            std::move(on_session_closed));
      });
    }

    void SetSynchronousMode(bool is_synchro) { // 设置服务器是否运行在同步模式
      _synchronous = is_synchro;
    }

    bool IsSynchronousMode() const {  // 获取服务器是否运行在同步模式
      return _synchronous;
    }

  private:

    void OpenSession( // 私有方法，用于打开新的会话
        time_duration timeout,
        ServerSession::callback_function_type on_session_opened,
        ServerSession::callback_function_type on_session_closed);

    boost::asio::io_context &_io_context; // I/O上下文引用，用于事件处理和I/O操作

    boost::asio::ip::tcp::acceptor _acceptor; // TCP协议的acceptor，用于接受客户端连接

    std::atomic<time_duration> _timeout; // 原子操作的超时时间，用于线程安全的超时时间设置

    bool _synchronous; // 布尔值，表示服务器是否运行在同步模式
  };

} // namespace tcp
} // namespace detail
} // namespace streaming
} // namespace carla
