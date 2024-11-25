// Copyright (c) 2021 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/MsgPack.h"

#ifdef LIBCARLA_INCLUDED_FROM_UE4
#  include "Carla/Vehicle/AckermannControllerSettings.h"
#endif // LIBCARLA_INCLUDED_FROM_UE4

namespace carla {
namespace rpc {
//负责Ackermann控制器的相关设置
  class AckermannControllerSettings {
  public:

    AckermannControllerSettings() = default;
// 构造函数，允许通过参数初始化控制器的比例、积分、微分系数
    AckermannControllerSettings(
        float speed_kp,
        float speed_ki,
        float speed_kd,
        float accel_kp,
        float accel_ki,
        float accel_kd)
      : speed_kp(speed_kp),
        speed_ki(speed_ki),
        speed_kd(speed_kd),
        accel_kp(accel_kp),
        accel_ki(accel_ki),
        accel_kd(accel_kd) {}
 // Ackermann控制器设置中的参数，速度控制的PID系数
    float speed_kp = 0.0f;
    float speed_ki = 0.0f;
    float speed_kd = 0.0f;
    float accel_kp = 0.0f;
    float accel_ki = 0.0f;
    float accel_kd = 0.0f;

#ifdef LIBCARLA_INCLUDED_FROM_UE4

// 使用UE4中FAckermannControllerSettings结构体进行初始化的构造函数
    AckermannControllerSettings(const FAckermannControllerSettings &Settings)
      : speed_kp(Settings.SpeedKp),
        speed_ki(Settings.SpeedKi),
        speed_kd(Settings.SpeedKd),
        accel_kp(Settings.AccelKp),
        accel_ki(Settings.AccelKi),
        accel_kd(Settings.AccelKd) {}

    // 操作符重载，将AckermannControllerSettings转换为UE4中的FAckermannControllerSettings结构体
    operator FAckermannControllerSettings() const {
      FAckermannControllerSettings Settings;
      Settings.SpeedKp = speed_kp;
      Settings.SpeedKi = speed_ki;
      Settings.SpeedKd = speed_kd;
      Settings.AccelKp = accel_kp;
      Settings.AccelKi = accel_ki;
      Settings.AccelKd = accel_kd;
      return Settings;
    }

#endif // LIBCARLA_INCLUDED_FROM_UE4

  // 操作符重载，比较两个AckermannControllerSettings对象是否不相等
    bool operator!=(const AckermannControllerSettings &rhs) const {
      return
          speed_kp != rhs.speed_kp ||
          speed_ki != rhs.speed_ki ||
          speed_kd != rhs.speed_kd ||
          accel_kp != rhs.accel_kp ||
          accel_ki != rhs.accel_ki ||
          accel_kd != rhs.accel_kd;
    }

    bool operator==(const AckermannControllerSettings &rhs) const {
      return !(*this != rhs);
    }

    MSGPACK_DEFINE_ARRAY(
        speed_kp,
        speed_ki,
        speed_kd,
        accel_kp,
        accel_ki,
        accel_kd
    );
  };

} // namespace rpc
} // namespace carla
