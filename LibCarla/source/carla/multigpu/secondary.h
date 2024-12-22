// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once  // 确保该头文件只被包含一次

#include "carla/Buffer.h"  // 引入 Buffer 相关头文件
#include "carla/NonCopyable.h"  // 引入不可拷贝类的头文件
#include "carla/TypeTraits.h"  // 引入类型特征相关头文件
#include "carla/profiler/LifetimeProfiled.h"  // 引入生命周期分析相关头文件
#include "carla/multigpu/secondaryCommands.h"  // 引入多 GPU 次级命令的头文件
#include "carla/streaming/detail/tcp/Message.h"  // 引入 TCP 消息的详细实现头文件
#include "carla/streaming/detail/Token.h"  // 引入 Token 的详细实现头文件
#include "carla/streaming/detail/Types.h"  // 引入流相关类型的头文件
#include "carla/ThreadPool.h"  // 引入线程池的头文件

#include <boost/asio/deadline_timer.hpp>  // 引入 Boost ASIO 的定时器头文件
#include <boost/asio/io_context.hpp>  // 引入 Boost ASIO 的 IO 上下文头文件
#include <boost/asio/ip/tcp.hpp>  // 引入 Boost ASIO 的 TCP 头文件
#include <boost/asio/strand.hpp>  // 引入 Boost ASIO 的 Strand 头文件

#include <atomic>  // 引入原子类型支持
#include <functional>  // 引入函数对象和绑定支持
#include <memory>  // 引入智能指针支持

namespace carla {  // 定义 carla 命名空间

  class BufferPool;  // 前向声明 BufferPool 类

namespace multigpu {  // 定义 multigpu 命名空间

  class Secondary  // 定义 Secondary 类
    : public std::enable_shared_from_this<Secondary>,  // 允许共享自身的智能指针
      private profiler::LifetimeProfiled,  // 继承生命周期分析特性
      private NonCopyable {  // 禁止拷贝
  public:

    using endpoint = boost::asio::ip::tcp::endpoint;  // 定义 endpoint 类型
    using protocol_type = endpoint::protocol_type;  // 定义协议类型

    // 构造函数：接受 TCP 端点和回调函数
    Secondary(boost::asio::ip::tcp::endpoint ep, SecondaryCommands::callback_type callback);
    // 构造函数：接受 IP 地址和端口号以及回调函数
    Secondary(std::string ip, uint16_t port, SecondaryCommands::callback_type callback);
    ~Secondary();  // 析构函数

    void Connect();  // 连接到指定的 TCP 端点

    void Stop();  // 停止操作

    void AsyncRun(size_t worker_threads);  // 异步运行，启动指定数量的工作线程

    void Write(std::shared_ptr<const carla::streaming::detail::tcp::Message> message);  // 写入消息
    void Write(Buffer buffer);  // 写入 Buffer
    void Write(std::string text);  // 写入字符串

    SecondaryCommands &GetCommander() {  // 获取命令器的引用
      return _commander;
    }

    // 静态成员函数：创建消息
    template <typename... Buffers>
    static auto MakeMessage(Buffers... buffers) {
      static_assert(
          are_same<SharedBufferView, Buffers...>::value,
          "This function only accepts arguments of type BufferView.");  // 检查参数类型
      return std::make_shared<const carla::streaming::detail::tcp::Message>(buffers...);  // 创建并返回消息
    }

  private:

    void Reconnect();  // 重新连接

    void ReadData();  // 读取数据

    ThreadPool                        _pool;  // 线程池
    boost::asio::ip::tcp::socket      _socket;  // TCP socket
    boost::asio::ip::tcp::endpoint    _endpoint;  // TCP 端点
    boost::asio::io_context::strand   _strand;  // ASIO 的 Strand 用于同步
    boost::asio::deadline_timer       _connection_timer;  // 连接定时器
    std::shared_ptr<BufferPool>       _buffer_pool;  // Buffer 池的智能指针
    std::atomic_bool                  _done {false};  // 原子布尔值，表示是否完成
    SecondaryCommands                 _commander;  // 次级命令处理器
  };

} // namespace multigpu
} // namespace carla
