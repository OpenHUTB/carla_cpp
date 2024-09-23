// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/client/Actor.h"

#include <functional>

namespace carla {
namespace sensor { class SensorData; }
namespace client {

  class Sensor : public Actor {
  public:

    using CallbackFunctionType = std::function<void(SharedPtr<sensor::SensorData>)>;

    using Actor::Actor;

    /// 注册一个@a回调，每次收到新的测量值时执行。
    /// 
    virtual void Listen(CallbackFunctionType callback) = 0;

    /// 停止收听新的测量结果。
    virtual void Stop() = 0;

    ///返回此传感器实例当前是否正在侦听新数据。
    virtual bool IsListening() const = 0;
  };

} // namespace client
} // namespace carla
