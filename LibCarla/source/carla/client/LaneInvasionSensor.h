// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/client/ClientSideSensor.h"

#include <atomic>

namespace carla {
namespace client {

  class Map;
  class Vehicle;

  class LaneInvasionSensor final : public ClientSideSensor {
  public:

    using ClientSideSensor::ClientSideSensor;

    ~LaneInvasionSensor();

    /// 注册一个 @a 回调，每次收到新的测量值时执行。
    ///
    /// @warning 在已在监听的传感器上调用此函数会窃取先前设置的回调中的数据流。
    /// 请注意，多个传感器实例（即使在不同的进程中）可能指向模拟器中的同一传感器。 
    void Listen(CallbackFunctionType callback) override;

    /// 停止监听新的测量结果。
    void Stop() override;

    /// 返回此传感器实例当前是否正在监听模拟器中的相关传感器。
    bool IsListening() const override {
      return _callback_id != 0u;
    }

  private:

    std::atomic_size_t _callback_id{0u};
  };

} // namespace client
} // namespace carla
