// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/streaming/detail/tcp/ServerSession.h"
#include "carla/streaming/detail/tcp/Server.h"

#include "carla/Debug.h"
#include "carla/Logging.h"

#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/bind_executor.hpp>
#include <boost/asio/post.hpp>

#include <atomic>
#include <thread>

namespace carla {
namespace streaming {
namespace detail {
namespace tcp {
// 用于统计服务器会话的数量
  static std::atomic_size_t SESSION_COUNTER{0u};
// ServerSession类的构造函数
  // @param io_context boost::asio的I/O上下文对象
  // @param timeout 会话超时时间
  // @param server 所属的服务器对象
  ServerSession::ServerSession(
      boost::asio::io_context &io_context,
      const time_duration timeout,
      Server &server)
    : LIBCARLA_INITIALIZE_LIFETIME_PROFILER(
          std::string("tcp server session ") + std::to_string(SESSION_COUNTER)),
      _server(server),
      _session_id(SESSION_COUNTER++),
      _socket(io_context),
      _timeout(timeout),
      _deadline(io_context),
      _strand(io_context) {}
// 打开会话的函数
  // @param on_opened 会话打开成功的回调函数
  // @param on_closed 会话关闭的回调函数
  void ServerSession::Open(
      callback_function_type on_opened,
      callback_function_type on_closed) {
      	// 断言回调函数不为空
    DEBUG_ASSERT(on_opened && on_closed);
    _on_closed = std::move(on_closed);

    // 强制不使用Nagle(内格尔)算法。
    // 将Linux上的同步模式速度提高了约3倍。
    const boost::asio::ip::tcp::no_delay option(true);
    _socket.set_option(option);
 // 启动定时器
    StartTimer();
    // 获取自身的共享指针，以便在异步操作中保持对象存活
    auto self = shared_from_this(); // 为了让自己存活下去。
    boost::asio::post(_strand, [=]() {
 // 定义处理查询的内部函数
      auto handle_query = [this, self, callback=std::move(on_opened)](
          const boost::system::error_code &ec,
          size_t DEBUG_ONLY(bytes_received)) {
        if (!ec) {
        	// 断言接收到的字节数等于流ID的大小
          DEBUG_ASSERT_EQ(bytes_received, sizeof(_stream_id));
          // 打印调试信息，表示会话已启动
          log_debug("session", _session_id, "for stream", _stream_id, " started");
          // 在strand的上下文环境中执行回调函数
          boost::asio::post(_strand.context(), [=]() { callback(self); });
        } else {
        	// 打印错误信息，表示获取流ID时出错
          log_error("session", _session_id, ": error retrieving stream id :", ec.message());
          // 立即关闭会话
          CloseNow(ec);
        }
      };

      // 读取流id。
      _deadline.expires_from_now(_timeout);
      // 异步读取流ID
      boost::asio::async_read(
          _socket,
          boost::asio::buffer(&_stream_id, sizeof(_stream_id)),
          boost::asio::bind_executor(_strand, handle_query));
    });
  }
// 向客户端写入消息的函数
  // @param message 要写入的消息指针
  void ServerSession::Write(std::shared_ptr<const Message> message) {
  	// 断言消息不为空且消息内容不为空
    DEBUG_ASSERT(message != nullptr);
    DEBUG_ASSERT(!message->empty());
    auto self = shared_from_this();
      if (!_socket.is_open()) {
        return;
      }
      if (_is_writing) {
        if (_server.IsSynchronousMode()) {
          // 等待上一条消息发送完毕
          while (_is_writing) {
            std::this_thread::yield();
          }
        } else {
          // 忽略该消息
          log_debug("session", _session_id, ": connection too slow: message discarded");
          return;
        }
      }
      _is_writing = true;
// 定义消息发送完成后的回调函数
      auto handle_sent = [this, self, message](const boost::system::error_code &ec, size_t DEBUG_ONLY(bytes)) {
        _is_writing = false;
        if (ec) {
        	// 如果发送出错，打印错误信息并立即关闭会话
          log_info("session", _session_id, ": error sending data :", ec.message());
          CloseNow(ec);
        } else {
        	// 如果发送成功，打印调试信息（可选）并断言发送的字节数正确
          DEBUG_ONLY(log_debug("session", _session_id, ": successfully sent", bytes, "bytes"));
          DEBUG_ASSERT_EQ(bytes, sizeof(message_size_type) + message->size());
        }
      };
// 打印调试信息，表示要发送的消息大小
      log_debug("session", _session_id, ": sending message of", message->size(), "bytes");
// 设置消息发送的截止时间
      _deadline.expires_from_now(_timeout);
      // 异步写入消息
      boost::asio::async_write(_socket, message->GetBufferSequence(), 
        boost::asio::bind_executor(_strand, handle_sent));
  }
// 关闭会话的函数
  void ServerSession::Close() {
    boost::asio::post(_strand, [self=shared_from_this()]() { self->CloseNow(); });
  }
 // 启动定时器的函数，如果定时器已过期则关闭会话，否则设置异步等待定时器到期的回调函数
  void ServerSession::StartTimer() {
    if (_deadline.expires_at() <= boost::asio::deadline_timer::traits_type::now()) {
      log_debug("session", _session_id, "timed out");
      Close();
    } else {
      _deadline.async_wait([this, self=shared_from_this()](boost::system::error_code ec) {
        if (!ec) {
          StartTimer();
        } else {
          log_debug("session", _session_id, "timed out error:", ec.message());
        }
      });
    }
  }
// 立即关闭会话的函数，取消定时器，关闭套接字并执行关闭回调函数
  void ServerSession::CloseNow(boost::system::error_code ec) {
    _deadline.cancel();
    if (!ec)
    {
      if (_socket.is_open()) {
        boost::system::error_code ec2;
        _socket.shutdown(boost::asio::socket_base::shutdown_both, ec2);
        _socket.close();
      }
    }
    _on_closed(shared_from_this());
    log_debug("session", _session_id, "closed");
  }

} // namespace tcp
} // namespace detail
} // namespace streaming
} // namespace carla
