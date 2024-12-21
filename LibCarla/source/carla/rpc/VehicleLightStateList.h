// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
// 这是一个预处理指令，用于确保头文件只被编译一次，避免重复包含导致的编译错误。
#include "carla/rpc/ActorId.h"
// 包含名为 "ActorId.h" 的头文件，通常这个头文件中定义了与ActorId相关的类型、结构体或函数声明等内容，
// 这里的ActorId可能用于在Carla的RPC（远程过程调用）相关机制中标识不同的参与者（Actor）。
#include "carla/rpc/VehicleLightState.h"
// 包含名为 "VehicleLightState.h" 的头文件，估计该头文件里定义了和车辆灯光状态相关的内容，
// 比如表示车辆各种灯光（大灯、转向灯等）状态的枚举类型或者结构体等。
#include <cstdint>
// 精确字节大小的整数类型使用
#include <vector>
// 包含标准的C++头文件，用于引入向量（vector）容器相关的模板类和函数定义，向量是一种可以动态改变大小的数组，
// 在C++编程中经常用于存储一组同类型的元素
namespace carla {
// 定义名为carla的命名空间，用于将相关的代码逻辑组织在一起，避免名字冲突，类似把相关的代码放在一个“代码区域”里。
namespace rpc {
    // 在carla命名空间内再定义一个rpc的子命名空间，可能用于存放和远程过程调用（RPC）相关的特定代码内容。
  using VehicleLightStateList =
      std::vector<std::pair<ActorId, VehicleLightState::flag_type>>;
// 使用关键字“using”进行类型别名的定义，将一个复杂的类型声明
      // （这里是一个由 std::vector 包裹的 std::pair 类型，pair中包含ActorId类型和VehicleLightState::flag_type类型）
      // 简化为一个更容易使用和理解的别名VehicleLightStateList，方便后续代码中使用该类型来声明变量等操作。
} // namespace rpc
} // namespace carla
