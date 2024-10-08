// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/multigpu/incomingMessage.h" // 包含接收消息的头文件
#include "carla/multigpu/secondary.h"       // 包含次要功能的头文件

#include "carla/BufferPool.h"                // 包含缓冲池的头文件
#include "carla/Debug.h"                     // 包含调试相关的头文件
#include "carla/Exception.h"                 // 包含异常处理的头文件
#include "carla/Logging.h"                   // 包含日志记录的头文件
#include "carla/Time.h"                      // 包含时间处理的头文件

#include <boost/asio/connect.hpp>            // 包含连接的Boost.Asio头文件
#include <boost/asio/read.hpp>               // 包含读取的Boost.Asio头文件
#include <boost/asio/write.hpp>              // 包含写入的Boost.Asio头文件
#include <boost/asio/post.hpp>               // 包含异步操作的Boost.Asio头文件
#include <boost/asio/bind_executor.hpp>      // 包含绑定执行器的Boost.Asio头文件

#include <exception>                         // 包含标准异常处理头文件

namespace carla {
namespace multigpu {

  Secondary::Secondary(                    // 构造函数，接受端点和回调函数
    boost::asio::ip::tcp::endpoint ep,
    SecondaryCommands::callback_type callback) :
      _pool(),                              // 初始化缓冲池
      _socket(_pool.io_context()),          // 初始化套接字
      _endpoint(ep),                        // 设置端点
      _strand(_pool.io_context()),          // 初始化strand以确保线程安全
      _connection_timer(_pool.io_context()),// 初始化连接计时器
      _buffer_pool(std::make_shared<BufferPool>()) { // 创建共享的缓冲池

      _commander.set_callback(callback);    // 设置回调函数
    }


  Secondary::Secondary(                    // 另一个构造函数，接受IP和端口以及回调函数
    std::string ip,
    uint16_t port,
    SecondaryCommands::callback_type callback) :
      _pool(),                              // 初始化缓冲池
      _socket(_pool.io_context()),          // 初始化套接字
      _strand(_pool.io_context()),          // 初始化strand以确保线程安全
      _connection_timer(_pool.io_context()),// 初始化连接计时器
      _buffer_pool(std::make_shared<BufferPool>()) { // 创建共享的缓冲池

    boost::asio::ip::address ip_address = boost::asio::ip::address::from_string(ip); // 从字符串转换为IP地址
    _endpoint = boost::asio::ip::tcp::endpoint(ip_address, port); // 设置端点
    _commander.set_callback(callback);    // 设置回调函数
  }

  Secondary::~Secondary() {                // 析构函数
    _pool.Stop();                          // 停止缓冲池
  }

  void Secondary::Connect() {              // 连接函数
    AsyncRun(2u);                          // 启动异步运行，使用2个工作线程

    _commander.set_secondary(shared_from_this()); // 设置当前对象为命令的次要部分

    std::weak_ptr<Secondary> weak = shared_from_this(); // 创建弱指针以防止循环引用
    boost::asio::post(_strand, [weak]() { // 在strand中发布任务
      auto self = weak.lock();             // 锁定弱指针
      if (!self) return;                   // 如果对象已被销毁，返回

      if (self->_done) {                   // 如果已完成
        return;
      }

      if (self->_socket.is_open()) {       // 如果套接字是打开的
        self->_socket.close();              // 关闭套接字
      }

      auto handle_connect = [weak](boost::system::error_code ec) { // 处理连接结果的回调
        auto self = weak.lock();            // 锁定弱指针
        if (!self) return;                  // 如果对象已被销毁，返回
        if (ec) {                           // 如果有错误
          log_error("secondary server: connection failed:", ec.message()); // 记录错误信息
          if (!self->_done)                 // 如果未完成，尝试重连
            self->Reconnect();
          return;
        }

        if (self->_done) {                  // 如果已完成
          return;
        }

        // 此设置强制不使用Nagle算法。
        // 在Linux上提高同步模式速度约3倍。
        self->_socket.set_option(boost::asio::ip::tcp::no_delay(true)); // 禁用Nagle算法

        log_info("secondary server: connected to ", self->_endpoint); // 记录连接信息

        self->ReadData();                   // 调用读取数据函数
      };

      self->_socket.async_connect(self->_endpoint, boost::asio::bind_executor(self->_strand, handle_connect)); // 异步连接
    });
  }

  void Secondary::Stop() {                 // 停止函数
    _connection_timer.cancel();             // 取消连接计时器
    std::weak_ptr<Secondary> weak = shared_from_this(); // 创建弱指针
    boost::asio::post(_strand, [weak]() {   // 在strand中发布停止任务
      auto self = weak.lock();              // 锁定弱指针
      if (!self) return;                    // 如果对象已被销毁，返回
      self->_done = true;                   // 标记为已完成
      if (self->_socket.is_open()) {       // 如果套接字是打开的
        self->_socket.close();              // 关闭套接字
      }
    });
  }

  void Secondary::Reconnect() {             // 重连函数
    std::weak_ptr<Secondary> weak = shared_from_this(); // 创建弱指针
    _connection_timer.expires_from_now(time_duration::seconds(1u)); // 设置计时器为1秒后到期
    _connection_timer.async_wait([weak](boost::system::error_code ec) { // 等待计时器到期后的回调
      auto self = weak.lock();              // 锁定弱指针
      if (!self) return;                    // 如果对象已被销毁，返回
      if (!ec) {                            // 如果没有错误
        self->Connect();                    // 重新连接
      }
    });
  }

