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
    DEBUG_ASSERT(on_opened && on_closed);// 确保回调函数不为空。 

    // 这强制不使用 Nagle 算法。将 Linux 上的同步模式速度提高了约 3 倍。
    const boost::asio::ip::tcp::no_delay option(true);
    _socket.set_option(option);

    // 保存回调函数的引用。
    _on_closed = std::move(on_closed);
    _on_response = std::move(on_response);
    // 调用`on_opened`回调，传入当前`Primary`对象的共享指针。 
    on_opened(shared_from_this());
    // 开始读取数据
    ReadData();
  }

  /// 将一些数据写入套接字。
  void Primary::Write(std::shared_ptr<const carla::streaming::detail::tcp::Message> message) {
    DEBUG_ASSERT(message != nullptr);// 确保消息不为空。
    DEBUG_ASSERT(!message->empty());// 确保消息内容不为空。  
    // 创建一个当前`Primary`对象的弱引用，以避免循环引用。
    std::weak_ptr<Primary> weak = shared_from_this();
    // 在IO上下文的执行器上异步执行任务。
    boost::asio::post(_strand, [=]() {
        // 尝试获取当前`Primary`对象的强引用。
      auto self = weak.lock();
      if (!self) return;// 如果对象已被销毁，则直接返回。
      // 检查套接字是否仍然打开。
      if (!self->_socket.is_open()) {
        return;// 如果套接字已关闭，则不执行写入操作。
      }
      // 定义一个回调函数来处理写入完成后的结果。 
      auto handle_sent = [weak, message](const boost::system::error_code &ec, size_t DEBUG_ONLY(bytes)) {
          // 尝试获取当前`Primary`对象的强引用。  
        auto self = weak.lock();
        if (!self) return;// 如果对象已被销毁，则直接返回。
        // 检查是否发生错误。
        if (ec) {
            // 记录错误日志，并立即关闭会话。  
          log_error("session ", self->_session_id, ": error sending data: ", ec.message());
          self->CloseNow(ec);
        } else {
          // DEBUG_ASSERT_EQ(bytes, sizeof(message_size_type) + message->size());
        }
      };
      // 设置超时时间。 
      self->_deadline.expires_from_now(self->_timeout);
      // 异步写入数据到套接字，并在写入完成后调用`handle_sent`回调函数。
      boost::asio::async_write(
          self->_socket,
          message->GetBufferSequence(),
          boost::asio::bind_executor(self->_strand, handle_sent));
    });
  }
  /**
 * @brief 异步地将文本数据写入套接字。
 *
 * 此方法将给定的文本数据异步写入TCP套接字。首先发送数据的大小（以字节为单位），
 * 然后发送实际的文本数据。如果套接字未打开或对象已被销毁，则不执行任何操作。
 *
 * @param text 要写入套接字的文本数据。
 */
  void Primary::Write(std::string text) {
      // 创建一个当前对象的弱引用，以避免循环引用。 
    std::weak_ptr<Primary> weak = shared_from_this();
    // 在IO上下文的执行器上异步执行任务。
    boost::asio::post(_strand, [=]() {
        // 尝试获取当前对象的强引用。
      auto self = weak.lock();
      if (!self) return; // 如果对象已被销毁，则直接返回。
      // 检查套接字是否仍然打开。
      if (!self->_socket.is_open()) {
        return;// 如果套接字已关闭，则不执行写入操作。 
      }

      // 发送的第一个大小缓冲区
      self->_deadline.expires_from_now(self->_timeout);
      int this_size = text.size();
      boost::asio::async_write(
          self->_socket,
          boost::asio::buffer(&this_size, sizeof(this_size)),
          // 发送完成后不执行任何操作（占位回调）。
          boost::asio::bind_executor(self->_strand, [](const boost::system::error_code &, size_t){ }));
      // 发送实际的文本数据。 
      boost::asio::async_write(
          self->_socket,
          boost::asio::buffer(text.c_str(), text.size()),
          // 发送完成后不执行任何操作（占位回调）。
          boost::asio::bind_executor(self->_strand, [](const boost::system::error_code &, size_t){ }));
    });
  }
  /**
 * @brief 异步地读取套接字数据。
 *
 * 此方法异步地从TCP套接字读取数据。它首先尝试获取当前对象的强引用，
 * 如果成功，则分配一个缓冲区来接收数据，并注册一个回调函数来处理读取操作的结果。
 * 如果读取成功，它将调用`_on_response`回调函数，并递归地调用自己以继续读取数据。
 * 如果读取失败，则记录错误日志并重新开始读取过程。
 */
  void Primary::ReadData() {
      // 创建一个当前对象的弱引用，以避免循环引用。
    std::weak_ptr<Primary> weak = shared_from_this();
    // 在IO上下文的执行器上异步执行任务。
    boost::asio::post(_strand, [weak]() {
        // 尝试获取当前对象的强引用。 
      auto self = weak.lock();
      if (!self) return;// 如果对象已被销毁，则直接返回。
      // 分配一个缓冲区来接收数据。
      auto message = std::make_shared<IncomingMessage>(self->_buffer_pool->Pop());
      // 定义回调函数来处理读取操作的结果。
      auto handle_read_data = [weak, message](boost::system::error_code ec, size_t DEBUG_ONLY(bytes)) {
          // 尝试获取当前对象的强引用。
        auto self = weak.lock();
        if (!self) return;// 如果对象已被销毁，则直接返回。
        // 检查是否读取成功。
        if (!ec) {
            // 验证读取的字节数是否与缓冲区大小一致，并且不为0。  
          DEBUG_ASSERT_EQ(bytes, message->size());
          DEBUG_ASSERT_NE(bytes, 0u);
          // 将缓冲区中的数据移交给回调函数，并开始读取下一块数据。 
          self->_on_response(self, message->pop());
          std::cout << "Getting data on listener\n";
          self->ReadData(); // 递归调用以继续读取数据。 
        } else {
            // 如果读取失败，则记录错误日志并重新开始读取过程。
          log_error("primary server: failed to read data: ", ec.message());
        }
      };
      /**
 * @brief 异步读取套接字头部信息（通常包含后续数据的大小）的回调函数。
 *
 * 此回调函数用于处理从TCP套接字异步读取的头部信息。如果成功读取且消息大小大于0，
 * 则会分配相应大小的缓冲区，并启动异步读取数据的操作。如果读取失败或消息大小为0，
 * 则会记录错误日志并关闭连接。
 *
 * @param ec 读取操作的结果代码。如果为0，表示读取成功；否则表示读取失败。
 * @param bytes 读取的字节数（仅在调试模式下使用）。
 */
      auto handle_read_header = [weak, message, handle_read_data](
          boost::system::error_code ec,
          size_t DEBUG_ONLY(bytes)) {
        auto self = weak.lock();
        if (!self) return;
        if (!ec && (message->size() > 0u)) {
            // 既然已经知道了即将到来的缓冲区的大小，我们就可以分配缓冲区并开始存储数据。
          boost::asio::async_read(
              self->_socket,
              message->buffer(),
              boost::asio::bind_executor(self->_strand, handle_read_data));
        } else {
          if (ec) {
            log_error("Primary server: failed to read header: ", ec.message());
          }
          // Connect(); // 此处可能需要根据实际情况决定是否重连 
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
  /**
 * @brief 异步读取套接字头部信息（即数据缓冲区的大小）。
 *
 * 此方法会启动一个异步读取操作，从TCP套接字中读取数据缓冲区的大小。
 * 读取完成后，会调用`handle_read_header`回调函数来处理读取结果。
 */
 // 接下来的代码段是前面代码的一部分，为了完整性而保留在此处，但注释已添加到上面的回调函数中。  
 // ...（省略了部分代码，具体为async_read的调用）  

 /**
  * @brief 关闭连接并释放资源。
  *
  * 此方法会启动一个异步任务，在该任务中尝试获取当前对象的强引用。
  * 如果成功获取，则调用`CloseNow`方法来关闭连接并释放资源。
  */
  void Primary::Close() {
    std::weak_ptr<Primary> weak = shared_from_this();
    boost::asio::post(_strand, [weak]() {
      auto self = weak.lock();
      if (!self) return;
      self->CloseNow();
    });
  }
  /**
 * @brief 启动定时器以监控连接是否超时。
 *
 * 此方法会检查定时器是否已经过期。如果已过期，则记录调试信息并关闭连接。
 * 如果未过期，则启动一个异步等待操作，等待定时器超时。超时后，会递归调用`StartTimer`
 * 方法以继续监控，或者在遇到错误时记录错误日志。
 */
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
  /// \brief 立即关闭连接并处理相关资源。  
///  
/// 此方法用于在接收到关闭指令或错误时，立即取消任何挂起的操作，关闭套接字，  
/// 并通知相关的关闭事件处理函数。  
///  
/// \param ec 错误代码，指示关闭操作是否由于错误而触发。
  void Primary::CloseNow(boost::system::error_code ec) {
      /// \details 取消所有挂起的定时器操作。 
    _deadline.cancel();
    /// \details 如果没有错误代码（即正常关闭），则检查套接字是否仍然打开
    if (!ec)
    {
        /// \details 如果套接字仍然打开，则先进行双向关闭操作，然后关闭套接字
      if (_socket.is_open()) {
        boost::system::error_code ec2;// 用于捕获shutdown操作的错误代码
        _socket.shutdown(boost::asio::socket_base::shutdown_both, ec2);
        _socket.close();// 关闭套接字  
      }
    }
    /// \details 通知关闭事件的处理函数，传递当前对象的共享指针。  
    _on_closed(shared_from_this());
    /// \details 记录调试信息，表明会话已关闭。
    log_debug("session", _session_id, "closed");
  }

} // namespace multigpu
} // namespace carla
