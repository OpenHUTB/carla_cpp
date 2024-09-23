// Copyright (c) 2017 计算机视觉中心 (CVC) - 巴塞罗那自治大学 (UAB).
//
// 本作品依据MIT许可证条款授权。
// 有关详细信息，请参见 <https://opensource.org/licenses/MIT>.

#pragma once


#include "carla/Debug.h"
#include "carla/Memory.h"
#include "carla/client/detail/ActorState.h"
#include "carla/profiler/LifetimeProfiled.h"

namespace carla {
    namespace client {

        /// 表示模拟中的一个行为体（Actor）。
        class Actor
            : public EnableSharedFromThis<Actor>,
            private profiler::LifetimeProfiled,
            public detail::ActorState {
            using Super = detail::ActorState;
        public:

            explicit Actor(ActorInitializer init)
                : LIBCARLA_INITIALIZE_LIFETIME_PROFILER(init.GetDisplayId()),
                Super(std::move(init)) {}

            using ActorState::GetBoundingBox;

            virtual ~Actor() = default;

            /// 返回行为体的当前位置。
            ///
            /// @note 该函数不会调用模拟器，而是返回上一个时刻接收到的位置。
            geom::Location GetLocation() const;

            /// 返回行为体的当前变换（位置和方向）。
            ///
            /// @note 该函数不会调用模拟器，而是返回上一个时刻接收到的变换。
            geom::Transform GetTransform() const;

            /// 返回行为体的当前3D速度。
            ///
            /// @note 该函数不会调用模拟器，而是返回上一个时刻接收到的速度。
            geom::Vector3D GetVelocity() const;

            /// 返回行为体的当前3D角速度。
            ///
            /// @note 该函数不会调用模拟器，而是返回上一个时刻接收到的角速度。
            geom::Vector3D GetAngularVelocity() const;

            /// 返回行为体的当前3D加速度。
            ///
            /// @note 该函数不会调用模拟器，而是返回根据行为体的速度计算的加速度。
            geom::Vector3D GetAcceleration() const;

            geom::Transform GetComponentWorldTransform(const std::string componentName) const;

            geom::Transform GetComponentRelativeTransform(const std::string componentName) const;

            std::vector<geom::Transform> GetBoneWorldTransforms() const;

            std::vector<geom::Transform> GetBoneRelativeTransforms() const;

            std::vector<std::string> GetComponentNames() const;

            std::vector<std::string> GetBoneNames() const;

            std::vector<geom::Transform> GetSocketWorldTransforms() const;

            std::vector<geom::Transform> GetSocketRelativeTransforms() const;

            std::vector<std::string> GetSocketNames() const;

            /// 将行为体传送到 @a location。
            void SetLocation(const geom::Location& location);

            /// 将行为体传送并旋转到 @a transform。
            void SetTransform(const geom::Transform& transform);

            /// 在应用物理之前设置行为体的速度。
            void SetTargetVelocity(const geom::Vector3D& vector);

            /// 在应用物理之前设置行为体的角速度。
            void SetTargetAngularVelocity(const geom::Vector3D& vector);

            /// 启用恒定速度模式。
            void EnableConstantVelocity(const geom::Vector3D& vector);

            /// 禁用恒定速度模式。
            void DisableConstantVelocity();

            /// 在行为体的质心施加冲量。
            void AddImpulse(const geom::Vector3D& vector);

            /// 在某个位置对行为体施加冲量。
            void AddImpulse(const geom::Vector3D& impulse, const geom::Vector3D& location);

            /// 在行为体的质心施加力。
            void AddForce(const geom::Vector3D& force);

            /// 在某个位置对行为体施加力。
            void AddForce(const geom::Vector3D& force, const geom::Vector3D& location);

            /// 对行为体施加角冲量。
            void AddAngularImpulse(const geom::Vector3D& vector);

            /// 对行为体施加扭矩。
            void AddTorque(const geom::Vector3D& vector);

            /// 启用或禁用该行为体的物理模拟。
            void SetSimulatePhysics(bool enabled = true);

            /// 启用或禁用该行为体的碰撞。
            void SetCollisions(bool enabled = true);

            /// 将行为体标记为已死亡并开始其生命周期。
            void SetActorDead();

            /// 启用或禁用该行为体的重力。
            void SetEnableGravity(bool enabled = true);

            rpc::ActorState GetActorState() const;

            bool IsAlive() const {
                return GetEpisode().IsValid() && (GetActorState() != rpc::ActorState::PendingKill && GetActorState() != rpc::ActorState::Invalid);
            }

            bool IsDormant() const {
                return GetEpisode().IsValid() && GetActorState() == rpc::ActorState::Dormant;
            }

            bool IsActive() const {
                return GetEpisode().IsValid() && GetActorState() == rpc::ActorState::Active;
            }

            /// 告诉模拟器销毁该行为体，并返回行为体是否成功被销毁的结果。
            ///
            /// @note 如果行为体已经成功销毁，则该函数没有效果。
            ///
            /// @warning 此函数会阻塞，直到模拟器完成销毁操作。
            virtual bool Destroy();

            const auto& Serialize() const {
                return Super::GetActorDescription();
            }

        };

    } // namespace client
} // namespace carla
