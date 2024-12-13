// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
// Copyright (c) 2019 Intel Corporation
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/MsgPack.h"
#include "carla/geom/Location.h"
#include "carla/geom/Vector2D.h"
#include "carla/rpc/GearPhysicsControl.h"
#include "carla/rpc/WheelPhysicsControl.h"

#include <string>
#include <vector>

namespace carla {
namespace rpc { 
// 用于控制车辆的物理属性
 class VehiclePhysicsControl {
  public:

    VehiclePhysicsControl() = default;
 // 带参数的构造函数，初始化所有车辆物理属性
    VehiclePhysicsControl(
        const std::vector<carla::geom::Vector2D> &in_torque_curve,
        float in_max_rpm,
        float in_moi,
        float in_damping_rate_full_throttle,
        float in_damping_rate_zero_throttle_clutch_engaged,
        float in_damping_rate_zero_throttle_clutch_disengaged,

        bool in_use_gear_autobox,
        float in_gear_switch_time,
        float in_clutch_strength,
        float in_final_ratio,
        std::vector<GearPhysicsControl> &in_forward_gears,

        float in_mass,
        float in_drag_coefficient,
        geom::Location in_center_of_mass,
        const std::vector<carla::geom::Vector2D> &in_steering_curve,
        std::vector<WheelPhysicsControl> &in_wheels,
        bool in_use_sweep_wheel_collision)
      : torque_curve(in_torque_curve),
        max_rpm(in_max_rpm),
        moi(in_moi),
        damping_rate_full_throttle(in_damping_rate_full_throttle),
        damping_rate_zero_throttle_clutch_engaged(in_damping_rate_zero_throttle_clutch_engaged),
        damping_rate_zero_throttle_clutch_disengaged(in_damping_rate_zero_throttle_clutch_disengaged),
        use_gear_autobox(in_use_gear_autobox),
        gear_switch_time(in_gear_switch_time),
        clutch_strength(in_clutch_strength),
        final_ratio(in_final_ratio),
        forward_gears(in_forward_gears),
        mass(in_mass),
        drag_coefficient(in_drag_coefficient),
        center_of_mass(in_center_of_mass),
        steering_curve(in_steering_curve),
        wheels(in_wheels),
        use_sweep_wheel_collision(in_use_sweep_wheel_collision) {}
// 获取前进档位的控制参数
    const std::vector<GearPhysicsControl> &GetForwardGears() const {
      return forward_gears;
    }
// 设置前进档位的控制参数
    void SetForwardGears(std::vector<GearPhysicsControl> &in_forward_gears) {
      forward_gears = in_forward_gears;
    }
 // 获取车轮的物理控制参数
    const std::vector<WheelPhysicsControl> &GetWheels() const {
      return wheels;
    }

    void SetWheels(std::vector<WheelPhysicsControl> &in_wheels) {
      wheels = in_wheels;
    }
// 获取扭矩曲线
    const std::vector<geom::Vector2D> &GetTorqueCurve() const {
      return torque_curve;
    }
 // 设置扭矩曲线
    void SetTorqueCurve(std::vector<geom::Vector2D> &in_torque_curve) {
      torque_curve = in_torque_curve;
    }
  // 获取转向曲线
    const std::vector<geom::Vector2D> &GetSteeringCurve() const {
      return steering_curve;
    }
// 设置转向曲线
    void SetSteeringCurve(std::vector<geom::Vector2D> &in_steering_curve) {
      steering_curve = in_steering_curve;
    }
// 设置是否使用车轮碰撞检测
    void SetUseSweepWheelCollision(bool in_sweep) {
      use_sweep_wheel_collision = in_sweep;
    }
    // 获取是否使用车轮碰撞检测
    bool GetUseSweepWheelCollision() {
      return use_sweep_wheel_collision;
    }
    // 默认的扭矩曲线，表示转速与扭矩的关系
    std::vector<geom::Vector2D> torque_curve = {geom::Vector2D(0.0f, 500.0f), geom::Vector2D(5000.0f, 500.0f)};// 最大转速
    float max_rpm = 5000.0f; // 动能矩
    float moi = 1.0f;  // 全油门时的阻尼系数
    float damping_rate_full_throttle = 0.15f;  // 零油门且离合器接合时的阻尼系数
    float damping_rate_zero_throttle_clutch_engaged = 2.0f;// 零油门且离合器分离时的阻尼系数
    float damping_rate_zero_throttle_clutch_disengaged = 0.35f;
// 是否使用自动变速箱
    bool use_gear_autobox = true; // 换挡时间
    float gear_switch_time = 0.5f;  // 离合器
    float clutch_strength = 10.0f; // 最终传动比
    float final_ratio = 4.0f; // 前进档位的控制参数
    std::vector<GearPhysicsControl> forward_gears;

