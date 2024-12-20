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
// 定义VehicleAckermannControl函数，它接受一个FVehicleAckermannControl类型的常量引用参数Control
    VehicleAckermannControl(const FVehicleAckermannControl &Control)
      : steer(Control.Steer), // 使用传入的Control对象中的Steer成员变量来初始化当前对象的steer成员变量
        steer_speed(Control.SteerSpeed),// 使用传入的Control对象中的SteerSpeed成员变量来初始化当前对象的steer_speed成员变量
        speed(Control.Speed),// 使用传入的Control对象中的Speed成员变量来初始化当前对象的speed成员变量
        acceleration(Control.Acceleration),// 使用传入的Control对象中的Acceleration成员变量来初始化当前对象的acceleration成员变量
        jerk(Control.Jerk) {}// 使用传入的Control对象中的Jerk成员变量来初始化当前对象的jerk成员变量
// 定义类型转换操作符函数，将当前对象转换为FVehicleAckermannControl类型
// 通常用于在需要的时候能够隐式或显式地把当前对象转换为对应的类型，方便在不同的场景下使用
    operator FVehicleAckermannControl() const {
      FVehicleAckermannControl Control;// 创建一个FVehicleAckermannControl类型的临时对象Control
      Control.Steer = steer;// 将当前对象的steer成员变量的值赋给临时对象Control的Steer成员变量
      Control.SteerSpeed = steer_speed;// 将当前对象的steer_speed成员变量的值赋给临时对象Control的SteerSpeed成员变量
      Control.Speed = speed; // 将当前对象的speed成员变量的值赋给临时对象Control的Speed成员变量
      Control.Acceleration = acceleration;// 将当前对象的acceleration成员变量的值赋给临时对象Control的Acceleration成员变量
      Control.Jerk = jerk;// 将当前对象的jerk成员变量的值赋给临时对象Control的Jerk成员变量
      return Control;// 返回这个已经赋值好的临时对象Control，实现了从当前类型到FVehicleAckermannControl类型的转换
    }

#endif // LIBCARLA_INCLUDED_FROM_UE4
// 定义不等比较操作符函数（!=），用于比较当前对象和传入的另一个VehicleAckermannControl类型的对象（rhs表示right hand side，即右侧操作数）是否不相等
// 该函数被声明为const成员函数，意味着它不会修改当前对象的状态
    bool operator!=(const VehicleAckermannControl &rhs) const {
      return
          steer != rhs.steer ||  // 比较当前对象的steer成员变量和传入对象rhs的steer成员变量是否不相等，如果不相等则整个表达式为真（只要其中有一个成员变量不相等，整体就不相等）
          steer_speed != rhs.steer_speed ||  // 比较当前对象的steer_speed成员变量和传入对象rhs的steer_speed成员变量是否不相等
          speed != rhs.speed ||  // 比较当前对象的speed成员变量和传入对象rhs的speed成员变量是否不相等
          acceleration != rhs.acceleration ||  // 比较当前对象的acceleration成员变量和传入对象rhs的acceleration成员变量是否不相等
          jerk != rhs.jerk;// 比较当前对象的jerk成员变量和传入对象rhs的jerk成员变量是否不相等
    }
// 定义相等比较操作符函数（==），用于判断当前对象和传入的另一个VehicleAckermannControl类型的对象是否相等
// 通过调用上面定义的!=操作符并取反来实现相等的判断逻辑，即如果两个对象通过!=操作符判断为不相等，那取反后就是相等的情况
    bool operator==(const VehicleAckermannControl &rhs) const {
      return !(*this != rhs);// 调用!=操作符判断当前对象和rhs是否不相等，然后对结果取反得到是否相等的结果
    }
// 使用MSGPACK_DEFINE_ARRAY宏（这里假设它是某个序列化相关库提供的宏，用于定义如何对一组成员变量进行序列化等相关操作）
// 来指定在序列化或者相关操作时，要处理的成员变量有steer、steer_speed、speed、acceleration、jerk这些
// 具体功能依赖于MSGPACK_DEFINE_ARRAY宏的实现逻辑，但通常用于方便地将对象转换为某种可存储或传输的格式（比如序列化到字节流等情况）
    MSGPACK_DEFINE_ARRAY(
        steer,
        steer_speed,
        speed,
        acceleration,
        jerk);
  };

} // namespace rpc
} // namespace carla
