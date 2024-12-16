// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once  // 确保头文件只被包含一次

#include "carla/MsgPack.h"  // 引入MsgPack库的头文件

#include <cstdint>  // 引入C++标准库中的整数类型支持

namespace carla {  // 定义carla命名空间
namespace rpc {   // 定义rpc子命名空间

  // 定义AttachmentType枚举类，表示附加组件的类型
  enum class AttachmentType : uint8_t {
    Rigid,         // 刚体类型
    SpringArm,     // 弹簧臂类型
    SpringArmGhost, // 弹簧臂幽灵类型

    SIZE,          // 枚举大小，用于边界检查
    INVALID        // 无效类型标志
  };

} // namespace rpc
} // namespace carla

// 将AttachmentType枚举类型添加到MsgPack序列化支持中
MSGPACK_ADD_ENUM(carla::rpc::AttachmentType);