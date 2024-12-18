// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// 车辆遥感数据
// 
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/MsgPack.h"
#include "carla/rpc/WheelTelemetryData.h"

#include <vector>

namespace carla {
namespace rpc {
  class VehicleTelemetryData {
  public:

    VehicleTelemetryData() = default;

    VehicleTelemetryData(
        float speed,      // 速度
        float steer,      // 驾驶方向
        float throttle,   // 油门
        float brake,      // 刹车
        float engine_rpm, // 引擎 每分钟转速(revolutions per minute)
        int32_t gear,     // 档位
        float drag,
        std::vector<WheelTelemetryData> wheels)
      : speed(speed),
        steer(steer),
        throttle(throttle),
        brake(brake),
        engine_rpm(engine_rpm),
        gear(gear),
        drag(drag),
        wheels(wheels) {}

    // 获得车轮
    const std::vector<WheelTelemetryData> &GetWheels() const {
      return wheels;
    }

    // 设置车轮
    void SetWheels(std::vector<WheelTelemetryData> &in_wheels) {
      wheels = in_wheels;
    }

    float speed = 0.0f;
    float steer = 0.0f;
    float throttle = 0.0f;
    float brake = 0.0f;
    float engine_rpm = 0.0f;
    int32_t gear = 0.0f;
    float drag = 0.0f;
    std::vector<WheelTelemetryData> wheels;

    bool operator!=(const VehicleTelemetryData &rhs) const {
      return
        speed != rhs.speed ||
        steer != rhs.steer ||
        throttle != rhs.throttle ||
        brake != rhs.brake ||
        engine_rpm != rhs.engine_rpm ||
        gear != rhs.gear ||
        drag != rhs.drag ||
        wheels != rhs.wheels;
    }

    bool operator==(const VehicleTelemetryData &rhs) const {
      return !(*this != rhs);
    }

#ifdef LIBCARLA_INCLUDED_FROM_UE4

    VehicleTelemetryData(const FVehicleTelemetryData &TelemetryData) {
      speed = TelemetryData.Speed;
      steer = TelemetryData.Steer;
      throttle = TelemetryData.Throttle;
      brake = TelemetryData.Brake;
      engine_rpm = TelemetryData.EngineRPM;
      gear = TelemetryData.Gear;
      drag = TelemetryData.Drag;

      // Wheels Setup
      wheels = std::vector<WheelTelemetryData>();
      for (const auto &Wheel : TelemetryData.Wheels) {
        wheels.push_back(WheelTelemetryData(Wheel));
      }
    }

    operator FVehicleTelemetryData() const {
      FVehicleTelemetryData TelemetryData;

      TelemetryData.Speed = speed;
      TelemetryData.Steer = steer;
      TelemetryData.Throttle = throttle;
      TelemetryData.Brake = brake;
      TelemetryData.EngineRPM = engine_rpm;
      TelemetryData.Gear = gear;
      TelemetryData.Drag = drag;

      TArray<FWheelTelemetryData> Wheels;
      for (const auto &wheel : wheels) {
        Wheels.Add(FWheelTelemetryData(wheel));
      }
      TelemetryData.Wheels = Wheels;

      return TelemetryData;
    }

#endif

    MSGPACK_DEFINE_ARRAY(speed,
        steer,
        throttle,
        brake,
        engine_rpm,
        gear,
        drag,
        wheels);
  };

} // namespace rpc
} // namespace carla
