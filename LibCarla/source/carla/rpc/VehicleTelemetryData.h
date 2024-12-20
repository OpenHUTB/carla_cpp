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
// 定义一个名为VehicleTelemetryData的函数，它接受一个FVehicleTelemetryData类型的常量引用TelemetryData作为参数
// 这个函数可能用于从传入的TelemetryData对象中提取相关数据来初始化当前对象的成员变量等操作
    VehicleTelemetryData(const FVehicleTelemetryData &TelemetryData) {
 // 将传入的TelemetryData对象中的速度值赋给当前对象的speed成员变量，实现数据的拷贝赋值
      speed = TelemetryData.Speed;
 // 将传入的TelemetryData对象中的转向值赋给当前对象的steer成员变量
      steer = TelemetryData.Steer;
// 将传入的TelemetryData对象中的油门值赋给当前对象的throttle成员变量
      throttle = TelemetryData.Throttle;
 // 将传入的TelemetryData对象中的刹车值赋给当前对象的brake成员变量
      brake = TelemetryData.Brake;
// 将传入的TelemetryData对象中的发动机转速值赋给当前对象的engine_rpm成员变量
      engine_rpm = TelemetryData.EngineRPM;
 // 将传入的TelemetryData对象中的挡位值赋给当前对象的gear成员变量
      gear = TelemetryData.Gear;
// 将传入的TelemetryData对象中的阻力值赋给当前对象的drag成员变量
      drag = TelemetryData.Drag;

// Wheels Setup，以下是对车轮相关数据的设置操作
      wheels = std::vector<WheelTelemetryData>();
// 开始遍历传入的TelemetryData对象中的车轮数据集合（可能是一个容器类型存储多个车轮相关的数据结构）
      for (const auto &Wheel : TelemetryData.Wheels) {
 // 针对每个车轮数据，创建一个WheelTelemetryData类型的对象（可能是通过拷贝构造等方式），并将其添加到当前对象的wheels成员变量（一个向量容器）中
        wheels.push_back(WheelTelemetryData(Wheel));
      }
    }

// 定义一个类型转换操作符函数，将当前对象转换为FVehicleTelemetryData类型，通常用于在需要该类型的地方隐式或显式地进行类型转换
    operator FVehicleTelemetryData() const {
   // 创建一个FVehicleTelemetryData类型的临时对象TelemetryData，后续会将当前对象的数据填充到这个临时对象中，以便返回合适类型的数据
      FVehicleTelemetryData TelemetryData;

  // 将当前对象的速度值赋给临时对象TelemetryData的Speed成员变量，以便返回完整的符合FVehicleTelemetryData类型结构的数据
      TelemetryData.Speed = speed;
// 将当前对象的转向值赋给临时对象TelemetryData的Steer成员变量
      TelemetryData.Steer = steer;
// 将当前对象的油门值赋给临时对象TelemetryData的Throttle成员变量
      TelemetryData.Throttle = throttle;
// 将当前对象的刹车值赋给临时对象TelemetryData的Brake成员变量
      TelemetryData.Brake = brake;
// 将当前对象的发动机转速值赋给临时对象TelemetryData的EngineRPM成员变量
      TelemetryData.EngineRPM = engine_rpm;
// 将当前对象的挡位值赋给临时对象TelemetryData的Gear成员变量
      TelemetryData.Gear = gear;
 // 将当前对象的阻力值赋给临时对象TelemetryData的Drag成员变量
      TelemetryData.Drag = drag;

// 创建一个TArray类型（可能是特定库中定义的数组类型）用于存储车轮相关的数据，后续会将当前对象中的车轮数据转换后填充进去
      TArray<FWheelTelemetryData> Wheels;
// 遍历当前对象的wheels成员变量（一个存储车轮数据的向量容器）
      for (const auto &wheel : wheels) {
 // 针对每个车轮数据，创建一个FWheelTelemetryData类型的对象（可能进行了合适的数据转换等操作），并添加到Wheels数组中
        Wheels.Add(FWheelTelemetryData(wheel));
      }
// 将填充好车轮数据的Wheels数组赋给临时对象TelemetryData的Wheels成员变量，使临时对象的结构完整
      TelemetryData.Wheels = Wheels;
  // 返回填充好数据的临时对象TelemetryData，完成从当前对象到FVehicleTelemetryData类型的转换
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