  void Secondary::AsyncRun(size_t worker_threads) { // 启动异步运行，接受工作线程数量
    _pool.AsyncRun(worker_threads);         // 调用缓冲池的异步运行
  }

  void Secondary::Write(std::shared_ptr<const carla::streaming::detail::tcp::Message> message) { // 写入函数
    DEBUG_ASSERT(message != nullptr);       // 确保消息不为空
    DEBUG_ASSERT(!message->empty());        // 确保消息不为空
    std::weak_ptr<Secondary> weak = shared_from_this(); // 创建弱指针
    boost::asio::post(_strand, [=]() {      // 在strand中发布写入任务
      auto self = weak.lock();              // 锁定弱指针
      if (!self) return;                    // 如果对象已被销毁，返回
      if (!self->_socket.is_open()) {      // 如果套接字未打开
        return;                             // 返回
      }

      auto handle_sent = [weak, message](const boost::system::error_code &ec, size_t DEBUG_ONLY(bytes)) { // 处理发送结果的回调
        auto self = weak.lock();             // 锁定弱指针
        if (!self) return;                   // 如果对象已被销毁，返回
        if (ec) {                           // 如果有错误
          log_error("error sending data: ", ec.message()); // 记录发送数据的错误
        }
      };

      // _deadline.expires_from_now(_timeout);
      boost::asio::async_write(
          self->_socket,
          message->GetBufferSequence(),
          boost::asio::bind_executor(self->_strand, handle_sent));
    });
  }

  void Secondary::Write(Buffer buffer) {
    auto view_data = carla::BufferView::CreateFrom(std::move(buffer));
    auto message = Secondary::MakeMessage(view_data);

    DEBUG_ASSERT(message != nullptr);
    DEBUG_ASSERT(!message->empty());
    std::weak_ptr<Secondary> weak = shared_from_this();
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
          log_error("error sending data: ", ec.message());
        }
      };

      // _deadline.expires_from_now(_timeout);
      boost::asio::async_write(
          self->_socket,
          message->GetBufferSequence(),
          boost::asio::bind_executor(self->_strand, handle_sent));
    });
  }

  void Secondary::Write(std::string text) {
    std::weak_ptr<Secondary> weak = shared_from_this();
    boost::asio::post(_strand, [=]() {
      auto self = weak.lock();
      if (!self) return;
      if (!self->_socket.is_open()) {
        return;
      }

      auto handle_sent = [weak](const boost::system::error_code &ec, size_t DEBUG_ONLY(bytes)) {
        auto self = weak.lock();
        if (!self) return;
        if (ec) {
          log_error("error sending data: ", ec.message());
        }
      };

      // _deadline.expires_from_now(_timeout);
      // sent first size buffer
      int this_size = text.size();
      boost::asio::async_write(
          self->_socket,
          boost::asio::buffer(&this_size, sizeof(this_size)),
          boost::asio::bind_executor(self->_strand, handle_sent));

      // send characters
      boost::asio::async_write(
          self->_socket,
          boost::asio::buffer(text.c_str(), text.size()),
          boost::asio::bind_executor(self->_strand, handle_sent));
    });
  }

  void Secondary::ReadData() {
    std::weak_ptr<Secondary> weak = shared_from_this();
    boost::asio::post(_strand, [weak]() {
      auto self = weak.lock();
      if (!self) return;
      if (self->_done) {
        return;
      }

      auto message = std::make_shared<IncomingMessage>(self->_buffer_pool->Pop());

      auto handle_read_data = [weak, message](boost::system::error_code ec, size_t DEBUG_ONLY(bytes)) {
        auto self = weak.lock();
        if (!self) return;
        if (!ec) {
          DEBUG_ASSERT_EQ(bytes, message->size());
          DEBUG_ASSERT_NE(bytes, 0u);
          // Move the buffer to the callback function and start reading the next
          // piece of data.
          self->GetCommander().process_command(message->pop());
          self->ReadData();
        } else {
          // As usual, if anything fails start over from the very top.
          log_error("secondary server: failed to read data: ", ec.message());
          // Connect();
        }
      };

      auto handle_read_header = [weak, message, handle_read_data](
        boost::system::error_code ec,
        size_t DEBUG_ONLY(bytes)) {
          auto self = weak.lock();
          if (!self) return;
          if (!ec && (message->size() > 0u)) {
            DEBUG_ASSERT_EQ(bytes, sizeof(carla::streaming::detail::message_size_type));
            if (self->_done) {
              return;
            }
            // Now that we know the size of the coming buffer, we can allocate our
            // buffer and start putting data into it.
            boost::asio::async_read(
                self->_socket,
                message->buffer(),
                boost::asio::bind_executor(self->_strand, handle_read_data));
          } else if (!self->_done) {
            log_error("secondary server: failed to read header: ", ec.message());
            // DEBUG_ONLY(printf("size  = ", message->size()));
            // DEBUG_ONLY(printf("bytes = ", bytes));
            // Connect();
          }
        };

      // Read the size of the buffer that is coming.
      boost::asio::async_read(
          self->_socket,
          message->size_as_buffer(),
          boost::asio::bind_executor(self->_strand, handle_read_header));
    });
  }

} // namespace multigpu
} // namespace carla
