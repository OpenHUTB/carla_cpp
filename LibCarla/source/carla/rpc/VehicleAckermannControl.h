// Copyright (c) 2021 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/MsgPack.h"

#ifdef LIBCARLA_INCLUDED_FROM_UE4
#  include "Carla/Vehicle/VehicleAckermannControl.h"
#endif // LIBCARLA_INCLUDED_FROM_UE4

namespace carla {
namespace rpc {
//定义一个名为VehicleAckermannControl的类，它可能用于控制车辆的阿克曼转向相关操作等
  class VehicleAckermannControl {
  public:
//默认构造函数，使用默认的初始化方式，这里不进行额外的初始化操作
    VehicleAckermannControl() = default;
    //带参数的构造函数，用于初始化VehicleAckermannControl类的对象
    // 参数in_steer用于初始化车辆的转向角度相关值
    // 参数in_steer_speed用于初始化转向速度相关值
    // 参数in_speed用于初始化车辆行驶速度相关值
    // 参数in_acceleration用于初始化车辆加速度相关值
    // 参数in_jerk用于初始化车辆加加速度（急动度）相关值
    VehicleAckermannControl(
        float in_steer,
        float in_steer_speed,
        float in_speed,
        float in_acceleration,
        float in_jerk)
      : steer(in_steer),
        steer_speed(in_steer_speed),
        speed(in_speed),
        acceleration(in_acceleration),
        jerk(in_jerk) {}

    float steer = 0.0f;// 定义一个成员变量steer，用于表示车辆的转向角度，初始化为0.0f（浮点数类型的0值）
    float steer_speed = 0.0f;//定义一个成员变量steer_speed，用于表示车辆的转向速度，初始化为0.0f
    float speed = 0.0f;//定义一个成员变量speed，用于表示车辆的行驶速度，初始化为0.0f
    float acceleration = 0.0f;//定义一个成员变量acceleration，用于表示车辆的加速度，初始化为0.0f
    float jerk = 0.0f;//定义一个成员变量jerk，用于表示车辆的加加速度（急动度），初始化为0.0f

#ifdef LIBCARLA_INCLUDED_FROM_UE4

    VehicleAckermannControl(const FVehicleAckermannControl &Control)
      : steer(Control.Steer),
        steer_speed(Control.SteerSpeed),
        speed(Control.Speed),
        acceleration(Control.Acceleration),
        jerk(Control.Jerk) {}

    operator FVehicleAckermannControl() const {
      FVehicleAckermannControl Control;
      Control.Steer = steer;
      Control.SteerSpeed = steer_speed;
      Control.Speed = speed;
      Control.Acceleration = acceleration;
      Control.Jerk = jerk;
      return Control;
    }

#endif // LIBCARLA_INCLUDED_FROM_UE4

    bool operator!=(const VehicleAckermannControl &rhs) const {
      return
          steer != rhs.steer ||
          steer_speed != rhs.steer_speed ||
          speed != rhs.speed ||
          acceleration != rhs.acceleration ||
          jerk != rhs.jerk;
    }

    bool operator==(const VehicleAckermannControl &rhs) const {
      return !(*this != rhs);
    }

    MSGPACK_DEFINE_ARRAY(
        steer,
        steer_speed,
        speed,
        acceleration,
        jerk);
  };

} // namespace rpc
} // namespace carla
