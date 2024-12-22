// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

// 这个头文件使用了预编译头指令，确保该头文件只会被编译一次，避免重复包含导致的编译错误等问题
#pragma once 

// 包含 "carla/rpc/ActorId.h" 头文件，通常这个头文件中可能定义了与Actor的唯一标识符（ActorId）相关的类型、结构体或类等内容
// 具体取决于 Carla 项目的架构，可能用于在 RPC（远程过程调用）等场景下标识不同的 Actor
#include "carla/rpc/ActorId.h" 

// 包含 "carla/rpc/Response.h" 头文件，很可能这个头文件定义了 Response 相关的模板类或者结构体，用于处理 RPC 调用后的响应相关逻辑
#include "carla/rpc/Response.h" 

namespace carla {
namespace rpc {

    // 使用类型别名（typedef 的替代语法，C++11 引入）创建一个新的类型 CommandResponse，
    // 它实际上是基于 Response 模板类并指定了模板参数为 ActorId 的一个特定类型。
    // 意味着 CommandResponse 类型用于表示和处理那些返回值类型为 ActorId 的 RPC 响应相关操作，
    // 方便代码中后续使用该类型来清晰地处理对应的响应逻辑，增强代码的可读性和类型安全性。
    using CommandResponse = Response<ActorId>; 

} // namespace rpc
} // namespace carla
