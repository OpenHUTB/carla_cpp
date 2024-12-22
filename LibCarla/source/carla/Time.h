// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once// 防止头文件被多次包含

#include "carla/Debug.h"// 引入调试相关的头文件

#include <boost/date_time/posix_time/posix_time_types.hpp>// 引入 Boost 的时间处理功能

#include <chrono>

namespace carla {

  /// Positive time duration up to milliseconds resolution. Automatically casts
  /// between std::chrono::duration and boost::posix_time::time_duration.
// #include "carla/Logging.h" // 这行代码原本用于包含日志记录相关的头文件，但已被注释掉，
// 以避免重复包含或因为某些原因而不希望在此处引入日志记录的依赖。

#include "carla/streaming/detail/tcp/Message.h" // 包含用于TCP消息处理的头文件，
// 该头文件可能定义了TCP消息的格式、解析和构建方法。

#include "carla/ThreadPool.h" // 包含用于管理线程池的头文件，
// 该头文件可能定义了线程池的创建、任务分发、线程管理等功能。

#include "carla/multigpu/primary.h" // 包含多GPU处理主要组件的头文件，
// 该头文件可能定义了Primary类，它是多GPU处理中的核心组件之一。

#include "carla/multigpu/primaryCommands.h" // 包含与Primary组件相关的命令处理的头文件，
// 该头文件可能定义了与Primary组件交互的命令类型、命令处理函数等。

#include "carla/multigpu/commands.h" // 包含多GPU处理中命令定义的头文件，
// 该头文件可能定义了多GPU处理中使用的各种命令的枚举类型或结构体。

#include <boost/asio/io_context.hpp> // 包含Boost.Asio库中I/O上下文的头文件，
// I/O上下文是Boost.Asio库的核心组件之一，用于管理异步操作的执行。

#include <boost/asio/ip/tcp.hpp> // 包含Boost.Asio库中TCP协议相关的头文件，
// 该头文件提供了TCP套接字、服务器和客户端的实现。

#include <mutex> // 包含C++标准库中的互斥锁头文件，
// 互斥锁用于保护共享数据，防止多个线程同时访问导致的数据竞争。

#include <vector> // 包含C++标准库中的动态数组头文件，
// 动态数组是一种能够根据需要动态调整大小的数组。

#include <sstream> // 包含C++标准库中的字符串流头文件，
// 字符串流提供了将字符串与其他数据类型进行转换和格式化的功能。

#include <unordered_map> // 包含C++标准库中的无序映射头文件，
// 无序映射是一种基于哈希表的关联容器，能够高效地存储和查找键值对。

namespace carla {
namespace multigpu {

  // class Primary; // 这是一个被注释掉的前向声明，由于代码中没有使用到Primary类，
  // 因此这个声明是多余的，被注释掉以减少代码的混淆。

  class Listener; // 前向声明Listener类，
  // Listener类可能用于监听网络连接或消息，等待并处理来自客户端或服务器的请求。

  // 定义一个结构体来保存会话信息
  struct SessionInfo {
    std::shared_ptr<Primary> session; // 指向Primary对象的智能指针，
    // 用于管理会话中的Primary组件实例，智能指针能够自动管理内存，防止内存泄漏。
    
    carla::Buffer buffer; // 用于存储数据的缓冲区，
    // 缓冲区可能用于临时存储从网络接收到的数据或准备发送到网络的数据。
  };

  // 注意：Listener类和Primary类的具体实现没有在这个代码片段中给出，
  // 它们可能在其他地方（如其他头文件或源文件）被定义。
}
}
    time_duration(const time_duration &) = default;
    time_duration &operator=(const time_duration &) = default;
// 将当前时间间隔转换为 boost::posix_time::time_duration 类型
    boost::posix_time::time_duration to_posix_time() const {
      return boost::posix_time::milliseconds(_milliseconds);
    }
// 将当前时间间隔转换为 std::chrono::milliseconds 类型
    constexpr auto to_chrono() const {
      return std::chrono::milliseconds(_milliseconds);
    }
// 类型转换运算符，将当前对象转换为 boost::posix_time::time_duration 类型
    operator boost::posix_time::time_duration() const {
      return to_posix_time();
    }
// 返回以毫秒为单位的时间值，不修改对象状态，保证不抛出异常
    constexpr size_t milliseconds() const noexcept {
      return _milliseconds;
    }

  private:
// 存储时间间隔的毫秒数
    size_t _milliseconds;
  };

} // namespace carla
