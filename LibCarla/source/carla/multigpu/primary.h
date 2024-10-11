// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once  // 防止头文件被重复包含

#include "carla/NonCopyable.h"  // 引入不可复制类的定义
#include "carla/Time.h"  // 引入时间相关的定义
#include "carla/TypeTraits.h"  // 引入类型特征的定义
#include "carla/profiler/LifetimeProfiled.h"  // 引入生命周期分析的定义
#include "carla/streaming/detail/Types.h"  // 引入流相关的类型定义
#include "carla/streaming/detail/tcp/Message.h"  // 引入 TCP 消息的定义
#include "carla/multigpu/listener.h"  // 引入监听器的定义

#include <boost/asio/deadline_timer.hpp>  // 引入 Boost.Asio 的截止时间定时器
#include <boost/asio/io_context.hpp>  // 引入 Boost.Asio 的 IO 上下文
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
