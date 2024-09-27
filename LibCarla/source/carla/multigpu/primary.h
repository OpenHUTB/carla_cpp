// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/NonCopyable.h"
#include "carla/Time.h"
#include "carla/TypeTraits.h"
#include "carla/profiler/LifetimeProfiled.h"
#include "carla/streaming/detail/Types.h"
#include "carla/streaming/detail/tcp/Message.h"
#include "carla/multigpu/listener.h"

#include <boost/asio/deadline_timer.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/strand.hpp>

#include <functional>
#include <memory>

namespace carla {
namespace multigpu {

  /// TCP 服务器会话。会话打开时，它会从套接字读取流 ID 对象并将其自身传递给回调函子。
  // 会话在达到不活动超时 @a timeout 后自行关闭。
  class Primary
    : public std::enable_shared_from_this<Primary>,
      private profiler::LifetimeProfiled,
      private NonCopyable {
  public:

    using socket_type = boost::asio::ip::tcp::socket;

    explicit Primary(
        boost::asio::io_context &io_context,
        time_duration timeout,
        Listener &server);

    ~Primary();

    /// 启动会话并在成功读取流 id 后调用 @a on_opened，会话关闭后调用 @a on_closed。
    void Open(
        Listener::callback_function_type on_opened,
        Listener::callback_function_type on_closed,
        Listener::callback_function_type_response on_response);

    template <typename... Buffers>
    static auto MakeMessage(Buffers... buffers) {
      static_assert(
          are_same<SharedBufferView, Buffers...>::value,
          "This function only accepts arguments of type BufferView.");
      return std::make_shared<const carla::streaming::detail::tcp::Message>(buffers...);
    }

    /// 将一些数据写入套接字。
    void Write(std::shared_ptr<const carla::streaming::detail::tcp::Message> message);

    /// 写入字符串
    void Write(std::string text);

    /// 读取数据
    void ReadData();

    /// 将一些数据写入套接字。
    template <typename... Buffers>
    void Write(Buffers... buffers) {
      Write(MakeMessage(buffers...));
    }

    /// 发布工作以关闭会话。
    void Close();

  private:

    // 开启计时器
    void StartTimer();

    // 立即关闭
    void CloseNow(boost::system::error_code ec = boost::system::error_code());

    friend class Listener;

    Listener &_server;

    const size_t _session_id;

    socket_type _socket;

    time_duration _timeout;

    boost::asio::deadline_timer _deadline;

    boost::asio::io_context::strand _strand;

    Listener::callback_function_type _on_closed;

    Listener::callback_function_type_response _on_response;

    std::shared_ptr<BufferPool> _buffer_pool;

    bool _is_writing = false;

  };

} // namespace multigpu
} // namespace carla
