// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/multigpu/listener.h"
#include "carla/multigpu/primary.h"

#include <boost/asio/post.hpp>

#include "carla/Logging.h"

#include <memory>

namespace carla {
namespace multigpu {
// Listener类的构造函数  
  // 初始化监听器，绑定到指定的io_context和endpoint上，并设置超时时间
  Listener::Listener(boost::asio::io_context &io_context, endpoint ep)
    : _io_context(io_context),
      _acceptor(_io_context, std::move(ep)),
      _timeout(time_duration::seconds(1u)) {
        _acceptor.listen();// 开始监听端口
      }
  // Listener类的析构函数  
  // 停止监听器，关闭acceptor，并停止io_context
  Listener::~Listener() {
    Stop();
  }
  // 停止监听器  
  // 取消并关闭acceptor，停止并重置io_context
  void Listener::Stop() {
    _acceptor.cancel();// 取消当前操作
    _acceptor.close();// 关闭acceptor  
    _io_context.stop();// 停止io_context的事件处理
    _io_context.reset();// 重置io_context到初始状态
  }
  // 打开一个新的会话  
  // 设置超时时间和回调函数，当会话建立、关闭或收到响应时调用
  void Listener::OpenSession(
      time_duration timeout,// 会话的超时时间
      callback_function_type on_opened,// 会话建立时调用的回调函数
      callback_function_type on_closed,// 会话关闭时调用的回调函数 
      callback_function_type_response on_response) {// 收到响应时调用的回调函数

    using boost::system::error_code;
   // 创建一个Primary对象，它代表一个会话
    auto session = std::make_shared<Primary>(_io_context, timeout, *this);
    auto self = shared_from_this();// 获取Listener的shared_ptr，以便在lambda中使用  
    // 定义一个lambda来处理accept操作的结果 
    auto handle_query = [on_opened, on_closed, on_response, session, self](const error_code &ec) {
    if (!ec) {
      // 如果accept成功，打开会话并设置回调函数 
      session->Open(std::move(on_opened), std::move(on_closed), std::move(on_response));
    } else {
      // 如果失败，记录错误信息 
      log_error("Secondary server:", ec.message());
    }
  };

    _acceptor.async_accept(session->_socket, [=](error_code ec) {
      // 处理查询并立刻开启一个新会话
      boost::asio::post(_io_context, [=]() { handle_query(ec); });
      OpenSession(timeout, on_opened, on_closed, on_response);
    });
  }

} // namespace multigpu
} // namespace carla