    float mass = 1000.0f;
    float drag_coefficient = 0.3f;
    geom::Location center_of_mass;
 // 转向曲线，表示转速与转向角度的关系
    std::vector<geom::Vector2D> steering_curve = {geom::Vector2D(0.0f, 1.0f), geom::Vector2D(10.0f, 0.5f)};// 车轮物理控制
    std::vector<WheelPhysicsControl> wheels;
 // 是否使用车轮碰撞检测
    bool use_sweep_wheel_collision = false;
  // 重载不等号运算符，比较两个VehiclePhysicsControl对象是否不同
    bool operator!=(const VehiclePhysicsControl &rhs) const {
      return
        max_rpm != rhs.max_rpm ||  // 比较最大转速
        moi != rhs.moi ||  // 比较转动惯量

        damping_rate_full_throttle ！ =rhs.damping_rate_full_throttle || // 比较全油门下的阻尼率
        damping_rate_zero_throttle_clutch_engaged != rhs.damping_rate_zero_throttle_clutch_engaged ||
        damping_rate_zero_throttle_clutch_disengaged != rhs.damping_rate_zero_throttle_clutch_disengaged ||
//比较不同油门下的阻尼率
        use_gear_autobox != rhs.use_gear_autobox || // 比较是否使用自动变速箱
        gear_switch_time != rhs.gear_switch_time || // 比较换挡时间
        clutch_strength != rhs.clutch_strength || // 比较离合器强度
        final_ratio != rhs.final_ratio ||  // 比较最终传动比
        forward_gears != rhs.forward_gears ||   // 比较前进挡列表

        mass != rhs.mass ||   // 比较车辆质量
        drag_coefficient != rhs.drag_coefficient ||  // 比较空气阻力系数
        steering_curve != rhs.steering_curve ||  // 比较转向曲线
        center_of_mass != rhs.center_of_mass ||  // 比较质心位置
        wheels != rhs.wheels ||   // 比较车轮列表
        use_sweep_wheel_collision != rhs.use_sweep_wheel_collision;   // 比较是否使用车轮扫掠碰撞
    }

