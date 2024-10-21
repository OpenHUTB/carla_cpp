// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/streaming/detail/tcp/Server.h" // 包含服务器类的定义，该类负责TCP流的接收

#include <boost/asio/post.hpp> // 包含boost库中用于在io_context上安排函数执行的函数

#include "carla/Logging.h" // 包含carla库的日志记录功能，用于记录错误和其他信息

#include <memory> // 包含C++标准库中的智能指针和内存管理工具

namespace carla {
namespace streaming {
namespace detail {
namespace tcp {

  Server::Server(boost::asio::io_context &io_context, endpoint ep)
    : _io_context(io_context), // 初始化io_context，用于处理异步操作
      _acceptor(_io_context, std::move(ep)), // 初始化acceptor，绑定到提供的端点
      _timeout(time_duration::seconds(10u)), // 设置接受连接的超时时间为10秒
      _synchronous(false) {} // 初始化同步标志为false，表示异步操作

  void Server::OpenSession( // 定义一个函数，用于开启新的会话
      time_duration timeout, // 会话的超时时间
      ServerSession::callback_function_type on_opened, // 会话开启时的回调函数
      ServerSession::callback_function_type on_closed) { // 会话关闭时的回调函数
    using boost::system::error_code; // 使用boost库中的错误代码类型

    auto session = std::make_shared<ServerSession>(_io_context, timeout, *this); // 创建一个新的会话实例，与io_context和超时时间相关联

    auto handle_query = [on_opened, on_closed, session](const error_code &ec) { // 定义一个lambda函数，用于处理异步接受连接的结果
      if (!ec) {
        session->Open(std::move(on_opened), std::move(on_closed)); // 如果没有错误，使用提供的回调函数打开会话
      } else {
        log_error("tcp accept stream error:", ec.message()); // 如果有错误，记录错误信息
      }
    };

    _acceptor.async_accept(session->_socket, [=](error_code ec) { // 异步接受连接，当新的连接到达时，会调用提供的回调函数
      // 立即处理查询并打开一个新的会话
      boost::asio::post(_io_context, [=]() { handle_query(ec); }); // 在io_context上安排处理查询，确保在正确的线程上执行
      OpenSession(timeout, on_opened, on_closed);  // 递归调用OpenSession，以接受下一个连接
    });
  }

} // namespace tcp
} // namespace detail
} // namespace streaming
} // namespace carla
