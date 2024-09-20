// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/streaming/detail/tcp/Client.h"

#include "carla/BufferPool.h"
#include "carla/Debug.h"
#include "carla/Exception.h"
#include "carla/Logging.h"
#include "carla/Time.h"

// C++ Boost Asio是一个基于事件驱动的网络编程库，提供了异步的、非阻塞的网络编程接口。
#include <boost/asio/connect.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>
// 通过停止使用boost post，删除了ServerSession和Client中Write函数的并行化，
// 这会导致客户机和服务器之间的不同步，并最终导致泄漏：https://github.com/carla-simulator/carla/pull/8130
#include <boost/asio/bind_executor.hpp>

#include <exception>

namespace carla {
namespace streaming {
namespace detail {
namespace tcp {

  // ===========================================================================
  // -- 传入消息 IncomingMessage ------------------------------------------------
  // ===========================================================================

  /// 读取传入TCP消息的助手。在单个缓冲区中分配整个消息。
  class IncomingMessage {
  public:

    explicit IncomingMessage(Buffer &&buffer) : _message(std::move(buffer)) {}

    // 获取缓冲区的大小
    boost::asio::mutable_buffer size_as_buffer() {
      return boost::asio::buffer(&_size, sizeof(_size));
    }

    // 获取消息的缓冲区
    boost::asio::mutable_buffer buffer() {
      DEBUG_ASSERT(_size > 0u);
      _message.reset(_size);
      return _message.buffer();
    }

    auto size() const {
      return _size;
    }

    auto pop() {
      // std::move 将左值转换为右值（转移所有权或启用对象的移动语义）
      // 移动语义允许开发人员有效地将资源（如内存或文件句柄）从一个对象传输到另一个对象，而无需进行不必要的复制。
      return std::move(_message);
    }

  private:

    message_size_type _size = 0u;

    Buffer _message;
  };

  // ===========================================================================
  // -- 客户端 ------------------------------------------------------------------
  // ===========================================================================

  Client::Client(
      boost::asio::io_context &io_context,
      const token_type &token,
      callback_function_type callback)
    : LIBCARLA_INITIALIZE_LIFETIME_PROFILER(
          std::string("tcp client ") + std::to_string(token.get_stream_id())),
      _token(token),
      _callback(std::move(callback)),
      _socket(io_context),
      _strand(io_context),
      _connection_timer(io_context),
      _buffer_pool(std::make_shared<BufferPool>()) {
    if (!_token.protocol_is_tcp()) {
      throw_exception(std::invalid_argument("invalid token, only TCP tokens supported"));
    }
  }

  Client::~Client() = default;


  // 连接
  void Client::Connect() {
    auto self = shared_from_this();
      if (_done) {
        return;
      }

      using boost::system::error_code;

      if (_socket.is_open()) {
        _socket.close();
      }

      DEBUG_ASSERT(_token.is_valid());
      DEBUG_ASSERT(_token.protocol_is_tcp());
      const auto ep = _token.to_tcp_endpoint();

      auto handle_connect = [this, self, ep](error_code ec) {
        if (!ec) {
          if (_done) {
            return;
          }
          // 强制不使用Nagle(内格尔)算法。
          // Nagle算法：当一个TCP连接上有数据要发送时，并不立即发送出去，
          // 而是等待一小段时间（通常是由一个RTT，即往返时延来估计），看看是否有更多的数据要发送。
          // 如果在这段时间内有额外的数据产生，那么这些数据就会被组装成一个更大的报文一起发送。
          // 这样做可以减少网络中由于过多的小包而引起的拥塞。
          // 然而，它也可能引入额外的延迟，特别是在同步模式中，用户可能感觉到响应变慢，
          // 禁用Nagle算法，将Linux上的同步模式速度提高了约3倍。
          // 以牺牲带宽效率为代价，换取更低的延迟。
          _socket.set_option(boost::asio::ip::tcp::no_delay(true));
          log_debug("streaming client: connected to", ep);
          // 发送流id以订阅流。
          const auto &stream_id = _token.get_stream_id();
          log_debug("streaming client: sending stream id", stream_id);
          boost::asio::async_write(
              _socket,
              boost::asio::buffer(&stream_id, sizeof(stream_id)),
              boost::asio::bind_executor(_strand, [=](error_code ec, size_t DEBUG_ONLY(bytes)) {
                // 确保在连接停止后停止执行。
                if (_done) {
                  return;
                }
                if (!ec) {
                  DEBUG_ASSERT_EQ(bytes, sizeof(stream_id));
                  // 如果成功，开始读取数据。
                  ReadData();
                } else {
                  // 否则再尝试连接一次。
                  log_debug("streaming client: failed to send stream id:", ec.message());
                  Connect();
                }
              }));
        } else {
          log_info("streaming client: connection failed:", ec.message());
          Reconnect();
        }
      };

      log_debug("streaming client: connecting to", ep);
      _socket.async_connect(ep, boost::asio::bind_executor(_strand, handle_connect));
  }


  // 停止连接
  void Client::Stop() {
    _connection_timer.cancel();
    auto self = shared_from_this();
      _done = true;
      if (_socket.is_open()) {
        _socket.close();
      }
  }


  // 重新连接
  void Client::Reconnect() {
    auto self = shared_from_this();
    _connection_timer.expires_from_now(time_duration::seconds(1u));
    _connection_timer.async_wait([this, self](boost::system::error_code ec) {
      if (!ec) {
        Connect();
      }
    });
  }


  // 读取数据
  void Client::ReadData() {
    auto self = shared_from_this();
      if (_done) {
        return;
      }

      // log_debug("streaming client: Client::ReadData");

      auto message = std::make_shared<IncomingMessage>(_buffer_pool->Pop());

      auto handle_read_data = [this, self, message](boost::system::error_code ec, size_t DEBUG_ONLY(bytes)) {
        DEBUG_ONLY(log_debug("streaming client: Client::ReadData.handle_read_data", bytes, "bytes"));
        if (!ec) {
          DEBUG_ASSERT_EQ(bytes, message->size());
          DEBUG_ASSERT_NE(bytes, 0u);
          // 将缓冲区移动到回调函数并开始读取下一块数据。
          // log_debug("streaming client: success reading data, calling the callback");
          self->_callback(message->pop());
          ReadData();
        } else {
          // 像往常一样，如果出了什么问题，就从头再来。
          log_debug("streaming client: failed to read data:", ec.message());
          Connect();
        }
      };

      auto handle_read_header = [this, self, message, handle_read_data](
          boost::system::error_code ec,
          size_t DEBUG_ONLY(bytes)) {
        DEBUG_ONLY(log_debug("streaming client: Client::ReadData.handle_read_header", bytes, "bytes"));
        if (!ec && (message->size() > 0u)) {
          DEBUG_ASSERT_EQ(bytes, sizeof(message_size_type));
          if (_done) {
            return;
          }
          // 现在我们知道了即将到来的缓冲区的大小，我们可以分配缓冲区并开始将数据放入其中。
          boost::asio::async_read(
              _socket,
              message->buffer(),
              boost::asio::bind_executor(_strand, handle_read_data));
        } else if (!_done) {
          log_debug("streaming client: failed to read header:", ec.message());
          DEBUG_ONLY(log_debug("size  = ", message->size()));
          DEBUG_ONLY(log_debug("bytes = ", bytes));
          Connect();
        }
      };

      // 读取即将到来的缓冲区的大小。
      boost::asio::async_read(
          _socket,
          message->size_as_buffer(),
          boost::asio::bind_executor(_strand, handle_read_header));
  }

} // namespace tcp
} // namespace detail
} // namespace streaming
} // namespace carla
