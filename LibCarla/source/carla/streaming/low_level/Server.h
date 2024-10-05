// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/streaming/detail/Dispatcher.h" // 引入 Dispatcher 头文件
#include "carla/streaming/detail/Types.h"      // 引入类型定义头文件
#include "carla/streaming/Stream.h"            // 引入 Stream 头文件

#include <boost/asio/io_context.hpp>           // 引入 Boost.Asio 的 IO 上下文头文件

namespace carla {
namespace streaming {
namespace low_level {

  /// 一个低级的流媒体服务器。每个新流都有一个关联的令牌，
  /// 客户端可以使用这个令牌来订阅流。此服务器需要外部 io_context 运行。
  ///
  /// @warning 在停止 @a io_context 之前，无法析构此服务器。
  template <typename T>
  class Server {
  public:

    using underlying_server = T; // 定义底层服务器类型

    using protocol_type = typename underlying_server::protocol_type; // 协议类型

    using token_type = carla::streaming::detail::token_type; // 令牌类型

    using stream_id = carla::streaming::detail::stream_id_type; // 流 ID 类型

    // 构造函数，接受内部和外部端点类型
    template <typename InternalEPType, typename ExternalEPType>
    explicit Server(
        boost::asio::io_context &io_context, // 输入 IO 上下文引用
        detail::EndPoint<protocol_type, InternalEPType> internal_ep, // 内部端点
        detail::EndPoint<protocol_type, ExternalEPType> external_ep) // 外部端点
      : _server(io_context, std::move(internal_ep)), // 初始化底层服务器
        _dispatcher(std::move(external_ep)) { // 初始化调度器
      StartServer(); // 启动服务器
    }

    // 构造函数，仅接受内部端点类型
    template <typename InternalEPType>
    explicit Server(
        boost::asio::io_context &io_context, // 输入 IO 上下文引用
        detail::EndPoint<protocol_type, InternalEPType> internal_ep) // 内部端点
      : _server(io_context, std::move(internal_ep)), // 初始化底层服务器
        _dispatcher(make_endpoint<protocol_type>(_server.GetLocalEndpoint().port())) { // 创建调度器
      StartServer(); // 启动服务器
    }

    // 通用构造函数，接受可变参数的端点
    template <typename... EPArgs>
    explicit Server(boost::asio::io_context &io_context, EPArgs &&... args)
      : Server(io_context, make_endpoint<protocol_type>(std::forward<EPArgs>(args)...)) {} // 调用其他构造函数

    // 获取本地端点
    typename underlying_server::endpoint GetLocalEndpoint() const {
      return _server.GetLocalEndpoint(); // 返回底层服务器的本地端点
    }

    // 设置超时时间
    void SetTimeout(time_duration timeout) {
      _server.SetTimeout(timeout); // 设置底层服务器的超时时间
    }

    // 创建流
    Stream MakeStream() {
      return _dispatcher.MakeStream(); // 调用调度器创建流
    }

    // 关闭指定的流
    void CloseStream(carla::streaming::detail::stream_id_type id) {
      return _dispatcher.CloseStream(id); // 调用调度器关闭流
    }

    // 设置同步模式
    void SetSynchronousMode(bool is_synchro) {
      _server.SetSynchronousMode(is_synchro); // 设置底层服务器的同步模式
    }

    // 获取流的令牌
    token_type GetToken(stream_id sensor_id) {
      return _dispatcher.GetToken(sensor_id); // 从调度器获取流的令牌
    }

    // 为 ROS 启用指定流
    void EnableForROS(stream_id sensor_id) {
      _dispatcher.EnableForROS(sensor_id); // 调用调度器启用流
    }

    // 为 ROS 禁用指定流
    void DisableForROS(stream_id sensor_id) {
      _dispatcher.DisableForROS(sensor_id); // 调用调度器禁用流
    }

    // 检查指定流是否为 ROS 启用
    bool IsEnabledForROS(stream_id sensor_id) {
      return _dispatcher.IsEnabledForROS(sensor_id); // 调用调度器检查流状态
    }

  private:

    // 启动服务器的方法
    void StartServer() {
      // 会话打开时的回调
      auto on_session_opened = [this](auto session) {
        if (!_dispatcher.RegisterSession(session)) { // 注册会话
          session->Close(); // 如果注册失败，关闭会话
        }
      };
      // 会话关闭时的回调
      auto on_session_closed = [this](auto session) {
        log_debug("on_session_closed called"); // 日志记录会话关闭
        _dispatcher.DeregisterSession(session); // 注销会话
      };
      _server.Listen(on_session_opened, on_session_closed); // 开始监听会话
    }

    underlying_server _server; // 底层服务器实例

    detail::Dispatcher _dispatcher; // 调度器实例
  };

} // namespace low_level
} // namespace streaming
} // namespace carla
