// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
// 引入所需的头文件
#include "carla/ThreadPool.h"
#include "carla/streaming/detail/tcp/Server.h"
#include "carla/streaming/detail/Types.h"
#include "carla/streaming/low_level/Server.h"

#include <boost/asio/io_context.hpp>
// 定义命名空间
namespace carla {
namespace streaming {

  /// A streaming server. Each new stream has a token associated, this token can
  /// be used by a client to subscribe to the stream.
  class Server {
	   // 使用低级服务器的特定类型别名
    using underlying_server = low_level::Server<detail::tcp::Server>;
    using protocol_type = low_level::Server<detail::tcp::Server>::protocol_type;
    using token_type = carla::streaming::detail::token_type;
    using stream_id = carla::streaming::detail::stream_id_type;
  public:
// 构造函数，接受端口号作为参数，初始化服务器并关联到指定端口的 IO 上下文。
    explicit Server(uint16_t port)
      : _server(_pool.io_context(), make_endpoint<protocol_type>(port)) {}
// 构造函数，接受地址和端口号作为参数，初始化服务器并关联到指定地址和端口的 IO 上下文。
    explicit Server(const std::string &address, uint16_t port)
      : _server(_pool.io_context(), make_endpoint<protocol_type>(address, port)) {}
// 构造函数，接受内部地址、内部端口、外部地址和外部端口作为参数，初始化服务器并关联到指定的内外部地址和端口的 IO 上下文。
    explicit Server(
        const std::string &address, uint16_t port,
        const std::string &external_address, uint16_t external_port)
      : _server(
          _pool.io_context(),
          make_endpoint<protocol_type>(address, port),
          make_endpoint<protocol_type>(external_address, external_port)) {}
// 析构函数，停止线程池。
    ~Server() {
      _pool.Stop();
    }
// 获取服务器的本地端点
    auto GetLocalEndpoint() const {
      return _server.GetLocalEndpoint();
    }
   // 设置服务器的超时时间。
    void SetTimeout(time_duration timeout) {
      _server.SetTimeout(timeout);
    }
    // 创建一个新的流。
    Stream MakeStream() {
      return _server.MakeStream();
    }
    // 关闭指定 ID 的流。
    void CloseStream(carla::streaming::detail::stream_id_type id) {
      return _server.CloseStream(id);
    }
  // 启动线程池，运行服务器。
    void Run() {
      _pool.Run();
    }
// 异步启动线程池，指定工作线程数量运行服务器。
    void AsyncRun(size_t worker_threads) {
      _pool.AsyncRun(worker_threads);
    }
// 设置服务器为同步模式或异步模式。
    void SetSynchronousMode(bool is_synchro) {
      _server.SetSynchronousMode(is_synchro);
    }
// 获取指定流 ID 的令牌。
    token_type GetToken(stream_id sensor_id) {
      return _server.GetToken(sensor_id);
    }
// 为指定流 ID 的传感器启用 ROS 支持。
    void EnableForROS(stream_id sensor_id) {
      _server.EnableForROS(sensor_id);
    }

    void DisableForROS(stream_id sensor_id) {
      _server.DisableForROS(sensor_id);
    }
 // 检查指定流 ID 的传感器是否启用了 ROS 支持。
    bool IsEnabledForROS(stream_id sensor_id) {
      return _server.IsEnabledForROS(sensor_id);
    }

  private:

    // 这两个参数的顺序非常重要。

    ThreadPool _pool;

    underlying_server _server;
  };

} // namespace streaming
} // namespace carla
