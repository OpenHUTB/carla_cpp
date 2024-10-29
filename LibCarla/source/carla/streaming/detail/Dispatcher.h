// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
// 包含必要的头文件
#include "carla/streaming/EndPoint.h"
#include "carla/streaming/Stream.h"
#include "carla/streaming/detail/Session.h"
#include "carla/streaming/detail/Stream.h"
#include "carla/streaming/detail/Token.h"

#include <memory>
#include <mutex>
#include <unordered_map>
// carla 命名空间
namespace carla {
// streaming 子命名空间
namespace streaming {
// detail 子命名空间
namespace detail {
// 前向声明 MultiStreamState 类
  class MultiStreamState;
// 定义 StreamMap 为 std::unordered_map，键为 stream_id_type，值为指向 MultiStreamState 的共享指针
  using StreamMap = std::unordered_map<stream_id_type, std::shared_ptr<MultiStreamState>>;

  /// Keeps the mapping between streams and sessions.
  class Dispatcher {
  public:
// 构造函数，接受一个 EndPoint 类型的参数并初始化成员变量
    template <typename Protocol, typename EndPointType>
    explicit Dispatcher(const EndPoint<Protocol, EndPointType> &ep)
      : _cached_token(0u, ep) {}
 // 析构函数
    ~Dispatcher();
// 创建一个新的流
    carla::streaming::Stream MakeStream();
// 关闭指定 ID 的流
    void CloseStream(carla::streaming::detail::stream_id_type id);
// 注册一个会话
    bool RegisterSession(std::shared_ptr<Session> session);
// 注销一个会话
    void DeregisterSession(std::shared_ptr<Session> session);
// 获取指定传感器 ID 的令牌
    token_type GetToken(stream_id_type sensor_id);
// 启用针对 ROS 的功能，通过传感器 ID 找到对应的流并调用其 EnableForROS 方法
    void EnableForROS(stream_id_type sensor_id) {
      auto search = _stream_map.find(sensor_id);
      if (search != _stream_map.end()) {
        search->second->EnableForROS();
      }
    }
// 禁用针对 ROS 的功能，通过传感器 ID 找到对应的流并调用其 DisableForROS 方法
    void DisableForROS(stream_id_type sensor_id) {
      auto search = _stream_map.find(sensor_id);
      if (search != _stream_map.end()) {
        search->second->DisableForROS();
      }
    }
// 检查指定传感器 ID 的流是否针对 ROS 启用，通过传感器 ID 找到对应的流并调用其 IsEnabledForROS 方法
    bool IsEnabledForROS(stream_id_type sensor_id) {
      auto search = _stream_map.find(sensor_id);
      if (search != _stream_map.end()) {
        return search->second->IsEnabledForROS();
      }
      return false;
    }

  private:

    // We use a mutex here, but we assume that sessions and streams won't be
    // created too often.
    std::mutex _mutex;
// 缓存的令牌
    token_type _cached_token;
// 存储流 ID 和对应的 MultiStreamState 共享指针的哈希表
    StreamMap _stream_map;
  };

} // namespace detail
} // namespace streaming
} // namespace carla
