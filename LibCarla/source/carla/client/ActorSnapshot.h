// Copyright (c) 2021 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/geom/Transform.h" // 引入坐标变换类
#include "carla/geom/Vector3D.h" // 引入三维向量类
#include "carla/rpc/ActorId.h" // 引入角色ID类
#include "carla/rpc/ActorState.h" // 引入角色状态类
#include "carla/sensor/data/ActorDynamicState.h" // 引入角色动态状态数据类

namespace carla {
    namespace client {

        struct ActorSnapshot { // 定义角色快照结构体
            ActorId id = 0u; // 角色的唯一标识符，默认为0
            rpc::ActorState actor_state; // 角色的状态信息
            geom::Transform transform; // 角色的位置信息和方向
            geom::Vector3D velocity; // 角色的线速度
            geom::Vector3D angular_velocity; // 角色的角速度
            geom::Vector3D acceleration; // 角色的加速度
            sensor::data::ActorDynamicState::TypeDependentState state; // 角色的动态状态
        };

    } // namespace client
} // namespace carla
