// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/MsgPack.h" // 引入 MsgPack 头文件

#ifdef LIBCARLA_INCLUDED_FROM_UE4 // 如果从 UE4 包含
#include <compiler/enable-ue4-macros.h> // 启用 UE4 宏
#include "Carla/Vehicle/VehicleLightState.h" // 引入车灯状态头文件
#include <compiler/disable-ue4-macros.h> // 禁用 UE4 宏
#endif // LIBCARLA_INCLUDED_FROM_UE4

namespace carla { // carla 命名空间
namespace rpc { // rpc 命名空间

  #define SET_FLAG(flag, value) flag |= static_cast<flag_type>(value) // 设置标志位宏定义

  #define FLAG_ENABLED(flag, value) flag & static_cast<flag_type>(value) // 检查标志位是否启用的宏定义

  /// 定义车辆的物理外观，通过传感器获取
  class VehicleLightState {
  public:

    using flag_type = uint32_t; // 定义标志类型为无符号整数

    /// 可以作为标志使用的枚举
    enum class LightState : flag_type {
      None         = 0, // 无灯光状态
      Position     = 0x1, // 位置灯
      LowBeam      = 0x1 << 1, // 近光灯
      HighBeam     = 0x1 << 2, // 远光灯
      Brake        = 0x1 << 3, // 刹车灯
      RightBlinker = 0x1 << 4, // 右转向灯
      LeftBlinker  = 0x1 << 5, // 左转向灯
      Reverse      = 0x1 << 6, // 倒车灯
      Fog          = 0x1 << 7, // 雾灯
      Interior     = 0x1 << 8, // 内部灯
      Special1     = 0x1 << 9, // 特殊灯光1（例如：警报灯）
      Special2     = 0x1 << 10, // 特殊灯光2
      All          = 0xFFFFFFFF // 所有灯光状态
    };

    VehicleLightState() = default; // 默认构造函数

    VehicleLightState(LightState light_state) // 构造函数，接受灯光状态枚举
      : light_state(static_cast<flag_type>(light_state)) {} // 将枚举转换为标志类型

    VehicleLightState(flag_type light_state) // 构造函数，接受标志类型
      : light_state(light_state) {} // 直接初始化标志类型

#ifdef LIBCARLA_INCLUDED_FROM_UE4

    VehicleLightState(const FVehicleLightState &InLightState) { // 构造函数，接受 UE4 的车辆灯光状态
      light_state = static_cast<flag_type>(LightState::None); // 初始化灯光状态为无
      if (InLightState.Position)     SET_FLAG(light_state, LightState::Position); // 如果位置灯开启，设置标志
      if (InLightState.LowBeam)      SET_FLAG(light_state, LightState::LowBeam); // 如果近光灯开启，设置标志
      if (InLightState.HighBeam)     SET_FLAG(light_state, LightState::HighBeam); // 如果远光灯开启，设置标志
      if (InLightState.Brake)        SET_FLAG(light_state, LightState::Brake); // 如果刹车灯开启，设置标志
      if (InLightState.RightBlinker) SET_FLAG(light_state, LightState::RightBlinker); // 如果右转向灯开启，设置标志
      if (InLightState.LeftBlinker)  SET_FLAG(light_state, LightState::LeftBlinker); // 如果左转向灯开启，设置标志
      if (InLightState.Reverse)      SET_FLAG(light_state, LightState::Reverse); // 如果倒车灯开启，设置标志
      if (InLightState.Fog)          SET_FLAG(light_state, LightState::Fog); // 如果雾灯开启，设置标志
      if (InLightState.Interior)     SET_FLAG(light_state, LightState::Interior); // 如果内部灯开启，设置标志
      if (InLightState.Special1)     SET_FLAG(light_state, LightState::Special1); // 如果特殊灯光1开启，设置标志
      if (InLightState.Special2)     SET_FLAG(light_state, LightState::Special2); // 如果特殊灯光2开启，设置标志
    }

    operator FVehicleLightState() const { // 转换运算符，将当前状态转换为 FVehicleLightState
      FVehicleLightState Lights; // 默认情况下所有灯光状态为 false
      if (FLAG_ENABLED(light_state, LightState::Position))     Lights.Position = true; // 如果位置灯开启，则设置为 true
      if (FLAG_ENABLED(light_state, LightState::LowBeam))      Lights.LowBeam = true; // 如果近光灯开启，则设置为 true
      if (FLAG_ENABLED(light_state, LightState::HighBeam))     Lights.HighBeam = true; // 如果远光灯开启，则设置为 true
      if (FLAG_ENABLED(light_state, LightState::Brake))        Lights.Brake = true; // 如果刹车灯开启，则设置为 true
      if (FLAG_ENABLED(light_state, LightState::RightBlinker)) Lights.RightBlinker = true; // 如果右转向灯开启，则设置为 true
      if (FLAG_ENABLED(light_state, LightState::LeftBlinker))  Lights.LeftBlinker = true; // 如果左转向灯开启，则设置为 true
      if (FLAG_ENABLED(light_state, LightState::Reverse))      Lights.Reverse = true; // 如果倒车灯开启，则设置为 true
      if (FLAG_ENABLED(light_state, LightState::Fog))          Lights.Fog = true; // 如果雾灯开启，则设置为 true
      if (FLAG_ENABLED(light_state, LightState::Interior))     Lights.Interior = true; // 如果内部灯开启，则设置为 true
      if (FLAG_ENABLED(light_state, LightState::Special1))     Lights.Special1 = true; // 如果特殊灯光1开启，则设置为 true
      if (FLAG_ENABLED(light_state, LightState::Special2))     Lights.Special2 = true; // 如果特殊灯光2开启，则设置为 true
      return Lights; // 返回状态
    }

#endif // LIBCARLA_INCLUDED_FROM_UE4

    /// 返回当前灯光状态作为枚举类型
    LightState GetLightStateEnum() const {
      return static_cast<LightState>(light_state); // 转换并返回灯光状态
    }

    /// 返回当前灯光状态作为值类型
    flag_type GetLightStateAsValue() const {
      return light_state; // 返回灯光状态值
    }

    /// 灯光状态标志，默认情况下全部关闭
    flag_type light_state = static_cast<flag_type>(LightState::None); // 初始化灯光状态为无

    MSGPACK_DEFINE_ARRAY(light_state) // 定义 MsgPack 数组序列化
  };

} // namespace rpc
} // namespace carla