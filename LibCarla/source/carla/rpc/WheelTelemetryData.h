// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once  // 防止头文件重复包含

#include "carla/MsgPack.h"  // 包含 MsgPack 库的头文件

namespace carla {  // 定义 carla 命名空间
namespace rpc {  // 定义 rpc 子命名空间

  class WheelTelemetryData {  // 定义 WheelTelemetryData 类
  public:

    WheelTelemetryData() = default;  // 默认构造函数

    // 带参数的构造函数，用于初始化类成员
    WheelTelemetryData(
    float tire_friction,          // 轮胎摩擦力
    float lat_slip,              // 侧滑
    float long_slip,             // 纵向滑动
    float omega,                 // 角速度
    float tire_load,             // 轮胎负载
    float normalized_tire_load,  // 归一化轮胎负载
    float torque,                // 扭矩
    float long_force,            // 纵向力
    float lat_force,             // 横向力
    float normalized_long_force,  // 归一化纵向力
    float normalized_lat_force)   // 归一化横向力
      : tire_friction(tire_friction),  // 初始化轮胎摩擦力
        lat_slip(lat_slip),              // 初始化侧滑
        long_slip(long_slip),            // 初始化纵向滑动
        omega(omega),                    // 初始化角速度
        tire_load(tire_load),            // 初始化轮胎负载
        normalized_tire_load(normalized_tire_load),  // 初始化归一化轮胎负载
        torque(torque),                  // 初始化扭矩
        long_force(long_force),          // 初始化纵向力
        lat_force(lat_force),            // 初始化横向力
        normalized_long_force(normalized_long_force),  // 初始化归一化纵向力
        normalized_lat_force(normalized_lat_force) {} // 初始化归一化横向力

    // 类成员变量定义及默认值
    float tire_friction = 0.0f;          // 轮胎摩擦力
    float lat_slip = 0.0f;                // 侧滑
    float long_slip = 0.0f;               // 纵向滑动
    float omega = 0.0f;                   // 角速度
    float tire_load = 0.0f;               // 轮胎负载
    float normalized_tire_load = 0.0f;    // 归一化轮胎负载
    float torque = 0.0f;                  // 扭矩
    float long_force = 0.0f;              // 纵向力
    float lat_force = 0.0f;               // 横向力
    float normalized_long_force = 0.0f;   // 归一化纵向力
    float normalized_lat_force = 0.0f;    // 归一化横向力

    // 不同对象比较运算符重载
    bool operator!=(const WheelTelemetryData &rhs) const {
      return
      tire_friction != rhs.tire_friction ||  // 比较轮胎摩擦力
      lat_slip != rhs.lat_slip ||              // 比较侧滑
      long_slip != rhs.long_slip ||            // 比较纵向滑动
      omega != rhs.omega ||                    // 比较角速度
      tire_load != rhs.tire_load ||            // 比较轮胎负载
      normalized_tire_load != rhs.normalized_tire_load ||  // 比较归一化轮胎负载
      torque != rhs.torque ||                  // 比较扭矩
      long_force != rhs.long_force ||          // 比较纵向力
      lat_force != rhs.lat_force ||            // 比较横向力
      normalized_long_force != rhs.normalized_long_force ||  // 比较归一化纵向力
      normalized_lat_force != rhs.normalized_lat_force;       // 比较归一化横向力
    }

    // 相等运算符重载
    bool operator==(const WheelTelemetryData &rhs) const {
      return !(*this != rhs);  // 如果不相等则返回 false
    }

#ifdef LIBCARLA_INCLUDED_FROM_UE4  // 如果定义了 LIBCARLA_INCLUDED_FROM_UE4

    // 构造函数，将 FWheelTelemetryData 转换为 WheelTelemetryData
    WheelTelemetryData(const FWheelTelemetryData &TelemetryData)
      : tire_friction(TelemetryData.TireFriction),  // 初始化轮胎摩擦力
        lat_slip(TelemetryData.LatSlip),              // 初始化侧滑
        long_slip(TelemetryData.LongSlip),            // 初始化纵向滑动
        omega(TelemetryData.Omega),                    // 初始化角速度
        tire_load(TelemetryData.TireLoad),            // 初始化轮胎负载
        normalized_tire_load(TelemetryData.NormalizedTireLoad),  // 初始化归一化轮胎负载
        torque(TelemetryData.Torque),                  // 初始化扭矩
        long_force(TelemetryData.LongForce),          // 初始化纵向力
        lat_force(TelemetryData.LatForce),            // 初始化横向力
        normalized_long_force(TelemetryData.NormalizedLongForce),  // 初始化归一化纵向力
        normalized_lat_force(TelemetryData.NormalizedLatForce) {} // 初始化归一化横向力

    // 类型转换操作符，将 WheelTelemetryData 转换为 FWheelTelemetryData
    operator FWheelTelemetryData() const {
      FWheelTelemetryData TelemetryData;  // 创建 FWheelTelemetryData 对象
      TelemetryData.TireFriction = tire_friction;  // 设置轮胎摩擦力
      TelemetryData.LatSlip = lat_slip;              // 设置侧滑
      TelemetryData.LongSlip = long_slip;            // 设置纵向滑动
      TelemetryData.Omega = omega;                    // 设置角速度
      TelemetryData.TireLoad = tire_load;            // 设置轮胎负载
      TelemetryData.NormalizedTireLoad = normalized_tire_load;  // 设置归一化轮胎负载
      TelemetryData.Torque = torque;                  // 设置扭矩
      TelemetryData.LongForce = long_force;          // 设置纵向力
      TelemetryData.LatForce = lat_force;            // 设置横向力
      TelemetryData.NormalizedLongForce = normalized_long_force;  // 设置归一化纵向力
      TelemetryData.NormalizedLatForce = normalized_lat_force;    // 设置归一化横向力

      return TelemetryData;  // 返回 FWheelTelemetryData 对象
    }
#endif

    // 使用 MsgPack 库定义序列化数据结构
    MSGPACK_DEFINE_ARRAY(tire_friction,
    lat_slip,
    long_slip,
    omega,
    tire_load,
    normalized_tire_load,
    torque,
    long_force,
    lat_force,
    normalized_long_force,
    normalized_lat_force)
  };

} // namespace rpc
} // namespace carla
