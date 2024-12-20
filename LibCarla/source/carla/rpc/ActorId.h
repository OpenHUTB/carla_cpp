// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

// 这是一个头文件保护，防止头文件被重复包含
#pragma once

// 包含标准的C++整数类型相关的头文件，这里后续可能会用到像uint32_t这样的类型定义
#include <cstdint>

// 定义了carla命名空间，通常用于将和Carla相关的代码组织在一个逻辑范围内，避免命名冲突
namespace carla {
    // 在carla命名空间内，又定义了rpc子命名空间，可能用于存放和远程过程调用（RPC）相关的类型、函数等
    namespace rpc {
        // 使用类型别名（typedef的替代语法）将uint32_t类型重命名为ActorId，
        // 这样在rpc命名空间内，ActorId就代表了一个32位无符号整数类型，
        // 可能用于标识某个Actor（比如在模拟环境中的某个对象，像车辆、行人等）的唯一ID
        using ActorId = uint32_t;
    } // namespace rpc

    // 这里再次使用类型别名，将rpc命名空间内定义的ActorId类型引入到carla命名空间下，
    // 使得在carla命名空间的其他地方可以直接使用ActorId这个类型名来表示对应的32位无符号整数类型，
    // 方便代码中统一使用该类型来处理与Actor相关的标识等操作。
    using ActorId = rpc::ActorId;
} // namespace carla
