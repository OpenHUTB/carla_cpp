// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

// #include "carla/Logging.h" // 这行代码被注释掉，为了避免重复包含或依赖
#include "carla/streaming/detail/tcp/Message.h" // 包含用于TCP消息的头文件
#include "carla/ThreadPool.h" // 包含用于线程池的头文件
#include "carla/multigpu/primary.h" // 包含用于多GPU处理的主要组件的头文件
#include "carla/multigpu/primaryCommands.h" 
#include "carla/multigpu/commands.h"

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>

#include <mutex> // 包含互斥锁的头文件
#include <vector> // 包含动态数组的头文件
#include <sstream> // 包含字符串流的头文件
#include <unordered_map> // 包含无序映射的头文件

namespace carla {
namespace multigpu {

  // class Primary; ///一个前向声明，但代码中没有使用
  class Listener;

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
