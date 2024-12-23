// Copyright (c) 2021 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/MsgPack.h"

// 以下条件编译部分，用于判断是否从UE4中包含此头文件，如果是，则包含对应的UE4相关设置头文件
#ifdef LIBCARLA_INCLUDED_FROM_UE4
#  include "Carla/Vehicle/AckermannControllerSettings.h"
#endif // LIBCARLA_INCLUDED_FROM_UE4

namespace carla {
namespace rpc {
    // AckermannControllerSettings类，负责Ackermann控制器的相关设置
    class AckermannControllerSettings {
    public:

        // 默认构造函数，使用默认初始化方式，不进行任何参数传递时可调用此构造函数
        AckermannControllerSettings() = default;

        // 构造函数，允许通过参数初始化控制器的比例、积分、微分系数（PID系数）
        // 参数说明：
        // - speed_kp：速度控制的比例系数
        // - speed_ki：速度控制的积分系数
        // - speed_kd：速度控制的微分系数
        // - accel_kp：加速度控制的比例系数
        // - accel_ki：加速度控制的积分系数
        // - accel_kd：加速度控制的微分系数
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

        // Ackermann控制器设置中的参数，以下是速度控制的PID系数相关成员变量
        float speed_kp = 0.0f;  // 速度控制的比例系数，初始化为0.0f
        float speed_ki = 0.0f;  // 速度控制的积分系数，初始化为0.0f
        float speed_kd = 0.0f;  // 速度控制的微分系数，初始化为0.0f

        // Ackermann控制器设置中的参数，以下是加速度控制的PID系数相关成员变量
        float accel_kp = 0.0f;  // 加速度控制的比例系数，初始化为0.0f
        float accel_ki = 0.0f;  // 加速度控制的积分系数，初始化为0.0f
        float accel_kd = 0.0f;  // 加速度控制的微分系数，初始化为0.0f

#ifdef LIBCARLA_INCLUDED_FROM_UE4
        // 使用UE4中FAckermannControllerSettings结构体进行初始化的构造函数
        // 它会将UE4结构体中的对应系数值赋给当前类的成员变量，实现从UE4相关设置到本类设置的转换
        AckermannControllerSettings(const FAckermannControllerSettings &Settings)
            : speed_kp(Settings.SpeedKp),
            speed_ki(Settings.SpeedKi),
            speed_kd(Settings.SpeedKd),
            accel_kp(Settings.AccelKp),
            accel_ki(Settings.AccelKi),
            accel_kd(Settings.AccelKd) {}

        // 操作符重载，将AckermannControllerSettings转换为UE4中的FAckermannControllerSettings结构体
        // 方便在需要使用UE4结构体的地方，能够直接将本类对象转换为对应的UE4结构体形式进行使用
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
        // 通过逐个比较成员变量（速度和加速度控制的PID系数）的值来判断两个对象是否不同
        bool operator!=(const AckermannControllerSettings &rhs) const {
            return
                speed_kp!= rhs.speed_kp ||
                speed_ki!= rhs.speed_ki ||
                speed_kd!= rhs.speed_kd ||
                accel_kp!= rhs.accel_kp ||
                accel_ki!= rhs.accel_ki ||
                accel_kd!= rhs.accel_kd;
        }

        // 操作符重载，判断两个AckermannControllerSettings对象是否相等，通过调用!=操作符的取反来实现
        bool operator==(const AckermannControllerSettings &rhs) const {
            return!(*this!= rhs);
        }

        // 使用MsgPack库来定义如何将此类的对象序列化为数组形式，便于网络传输或存储等操作
        // 按照顺序将成员变量（速度和加速度控制的PID系数）组成一个数组进行定义
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
