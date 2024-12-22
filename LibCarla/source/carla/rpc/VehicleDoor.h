// Copyright (c) 2021 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/MsgPack.h"// 包含 MsgPack 相关的头文件，可能用于序列化、反序列化等与消息打包相关的功能（具体取决于其内部实现）

#include <cstdint>// 引入标准库中用于处理固定宽度整数类型的头文件，这里主要可能会用到其中的一些整数类型定义等功能

namespace carla {// 定义 carla 命名空间，代码应该是属于 carla 项目相关的内容，在这个命名空间下对各类功能进行组织
namespace rpc {// 进一步定义 rpc 子命名空间，可能用于存放远程过程调用（RPC）相关的类、函数等实现

  enum class VehicleDoor : uint8_t { // 定义一个名为 VehicleDoor 的枚举类型，它继承自无符号 8 位整数类型（uint8_t）
    // 用于表示车辆的门的类型，通过不同的枚举值来区分车辆的各个门以及特殊的门状态
    FL = 0,// 前门左侧（Front Left）
    FR = 1, // 前门右侧（Front Right）
    RL = 2,// 后门左侧（Rear Left）
    RR = 3, // 后门右侧（Rear Right）
    Hood = 4, // 引擎盖（Hood）
    Trunk = 5,   // 后备箱（Trunk）
    All = 6 // 所有门（All，表示一种特殊状态，可能用于同时操作所有门的情况）
  };

} // namespace rpc
} // namespace carla
//使用 MSGPACK_ADD_ENUM 宏（可能是由 "carla/MsgPack.h" 头文件提供）将 VehicleDoor 枚举类型注册到 MsgPack 的序列化/反序列化机制中
// 这样就可以方便地对 VehicleDoor 类型的变量进行消息打包和解包操作，以便在网络传输或数据存储等场景中使用
MSGPACK_ADD_ENUM(carla::rpc::VehicleDoor);
MSGPACK_ADD_ENUM(carla::rpc::VehicleDoor);
