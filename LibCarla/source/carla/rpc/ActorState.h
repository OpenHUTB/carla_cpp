// Copyright (c) 2021 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// 说明这段代码的版权归属信息，表明其版权归属于巴塞罗那自治大学的计算机视觉中心（CVC），
// 且此版权相关的工作遵循MIT许可协议。
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

// 头文件保护指令，确保该头文件在一个编译单元中只会被包含一次，避免重复定义等问题。
#pragma once

// 包含C++标准库中关于固定宽度整数类型的头文件，后续代码可能会用到其中定义的整数类型，比如这里的uint8_t。
#include <cstdint>

// 定义名为carla的命名空间，一般用于将和Carla项目相关的各种类型、函数等逻辑上组织在一起，
// 避免与其他代码中的同名元素产生命名冲突。
namespace carla {
    // 在carla命名空间内再定义一个名为rpc的子命名空间，
    // 推测其可能用于存放和远程过程调用（RPC）相关的各类定义，例如下面定义的枚举类型。
    namespace rpc {
        // 定义一个名为ActorState的强类型枚举（enum class），
        // 它继承自uint8_t类型，意味着其底层存储占用1个字节（8位）的内存空间，
        // 常用于表示Actor（可能是模拟环境中的某个实体，如车辆、行人等对象）的不同状态。
        enum class ActorState : uint8_t {
            // 表示Actor处于无效的状态，可能是初始化未完成、出现错误等情况导致其不能正常使用。
            Invalid,
            // 代表Actor当前处于活动状态，正在正常参与模拟场景中的相关操作或行为，比如车辆正在行驶等。
            Active,
            // 表示Actor暂时处于休眠状态，可能暂时不参与主要的模拟活动，但依然存在于场景中，等待被唤醒等操作。
            Dormant,
            // 说明Actor已经被标记为等待销毁的状态，可能是模拟场景中不再需要它，即将从场景中移除。
            PendingKill,
        };
    } // namespace rpc
} // namespace carla
