// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
// #include "carla/Logging.h" // 这行代码原本用于包含Carla日志记录功能的头文件，但已被注释掉，可能是为了避免重复包含或减少对特定日志依赖的考虑。

#include "carla/streaming/detail/tcp/Message.h" // 包含Carla流处理框架中TCP消息定义的头文件，这些消息可能用于网络通信。

#include "carla/ThreadPool.h" // 包含Carla线程池管理的头文件，线程池用于高效地管理和复用线程资源。

#include "carla/multigpu/primary.h" // 包含Carla多GPU处理框架中主要组件和接口的定义，这些组件和接口可能用于管理GPU资源和任务分配。

#include "carla/multigpu/primaryCommands.h" // 包含与Primary组件相关的命令定义的头文件，这些命令可能用于控制或查询Primary组件的状态。

#include "carla/multigpu/commands.h" // 包含Carla多GPU处理框架中通用命令定义的头文件，这些命令可能用于多GPU之间的通信或任务同步。

#include <boost/asio/io_context.hpp> // 包含Boost.Asio库中IO上下文定义的头文件，IO上下文是异步IO操作的核心组件。

#include <boost/asio/ip/tcp.hpp> // 包含Boost.Asio库中TCP网络通信相关的定义和类，用于实现TCP客户端和服务器。

#include <mutex> // 包含C++标准库中的互斥锁头文件，互斥锁用于保护共享资源免受并发访问的干扰。

#include <vector> // 包含C++标准库中的动态数组（向量）头文件，向量是一种能够动态增长和缩小的数组类型。

#include <sstream> // 包含C++标准库中的字符串流头文件，字符串流提供了一种将字符串作为流来处理的机制。

#include <unordered_map> // 包含C++标准库中的无序映射（哈希表）头文件，无序映射是一种能够根据键快速查找值的容器。

namespace carla {
namespace multigpu {

  // class Primary; // 这是一个被注释掉的前向声明，前向声明用于在正式定义类之前声明类的存在，但在此代码中并未使用。
  class Listener; // 声明Listener类，Listener类可能用于监听多GPU处理框架中的事件或状态变化。

  struct SessionInfo { // 定义一个结构体来保存会话信息。
    std::shared_ptr<Primary> session; // 成员变量，指向Primary对象的智能指针，用于管理会话中的Primary对象。
    carla::Buffer buffer; // 成员变量，用于存储数据的缓冲区，Buffer可能是Carla定义的一种数据结构。
  };
} // namespace multigpu
} // namespace carla
  struct SessionInfo { // 定义一个结构体来保存会话信息
    std::shared_ptr<Primary>  session; // 指向Primary对象的智能指针
    carla::Buffer             buffer; // 用于存储数据的缓冲区
  };

  class Router : public std::enable_shared_from_this<Router> { // 定义Router类，继承自std::enable_shared_from_this
  public:

    Router(void); // 构造函数
    explicit Router(uint16_t port); // 带端口参数的构造函数
    ~Router(); // 析构函数

    void Write(MultiGPUCommand id, Buffer &&buffer); // 写入命令到下一个可用的GPU
    std::future<SessionInfo> WriteToNext(MultiGPUCommand id, Buffer &&buffer); // 写入命令到下一个可用的GPU并返回一个future对象
    std::future<SessionInfo> WriteToOne(std::weak_ptr<Primary> server, MultiGPUCommand id, Buffer &&buffer); // 写入命令到指定的GPU并返回一个future对象
    void Stop(); // 停止Router

    void SetCallbacks(); // 设置回调函数
    void SetNewConnectionCallback(std::function<void(void)>); // 设置新连接的回调函数

    void AsyncRun(size_t worker_threads); // 异步运行，指定工作线程的数量

    boost::asio::ip::tcp::endpoint GetLocalEndpoint() const;

    bool HasClientsConnected() {  // 检查是否有客户端连接
      return (!_sessions.empty()); // 如果会话列表不为空，则返回true
    }

    PrimaryCommands &GetCommander() { // 获取命令对象
      return _commander;
    }

    std::weak_ptr<Primary> GetNextServer();  // 获取下一个服务器的弱引用

  private:
    void ConnectSession(std::shared_ptr<Primary> session); // 连接会话
    void DisconnectSession(std::shared_ptr<Primary> session); // 断开会话
    void ClearSessions(); // 清除会话

    // 互斥锁和线程池必须放在开始位置，以确保最后被销毁
    std::mutex                              _mutex; // 互斥锁
    ThreadPool                              _pool; // 线程池
    boost::asio::ip::tcp::endpoint          _endpoint; // TCP端点
    std::vector<std::shared_ptr<Primary>>   _sessions; // 会话列表
    std::shared_ptr<Listener>               _listener; // 监听器
    uint32_t                                _next;  // 下一个会话的索引
    std::unordered_map<Primary *, std::shared_ptr<std::promise<SessionInfo>>> _promises;  // 用于异步操作的承诺映射
    PrimaryCommands                         _commander; // 命令对象
    std::function<void(void)>               _callback; // 回调函数
  };

} // namespace multigpu
} // namespace carla
