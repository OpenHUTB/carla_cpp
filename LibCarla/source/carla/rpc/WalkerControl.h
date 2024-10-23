// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/MsgPack.h"

#ifdef LIBCARLA_INCLUDED_FROM_UE4
  #include <compiler/enable-ue4-macros.h>
  #include "Carla/Walker/WalkerControl.h"
  #include <compiler/disable-ue4-macros.h>
#endif // LIBCARLA_INCLUDED_FROM_UE4

namespace carla {
namespace rpc {
// 定义 WalkerControl 类
  class WalkerControl {
  public:
// 默认构造函数
    WalkerControl() = default;
// 带参数的构造函数，接收方向、速度和是否跳跃的参数
    WalkerControl(
        geom::Vector3D in_direction,
        float in_speed,
        bool in_jump)
      : direction(in_direction),
        speed(in_speed),
        jump(in_jump) {}
// 代表行人方向的三维向量，默认值为 {1.0f, 0.0f, 0.0f}
    geom::Vector3D direction = {1.0f, 0.0f, 0.0f};

    float speed = 0.0f;

    bool jump = false;

#ifdef LIBCARLA_INCLUDED_FROM_UE4
// 从虚幻引擎中的 FWalkerControl 类型转换为当前的 WalkerControl 类型的构造函数
    WalkerControl(const FWalkerControl &Control)
      : direction(Control.Direction.X, Control.Direction.Y, Control.Direction.Z),
        speed(1e-2f * Control.Speed),
        jump(Control.Jump) {}
 // 重载类型转换运算符，将当前的 WalkerControl 类型转换为虚幻引擎中的 FWalkerControl 类型
    operator FWalkerControl() const {
      FWalkerControl Control;
      Control.Direction = {direction.x, direction.y, direction.z};
      Control.Speed = 1e2f * speed;
      Control.Jump = jump;
      return Control;
    }

#endif // LIBCARLA_INCLUDED_FROM_UE4
// 重载不等于运算符，用于比较两个 WalkerControl 对象是否不同
    bool operator!=(const WalkerControl &rhs) const {
      return direction != rhs.direction || speed != rhs.speed || jump != rhs.jump;
    }

    bool operator==(const WalkerControl &rhs) const {
      return !(*this != rhs);
    }
// 使用 MsgPack 进行序列化定义
    MSGPACK_DEFINE_ARRAY(direction, speed, jump);
  };

} // namespace rpc
} // namespace carla
