// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// 主服务器
// 
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/multigpu/primary.h"

#include "carla/Debug.h"
#include "carla/Logging.h"
#include "carla/multigpu/incomingMessage.h"
#include "carla/multigpu/listener.h"

#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/bind_executor.hpp>
#include <boost/asio/post.hpp>

#include <atomic>
#include <thread>

namespace carla {
namespace multigpu {

  static std::atomic_size_t SESSION_COUNTER{0u};

  Primary::Primary(
      boost::asio::io_context &io_context,
      const time_duration timeout,
      Listener &server)
    : LIBCARLA_INITIALIZE_LIFETIME_PROFILER(
          std::string("tcp multigpu server session ") + std::to_string(SESSION_COUNTER)),
      _server(server),
      _session_id(SESSION_COUNTER++),
      _socket(io_context),
      _timeout(timeout),
      _deadline(io_context),
      _strand(io_context),
      _buffer_pool(std::make_shared<BufferPool>()) {}

  Primary::~Primary() {
    if (_socket.is_open()) {
      boost::system::error_code ec;
      _socket.shutdown(boost::asio::socket_base::shutdown_both, ec);
      _socket.close();
    }
  }

  /// 启动会话并在成功读取流 id 后调用 @a on_opened，会话关闭后调用 @a on_closed。
  void Primary::Open(
      Listener::callback_function_type on_opened,
      Listener::callback_function_type on_closed,
      Listener::callback_function_type_response on_response) {
    DEBUG_ASSERT(on_opened && on_closed);

    // 这强制不使用 Nagle 算法。将 Linux 上的同步模式速度提高了约 3 倍。
    const boost::asio::ip::tcp::no_delay option(true);
    _socket.set_option(option);

    // 回调
    _on_closed = std::move(on_closed);
    _on_response = std::move(on_response);
    on_opened(shared_from_this());

    ReadData();
  }

  /// 将一些数据写入套接字。
  void Primary::Write(std::shared_ptr<const carla::streaming::detail::tcp::Message> message) {
    DEBUG_ASSERT(message != nullptr);
    DEBUG_ASSERT(!message->empty());
    std::weak_ptr<Primary> weak = shared_from_this();
    boost::asio::post(_strand, [=]() {
      auto self = weak.lock();
      if (!self) return;
      if (!self->_socket.is_open()) {
        return;
      }

      auto handle_sent = [weak, message](const boost::system::error_code &ec, size_t DEBUG_ONLY(bytes)) {
        auto self = weak.lock();
        if (!self) return;
        if (ec) {
          log_error("session ", self->_session_id, ": error sending data: ", ec.message());
          self->CloseNow(ec);
        } else {
          // DEBUG_ASSERT_EQ(bytes, sizeof(message_size_type) + message->size());
        }
      };

      self->_deadline.expires_from_now(self->_timeout);
      boost::asio::async_write(
          self->_socket,
          message->GetBufferSequence(),
          boost::asio::bind_executor(self->_strand, handle_sent));
    });
  }

  void Primary::Write(std::string text) {
    std::weak_ptr<Primary> weak = shared_from_this();
    boost::asio::post(_strand, [=]() {
      auto self = weak.lock();
      if (!self) return;
      if (!self->_socket.is_open()) {
        return;
      }

      // 发送的第一个大小缓冲区
      self->_deadline.expires_from_now(self->_timeout);
      int this_size = text.size();
      boost::asio::async_write(
          self->_socket,
          boost::asio::buffer(&this_size, sizeof(this_size)),
          boost::asio::bind_executor(self->_strand, [](const boost::system::error_code &, size_t){ }));
      // send characters
      boost::asio::async_write(
          self->_socket,
          boost::asio::buffer(text.c_str(), text.size()),
          boost::asio::bind_executor(self->_strand, [](const boost::system::error_code &, size_t){ }));
    });
  }

  void Primary::ReadData() {
    std::weak_ptr<Primary> weak = shared_from_this();
    boost::asio::post(_strand, [weak]() {
      auto self = weak.lock();
      if (!self) return;

      auto message = std::make_shared<IncomingMessage>(self->_buffer_pool->Pop());

      auto handle_read_data = [weak, message](boost::system::error_code ec, size_t DEBUG_ONLY(bytes)) {
        auto self = weak.lock();
        if (!self) return;
        if (!ec) {
          DEBUG_ASSERT_EQ(bytes, message->size());
          DEBUG_ASSERT_NE(bytes, 0u);
          // Move the buffer to the callback function and start reading the next
          // piece of data.
          self->_on_response(self, message->pop());
          std::cout << "Getting data on listener\n";
          self->ReadData();
        } else {
          // As usual, if anything fails start over from the very top.
          log_error("primary server: failed to read data: ", ec.message());
        }
      };

      auto handle_read_header = [weak, message, handle_read_data](
          boost::system::error_code ec,
          size_t DEBUG_ONLY(bytes)) {
        auto self = weak.lock();
        if (!self) return;
        if (!ec && (message->size() > 0u)) {
          // Now that we know the size of the coming buffer, we can allocate our
          // buffer and start putting data into it.
          boost::asio::async_read(
              self->_socket,
              message->buffer(),
              boost::asio::bind_executor(self->_strand, handle_read_data));
        } else {
          if (ec) {
            log_error("Primary server: failed to read header: ", ec.message());
          }
          // Connect();
          self->Close();
        }
      };

      // Read the size of the buffer that is coming.
      boost::asio::async_read(
          self->_socket,
          message->size_as_buffer(),
          boost::asio::bind_executor(self->_strand, handle_read_header));
    });
  }

  void Primary::Close() {
    std::weak_ptr<Primary> weak = shared_from_this();
    boost::asio::post(_strand, [weak]() {
      auto self = weak.lock();
      if (!self) return;
      self->CloseNow();
    });
  }

  void Primary::StartTimer() {
    if (_deadline.expires_at() <= boost::asio::deadline_timer::traits_type::now()) {
      log_debug("session ", _session_id, " time out");
      Close();
    } else {
      std::weak_ptr<Primary> weak = shared_from_this();
      _deadline.async_wait([weak](boost::system::error_code ec) {
        auto self = weak.lock();
        if (!self) return;
        if (!ec) {
          self->StartTimer();
        } else {
          log_error("session ", self->_session_id, " timed out error: ", ec.message());
        }
      });
    }
  }

  void Primary::CloseNow(boost::system::error_code ec) {
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

} // namespace multigpu
} // namespace carla
