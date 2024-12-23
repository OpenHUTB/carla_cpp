// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once  // 这是一个预处理指令，用于确保当前头文件在每个编译单元中只被包含（include）一次，防止重复包含导致的编译错误。

// 引入Carla库中定义的不可复制类的头文件
#include "carla/NonCopyable.h"  
// 这个类通常用于实现禁止对象被复制的功能，通过删除拷贝构造函数和赋值运算符来实现。

// 引入Carla库中与时间相关的定义的头文件
#include "carla/Time.h"  
// 这个头文件可能包含了时间类型、时间戳、时间间隔等与时间相关的定义和函数。

// 引入Carla库中类型特征的定义的头文件
#include "carla/TypeTraits.h"  
// 类型特征（Type Traits）是C++中用于查询类型属性的一组模板类和函数，这个头文件可能包含了这些模板类和函数的定义。

// 引入Carla库中生命周期分析的定义的头文件
#include "carla/profiler/LifetimeProfiled.h"  
// 这个头文件可能包含了用于分析对象生命周期的类、函数或宏定义，帮助开发者了解对象的创建和销毁情况。

// 引入Carla流处理框架中流相关的类型定义的头文件
#include "carla/streaming/detail/Types.h"  
// 这个头文件可能包含了流处理框架中使用的各种类型定义，如数据类型、错误码等。

// 引入Carla流处理框架中TCP消息的定义的头文件
#include "carla/streaming/detail/tcp/Message.h"  
// 这个头文件可能包含了TCP消息的结构定义、序列化/反序列化函数等。

// 引入Carla多GPU处理框架中用于监听GPU事件和状态的Listener类的定义的头文件
#include "carla/multigpu/listener.h"  
// 这个头文件包含了Listener类的定义，该类可能用于监听GPU的事件（如任务完成、错误等）和状态（如温度、利用率等）。

// 引入Boost.Asio库中截止时间定时器的头文件
#include <boost/asio/deadline_timer.hpp>  
// 截止时间定时器（deadline_timer）是Boost.Asio库中用于在指定时间点触发操作的类。

// 引入Boost.Asio库中IO上下文的头文件
#include <boost/asio/io_context.hpp>  
// IO上下文（io_context）是Boost.Asio库中的核心组件之一，它管理异步操作的执行和调度。
#include <boost/asio/ip/tcp.hpp>  // 引入 Boost.Asio 的 TCP IP 协议
#include <boost/asio/strand.hpp>  // 引入 Boost.Asio 的 Strand 用于同步

#include <functional>  // 引入函数对象的定义
#include <memory>  // 引入智能指针的定义

namespace carla {
namespace multigpu {

  /// TCP 服务器会话。会话打开时，它会从套接字读取流 ID 对象并将其自身传递给回调函子。
  // 会话在达到不活动超时 @a timeout 后自行关闭。
  class Primary
    : public std::enable_shared_from_this<Primary>,  // 允许生成指向自身的共享指针
      private profiler::LifetimeProfiled,  // 进行生命周期分析
      private NonCopyable {  // 禁止拷贝构造和赋值操作
  public:

    using socket_type = boost::asio::ip::tcp::socket;  // 定义 socket 类型

    // 构造函数，接受 IO 上下文、超时时间和监听器引用
    explicit Primary(
        boost::asio::io_context &io_context,  // IO 上下文引用
        time_duration timeout,  // 超时时间
        Listener &server);  // 监听器引用

    ~Primary();  // 析构函数

    /// 启动会话并在成功读取流 id 后调用 @a on_opened，会话关闭后调用 @a on_closed。
    void Open(
        Listener::callback_function_type on_opened,  // 会话打开后的回调
        Listener::callback_function_type on_closed,  // 会话关闭后的回调
        Listener::callback_function_type_response on_response);  // 响应回调

    // 创建消息的静态模板函数
    template <typename... Buffers>
    static auto MakeMessage(Buffers... buffers) {
      static_assert(
          are_same<SharedBufferView, Buffers...>::value,  // 确保参数类型正确
          "This function only accepts arguments of type BufferView.");  // 错误信息
      return std::make_shared<const carla::streaming::detail::tcp::Message>(buffers...);  // 返回共享消息指针
    }

    /// 将一些数据写入套接字。
    void Write(std::shared_ptr<const carla::streaming::detail::tcp::Message> message);  // 写入消息

    /// 写入字符串
    void Write(std::string text);  // 写入字符串

    /// 读取数据
    void ReadData();  // 读取数据

    /// 将一些数据写入套接字。
    template <typename... Buffers>
    void Write(Buffers... buffers) {
      Write(MakeMessage(buffers...));  // 使用 MakeMessage 创建并写入消息
    }

    /// 发布工作以关闭会话。
    void Close();  // 关闭会话

  private:

    // 开启计时器
    void StartTimer();  // 启动定时器

    // 立即关闭
    void CloseNow(boost::system::error_code ec = boost::system::error_code());  // 立即关闭会话

    friend class Listener;  // 允许 Listener 访问私有成员

    Listener &_server;  // 监听器引用

    const size_t _session_id;  // 会话 ID

    socket_type _socket;  // 套接字

    time_duration _timeout;  // 超时时间

    boost::asio::deadline_timer _deadline;  // 截止时间定时器

    boost::asio::io_context::strand _strand;  // 用于同步的 Strand

    Listener::callback_function_type _on_closed;  // 会话关闭时的回调

    Listener::callback_function_type_response _on_response;  // 响应回调

    std::shared_ptr<BufferPool> _buffer_pool;  // 缓冲池的共享指针

    bool _is_writing = false;  // 写入状态标志
  };

} // namespace multigpu
} // namespace carla
