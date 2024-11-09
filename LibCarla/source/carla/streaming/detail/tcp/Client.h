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
    /// 缓冲区池类，用于管理缓冲区的分配和释放。
  class BufferPool;

namespace streaming {
namespace detail {
namespace tcp {

    /// @class Client
    /// @brief 连接单个流的客户端。
    /// 
    /// @warning 在释放共享指针之前，应该先停止这个客户端，否则它将不会被销毁。
  class Client
    : public std::enable_shared_from_this<Client>,// 使Client类支持通过shared_from_this()获取自身的shared_ptr
      private profiler::LifetimeProfiled,// 继承自LifetimeProfiled类，用于生命周期性能分析（私有继承表示不在外部接口中暴露该基类）
      private NonCopyable {// 继承自NonCopyable类，表示该类不可复制（私有继承表示不在外部接口中暴露该基类）
  public:
      /// @typedef endpoint
    /// @brief TCP端点类型，用于表示TCP连接的一端。
    using endpoint = boost::asio::ip::tcp::endpoint;
    /// @typedef protocol_type
    /// @brief 协议类型，表示TCP协议。
    using protocol_type = endpoint::protocol_type;
    /// @typedef callback_function_type
    /// @brief 回调函数类型，接收一个Buffer作为参数。
    using callback_function_type = std::function<void (Buffer)>;
    /// @brief 构造函数。
   /// 
   /// @param io_context 引用boost::asio的I/O上下文对象，用于异步操作。
   /// @param token 流的令牌，包含流的唯一标识等信息。
   /// @param callback 回调函数，当接收到数据时调用。
    Client(
        boost::asio::io_context &io_context,
        const token_type &token,
        callback_function_type callback);
    /// @brief 析构函数。
    ~Client();
    /// @brief 连接到指定的流。
    void Connect();
    /// @brief 获取流的ID。
    /// 
    /// @return 流的ID。
    stream_id_type GetStreamId() const {
      return _token.get_stream_id();
    }
    /// @brief 停止客户端。
    void Stop();

  private:
      /// @brief 重新连接流。
///
/// 此方法尝试重新建立与流的连接。
    void Reconnect();
    /// @brief 从流中读取数据。
///
/// 此方法从已连接的流中读取数据，并处理这些数据。
    void ReadData();
    /// @brief 存储流的唯一标识令牌。
///
/// 这是一个常量，用于在客户端的整个生命周期内唯一标识流。
    const token_type _token;
    /// @brief 回调函数类型，用于处理读取的数据。
///
/// 当从流中读取到数据时，将调用此回调函数，并将读取到的数据作为参数传递给它。
    callback_function_type _callback;
    /// @brief TCP套接字，用于与流建立连接。
///
/// 这是一个Boost.Asio的TCP套接字对象，用于与远程服务器进行通信。
    boost::asio::ip::tcp::socket _socket;
    /// @brief 序列化对套接字的访问。
///
/// 这是一个Boost.Asio的io_context strand对象，用于确保对套接字的访问是线程安全的。
    boost::asio::io_context::strand _strand;
    /// @brief 连接超时定时器。
///
/// 这是一个Boost.Asio的deadline_timer对象，用于在连接尝试超过指定时间后触发超时事件。
    boost::asio::deadline_timer _connection_timer;
    /// @brief 指向缓冲区池的共享指针。
///
/// 这是一个指向BufferPool对象的共享指针，用于管理内存缓冲区。
    std::shared_ptr<BufferPool> _buffer_pool;
    /// @brief 表示客户端是否已完成工作的原子布尔值。
///
/// 这是一个原子布尔值，用于在线程之间安全地表示客户端是否已完成其工作。初始值为false，表示客户端仍在运行。
    std::atomic_bool _done{false};
  };

} // namespace tcp
} // namespace detail
} // namespace streaming
} // namespace carla
