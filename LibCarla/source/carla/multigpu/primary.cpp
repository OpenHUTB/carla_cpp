// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// 主服务器
// 
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.
/// @brief 包含CARLA多GPU支持和网络通信相关的头文件，以及Boost.Asio和C++标准库的部分功能。
#include "carla/multigpu/primary.h"///< 包含CARLA多GPU支持的主头文件，可能定义了多GPU环境下的主要接口或类。

#include "carla/Debug.h"///< 包含CARLA的调试功能，可能包括断言、日志记录级别等。
#include "carla/Logging.h"///< 包含CARLA的日志记录功能，可能定义了日志记录器、日志级别和日志消息格式等。
#include "carla/multigpu/incomingMessage.h"///< 包含CARLA多GPU支持中接收消息的相关类和函数。
#include "carla/multigpu/listener.h"///< 包含CARLA多GPU支持中监听网络通信的相关类和函数。
/// @brief 包含Boost.Asio库的头文件，用于网络编程和异步I/O操作。 
#include <boost/asio/read.hpp>///< Boost.Asio库中的读操作函数，用于从网络套接字读取数据。
#include <boost/asio/write.hpp> ///< Boost.Asio库中的写操作函数，用于向网络套接字写入数据。 
#include <boost/asio/bind_executor.hpp>///< Boost.Asio库中的绑定执行器函数，用于将异步操作与特定的执行器（如I/O上下文）关联。
#include <boost/asio/post.hpp> ///< Boost.Asio库中的post函数，用于将任务异步地发布到执行器上执行。
/// @brief 包含C++标准库的头文件，用于多线程编程和原子操作。  
#include <atomic>///< C++标准库中的原子操作类，用于实现线程安全的计数器、标志位等。
#include <thread>///< C++标准库中的线程类和相关函数，用于创建和管理线程。

namespace carla {
namespace multigpu {
    /**
 * @namespace carla::multigpu
 * @brief CARLA模拟器中处理多GPU通信的命名空间。
 */

 /**
  * @brief 用于生成唯一会话ID的静态原子计数器。
  *
  * 这是一个线程安全的计数器，用于为每个新的Primary会话实例分配一个唯一的会话ID。
  */
  static std::atomic_size_t SESSION_COUNTER{0u};
  /**
   * @class Primary
   * @brief 管理TCP会话的类，用于CARLA的多GPU通信。
   */
   /**
      * @brief Primary类的构造函数。
      *
      * @param io_context 引用Boost.Asio的IO上下文，用于异步通信。
      * @param timeout 会话的超时时间。
      * @param server 对Listener对象的引用，用于处理传入的连接和消息。
      *
      * 构造函数初始化Primary类的成员变量，并设置性能分析器（如果使用了LIBCARLA_INITIALIZE_LIFETIME_PROFILER宏）。
      * 它还会为当前会话分配一个唯一的会话ID，并创建一个新的套接字、截止时间和执行器绑定。
      */
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
  /**
 * @brief Primary类的析构函数实现。
 *
 * 如果套接字仍然打开，则先调用shutdown方法关闭套接字的读写操作，然后调用close方法关闭套接字。
 */
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
