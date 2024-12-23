// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once 
#include "carla/MsgPack.h"  // 引入 MsgPack 库头文件，用于序列化

#ifdef LIBCARLA_INCLUDED_FROM_UE4  // 
#include <compiler/enable-ue4-macros.h>  
#include "Carla/Vehicle/VehicleControl.h"  // 引入与车辆控制相关的头文件
#include <compiler/disable-ue4-macros.h> 
#endif // LIBCARLA_INCLUDED_FROM_UE4  // 结束条件编译判断

namespace carla {  // carla 命名空间
namespace rpc {  // rpc 命名空间

  // 定义一个车辆控制类 VehicleControl
  class VehicleControl {
  public:

    VehicleControl() = default;  // 默认构造函数，使用默认成员初始化

    // 带参数的构造函数，初始化所有成员变量
    VehicleControl(
        float in_throttle,  // 油门
        float in_steer,  // 转向
        float in_brake,  // 刹车
        bool in_hand_brake,  // 手刹
        bool in_reverse,  // 倒车
        bool in_manual_gear_shift,  // 是否手动换挡
        int32_t in_gear)  // 当前挡位
      : throttle(in_throttle),  // 初始化油门
        steer(in_steer),  // 初始化转向
        brake(in_brake),  // 初始化刹车
        hand_brake(in_hand_brake),  // 初始化手刹
        reverse(in_reverse),  // 初始化倒车
        manual_gear_shift(in_manual_gear_shift),  // 初始化手动换挡
        gear(in_gear) {}  // 初始化挡位

    // 定义所有成员变量
    float throttle = 0.0f;  // 油门，默认值为 0.0
    float steer = 0.0f;  // 转向，默认值为 0.0
    float brake = 0.0f;  // 刹车，默认值为 0.0
    bool hand_brake = false;  // 手刹，默认值为 false
    bool reverse = false;  // 倒车，默认值为 false
    bool manual_gear_shift = false;  // 是否手动换挡，默认值为 false
    int32_t gear = 0;  // 当前挡位，默认值为 0

#ifdef LIBCARLA_INCLUDED_FROM_UE4  // 如果是从 UE4 环境中包含此代码
    // 从 FVehicleControl 转换为 VehicleControl 的构造函数
    VehicleControl(const FVehicleControl &Control)
      : throttle(Control.Throttle),  // 初始化油门
        steer(Control.Steer),  // 初始化转向
        brake(Control.Brake),  // 初始化刹车
        hand_brake(Control.bHandBrake),  // 初始化手刹
        reverse(Control.bReverse),  // 初始化倒车
        manual_gear_shift(Control.bManualGearShift),  // 初始化手动换挡
        gear(Control.Gear) {}  // 初始化挡位

    // 将 VehicleControl 转换为 FVehicleControl 的操作符
    operator FVehicleControl() const {
      FVehicleControl Control;  // 创建一个 FVehicleControl 对象
      Control.Throttle = throttle;  // 设置油门
      Control.Steer = steer;  // 设置转向
      Control.Brake = brake;  // 设置刹车
      Control.bHandBrake = hand_brake;  // 设置手刹
      Control.bReverse = reverse;  // 设置倒车
      Control.bManualGearShift = manual_gear_shift;  // 设置手动换挡
      Control.Gear = gear;  // 设置挡位
      return Control;  // 返回转换后的 FVehicleControl 对象
    }

#endif // LIBCARLA_INCLUDED_FROM_UE4  // 结束 UE4 环境的条件编译

    // 定义不等于操作符，用于比较两个 VehicleControl 对象是否不同
    bool operator!=(const VehicleControl &rhs) const {
      return
          throttle != rhs.throttle ||  // 油门不相等
          steer != rhs.steer ||  // 转向不相等
          brake != rhs.brake ||  // 刹车不相等
          hand_brake != rhs.hand_brake ||  // 手刹不相等
          reverse != rhs.reverse ||  // 倒车不相等
          manual_gear_shift != rhs.manual_gear_shift ||  // 是否手动换挡不相等
          gear != rhs.gear;  // 挡位不相等
    }

    // 定义等于操作符，用于比较两个 VehicleControl 对象是否相等
    bool operator==(const VehicleControl &rhs) const {
      return !(*this != rhs);  // 如果不等于返回 false，则相等
    }

    // 定义序列化方式，使用 MsgPack 库
    MSGPACK_DEFINE_ARRAY(
        throttle,  // 序列化油门
        steer,  // 序列化转向
        brake,  // 序列化刹车
        hand_brake,  // 序列化手刹
        reverse,  // 序列化倒车
        manual_gear_shift,  // 序列化手动换挡
        gear);  // 序列化挡位
  };

} // namespace rpc  // 结束 rpc 命名空间
} // namespace carla  // 结束 carla 命名空间