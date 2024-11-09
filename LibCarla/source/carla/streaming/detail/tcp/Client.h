// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
/// \file
/// \brief 本文件包含了网络通信相关类所需的头文件。
#include "carla/Buffer.h"/// \include 包含用于网络通信的缓冲区类定义。
#include "carla/NonCopyable.h"/// \include 包含禁止对象复制和赋值的基类定义。
#include "carla/profiler/LifetimeProfiled.h"/// \include 包含用于性能分析的生命周期跟踪类定义。
#include "carla/streaming/detail/Token.h"/// \include 包含流处理中的令牌类定义。
#include "carla/streaming/detail/Types.h"/// \include 包含流处理中使用的类型别名和常量定义。

#include <boost/asio/deadline_timer.hpp>/// \include 包含Boost.Asio的定时器类定义，用于处理超时事件。
#include <boost/asio/io_context.hpp>/// \include 包含Boost.Asio的I/O上下文类定义，是异步操作的核心。
#include <boost/asio/ip/tcp.hpp> /// \include 包含Boost.Asio的TCP协议支持，用于网络通信。
#include <boost/asio/strand.hpp> /// \include 包含Boost.Asio的线程安全操作类定义，用于在多个线程间同步异步操作。

#include <atomic>/// \include 包含C++标准库中的原子操作支持，用于实现线程安全的计数器等。
#include <functional>/// \include 包含C++标准库中的函数对象支持，用于定义回调和可调用对象。
#include <memory>/// \include 包含C++标准库中的智能指针支持，用于管理动态分配的内存。

namespace carla {

  class BufferPool;

namespace streaming {
namespace detail {
namespace tcp {

  /// A client that connects to a single stream.
  ///
  /// @warning This client should be stopped before releasing the shared pointer
  /// or won't be destroyed.
  class Client
    : public std::enable_shared_from_this<Client>,
      private profiler::LifetimeProfiled,
      private NonCopyable {
  public:

    using endpoint = boost::asio::ip::tcp::endpoint;
    using protocol_type = endpoint::protocol_type;
    using callback_function_type = std::function<void (Buffer)>;

    Client(
        boost::asio::io_context &io_context,
        const token_type &token,
        callback_function_type callback);

    ~Client();

    void Connect();

    stream_id_type GetStreamId() const {
      return _token.get_stream_id();
    }

    void Stop();

  private:

    void Reconnect();

    void ReadData();

    const token_type _token;

    callback_function_type _callback;

    boost::asio::ip::tcp::socket _socket;

    boost::asio::io_context::strand _strand;

    boost::asio::deadline_timer _connection_timer;

    std::shared_ptr<BufferPool> _buffer_pool;

    std::atomic_bool _done{false};
  };

} // namespace tcp
} // namespace detail
} // namespace streaming
} // namespace carla
