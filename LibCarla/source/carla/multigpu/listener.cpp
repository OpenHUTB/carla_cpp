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

// Listener类负责监听网络连接，并管理会话
class Listener {
public:
  // 构造函数，初始化监听器并开始监听指定端口
  Listener(boost::asio::io_context &io_context, endpoint ep);

  // 析构函数，停止监听器并关闭相关资源
  ~Listener();

  // 停止监听器，关闭acceptor，并重置io_context
  void Stop();

  // 打开一个新的会话，设置超时时间和回调函数
  void OpenSession(
      time_duration timeout,
      callback_function_type on_opened,
      callback_function_type on_closed,
      callback_function_type_response on_response);

private:
  boost::asio::io_context &_io_context; // 引用io_context对象
  boost::asio::ip::tcp::acceptor _acceptor; // 用于接受连接的acceptor
  time_duration _timeout; // 超时时间设置
};

// Listener类的构造函数实现
Listener::Listener(boost::asio::io_context &io_context, endpoint ep)
  : _io_context(io_context),
    _acceptor(_io_context, std::move(ep)),
    _timeout(time_duration::seconds(1u)) {
  _acceptor.listen(); // 开始监听端口
}

// Listener类的析构函数实现
Listener::~Listener() {
  Stop();
}

// Listener类的Stop方法实现
void Listener::Stop() {
  _acceptor.cancel(); // 取消当前操作
  _acceptor.close(); // 关闭acceptor
  _io_context.stop(); // 停止io_context的事件处理
  _io_context.reset(); // 重置io_context到初始状态
}

// Listener类的OpenSession方法实现
void Listener::OpenSession(
    time_duration timeout,
    callback_function_type on_opened,
    callback_function_type on_closed,
    callback_function_type_response on_response) {
  using boost::system::error_code;

  // 创建Primary对象代表一个会话
  auto session = std::make_shared<Primary>(_io_context, timeout, *this);
  auto self = shared_from_this(); // 获取Listener的shared_ptr引用

  // 定义lambda函数处理accept操作的结果
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
    // 处理查询并立刻开启一个新会话。
    boost::asio::post(_io_context, [=]() { handle_query(ec); });
    OpenSession(timeout, on_opened, on_closed, on_response);
  });
}

} // namespace multigpu
} // namespace carla