    bool operator==(const VehiclePhysicsControl &rhs) const {
      return !(*this != rhs);
    }

#ifdef LIBCARLA_INCLUDED_FROM_UE4
// 构造函数：将一个 FVehiclePhysicsControl 对象转换为 VehiclePhysicsControl 对象
    VehiclePhysicsControl(const FVehiclePhysicsControl &Control) {
// 初始化引擎设置
      // Engine Setup
      torque_curve = std::vector<carla::geom::Vector2D>();
      TArray<FRichCurveKey> TorqueCurveKeys = Control.TorqueCurve.GetCopyOfKeys();
      for (int32 KeyIdx = 0; KeyIdx < TorqueCurveKeys.Num(); KeyIdx++) {
        geom::Vector2D point(TorqueCurveKeys[KeyIdx].Time, TorqueCurveKeys[KeyIdx].Value);
        torque_curve.push_back(point);
      }
      max_rpm = Control.MaxRPM;
      moi = Control.MOI;
      damping_rate_full_throttle = Control.DampingRateFullThrottle;
      damping_rate_zero_throttle_clutch_engaged = Control.DampingRateZeroThrottleClutchEngaged;
      damping_rate_zero_throttle_clutch_disengaged = Control.DampingRateZeroThrottleClutchDisengaged;

      // Transmission Setup
      use_gear_autobox = Control.bUseGearAutoBox;
      gear_switch_time = Control.GearSwitchTime;
      clutch_strength = Control.ClutchStrength;
      final_ratio = Control.FinalRatio;
      forward_gears = std::vector<GearPhysicsControl>();
      for (const auto &Gear : Control.ForwardGears) {
        forward_gears.push_back(GearPhysicsControl(Gear));
      }

      // Vehicle Setup
      mass = Control.Mass;
      drag_coefficient = Control.DragCoefficient;

      steering_curve = std::vector<carla::geom::Vector2D>();
      TArray<FRichCurveKey> SteeringCurveKeys = Control.SteeringCurve.GetCopyOfKeys();
      for (int32 KeyIdx = 0; KeyIdx < SteeringCurveKeys.Num(); KeyIdx++) {
        geom::Vector2D point(SteeringCurveKeys[KeyIdx].Time, SteeringCurveKeys[KeyIdx].Value);
        steering_curve.push_back(point);
      }

      center_of_mass = Control.CenterOfMass;

      // Wheels Setup
      wheels = std::vector<WheelPhysicsControl>();
      for (const auto &Wheel : Control.Wheels) {
        wheels.push_back(WheelPhysicsControl(Wheel));
      }

      use_sweep_wheel_collision = Control.UseSweepWheelCollision;
    }

    operator FVehiclePhysicsControl() const {
      FVehiclePhysicsControl Control;

      // Engine Setup
      FRichCurve TorqueCurve;
      for (const auto &point : torque_curve) {
        TorqueCurve.AddKey(point.x, point.y);
      }
      Control.TorqueCurve = TorqueCurve;
      Control.MaxRPM = max_rpm;
      Control.MOI = moi;
      Control.DampingRateFullThrottle = damping_rate_full_throttle;
      Control.DampingRateZeroThrottleClutchEngaged = damping_rate_zero_throttle_clutch_engaged;
      Control.DampingRateZeroThrottleClutchDisengaged = damping_rate_zero_throttle_clutch_disengaged;

      // Transmission Setup
      Control.bUseGearAutoBox = use_gear_autobox;
      Control.GearSwitchTime = gear_switch_time;
      Control.ClutchStrength = clutch_strength;
      Control.FinalRatio = final_ratio;
      TArray<FGearPhysicsControl> ForwardGears;
      for (const auto &gear : forward_gears) {
        ForwardGears.Add(FGearPhysicsControl(gear));
      }
      Control.ForwardGears = ForwardGears;


      // Vehicle Setup
      Control.Mass = mass;
      Control.DragCoefficient = drag_coefficient;

      // Transmission Setup
      FRichCurve SteeringCurve;
      for (const auto &point : steering_curve) {
        SteeringCurve.AddKey(point.x, point.y);
      }
      Control.SteeringCurve = SteeringCurve;

      Control.CenterOfMass = center_of_mass;

      // Wheels Setup
      TArray<FWheelPhysicsControl> Wheels;
      for (const auto &wheel : wheels) {
        Wheels.Add(FWheelPhysicsControl(wheel));
      }
      Control.Wheels = Wheels;

      Control.UseSweepWheelCollision = use_sweep_wheel_collision;

      return Control;
    }

#endif

    MSGPACK_DEFINE_ARRAY(torque_curve,
        max_rpm,
        moi,
        damping_rate_full_throttle,
        damping_rate_zero_throttle_clutch_engaged,
        damping_rate_zero_throttle_clutch_disengaged,
        use_gear_autobox,
        gear_switch_time,
        clutch_strength,
        final_ratio,
        forward_gears,
        mass,
        drag_coefficient,
        center_of_mass,
        steering_curve,
        wheels,
        use_sweep_wheel_collision);
  };

} // namespace rpc
} // namespace carla
