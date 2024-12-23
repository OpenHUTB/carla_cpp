// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
// 包含非复制类的定义，可能用于限制类对象不能被复制，以确保某些特定的语义和资源管理逻辑
#include "carla/NonCopyable.h"
#include "carla/NonCopyable.h" // 包含禁止拷贝操作的NonCopyable基类定义，用于防止派生类被意外复制
#include "carla/Time.h" // 包含时间相关的类定义
#include "carla/Buffer.h" // 包含缓冲区类的定义
// 引入Boost库中asio模块的io_context类，用于管理异步I/O操作的上下文环境
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/post.hpp>
// 包含原子操作类，用于实现多线程环境下对变量的原子性操作，保证数据的一致性和并发安全
#include <atomic> // 包含原子操作类

namespace carla {
namespace multigpu {
  // 前置声明一个名为Primary的类，这个类在其他地方定义，意味着此处只是告知编译器有这么一个类存在，后续会在别处完整定义它，并且它与Listener类存在交互关系
  class Primary; // 声明一个名为Primary的类，这个类在其他地方定义，与Listener类有交互

  /// 警告：在它的io_context停止之前，这个服务器不能被销毁。
  class Listener : public std::enable_shared_from_this<Listener>, private NonCopyable {
  public:

    using endpoint = boost::asio::ip::tcp::endpoint; // 使用typedef定义endpoint类型
    using protocol_type = endpoint::protocol_type; // 使用typedef定义protocol_type类型
    using Session = std::shared_ptr<Primary>; // 使用typedef定义Session类型为Primary的shared_ptr
    using callback_function_type = std::function<void(std::shared_ptr<Primary>)>; // 定义回调函数类型
    using callback_function_type_response = std::function<void(std::shared_ptr<Primary>, carla::Buffer)>;// 定义带响应的回调函数类型

    explicit Listener(boost::asio::io_context &io_context, endpoint ep);  // 构造函数，接受一个io_context引用和一个endpoint对象
    ~Listener();  // 析构函数，用于清理资源
    
    endpoint GetLocalEndpoint() const {  // 获取当前Listener绑定的本地端点
      return _acceptor.local_endpoint();
    }

    /// 设置会话超时时间。这只适用于新创建的会话。默认情况下，超时时间设置为10秒。
    /// 
    void SetTimeout(time_duration timeout) { // 设置会话超时时间，用于控制连接的超时行为
      _timeout = timeout;
    }

    /// 开始监听连接。在每个新连接时，会调用on_session_opened，
    /// 在会话关闭时调用on_session_closed，同时在收到响应时调用on_response。
    ///
    void Listen(callback_function_type on_session_opened, 
                callback_function_type on_session_closed,
                callback_function_type_response on_response) { // 开始监听连接
      boost::asio::post(_io_context, [=]() {
        OpenSession(
            _timeout,
            std::move(on_session_opened),
            std::move(on_session_closed),
            std::move(on_response));
      });
    }

    void Stop(); // 停止监听TCP连接

  private:

    void OpenSession(
        time_duration timeout,// time_duration 是一个表示时间长度的类型，可能是自定义的结构体或类
// timeout 参数指定了某个操作（如打开或保持会话）的最大允许时间
        callback_function_type on_session_opened,// callback_function_type 是一个函数类型别名，用于指定回调函数的签名
// on_session_opened 是一个回调函数，当会话成功打开时被调用
        callback_function_type on_session_closed,// on_session_closed 是一个回调函数，当会话关闭时被调用
// 它允许开发者执行清理操作或处理会话结束后的逻辑
        callback_function_type_response on_response); // 私有成员函数，用于处理新会话的打开

    boost::asio::io_context         &_io_context; // 引用io_context
    boost::asio::ip::tcp::acceptor  _acceptor; // TCP acceptor对象，用于接受新的TCP连接
    std::atomic<time_duration>      _timeout; // 原子变量，用于存储会话超时时间
  };

} // namespace multigpu
} // namespace carla
