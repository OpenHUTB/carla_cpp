// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/client/Sensor.h"
#include <bitset>

namespace carla {
namespace client {

  class ServerSideSensor final : public Sensor {
  public:

    using Sensor::Sensor;

    ~ServerSideSensor();

    /// 注册一个 @a 回调，每次收到新的测量值时执行。
    ///
    /// @warning 在已在监听的传感器上调用此函数会窃取先前设置的回调中的数据流。
    /// 请注意，多个传感器实例（即使在不同的进程中）可能指向模拟器中的同一传感器。
    void Listen(CallbackFunctionType callback) override;

    /// 停止监听新的测量结果。
    void Stop() override;

    /// 返回此传感器实例当前是否正在监听模拟器中的相关传感器。
    bool IsListening() const override {
      return listening_mask.test(0);
    }

    /// 监听 fr
    void ListenToGBuffer(uint32_t GBufferId, CallbackFunctionType callback);

    /// 停止监听特定的 gbuffer 流。
    void StopGBuffer(uint32_t GBufferId);

    inline bool IsListeningGBuffer(uint32_t id) const {
      return listening_mask.test(id + 1);
    }

    /// 启用此传感器以进行 ROS2 发布
    void EnableForROS();

    /// 禁用此传感器以进行 ROS2 发布
    void DisableForROS();

    /// 如果传感器正在为 ROS2 发布，则返回
    bool IsEnabledForROS();

    /// 通过该传感器发送数据
    void Send(std::string message);

    /// @copydoc Actor::Destroy()
    ///
    /// 另外停止监听。
    bool Destroy() override;

  private:

    std::bitset<16> listening_mask;
  };

} // namespace client
} // namespace carla
