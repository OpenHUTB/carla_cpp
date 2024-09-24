// Copyright (c) 2017 ������Ӿ����� (CVC) - �����������δ�ѧ (UAB).
//
// ����Ʒ����MIT���֤������Ȩ��
// �й���ϸ��Ϣ����μ� <https://opensource.org/licenses/MIT>.

#pragma once


#include "carla/Debug.h"
#include "carla/Memory.h"
#include "carla/client/detail/ActorState.h"
#include "carla/profiler/LifetimeProfiled.h"

namespace carla {
    namespace client {

        /// ��ʾģ���е�һ����Ϊ�壨Actor����
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

            /// ������Ϊ��ĵ�ǰλ�á�
            ///
            /// @note �ú����������ģ���������Ƿ�����һ��ʱ�̽��յ���λ�á�
            geom::Location GetLocation() const;

            /// ������Ϊ��ĵ�ǰ�任��λ�úͷ��򣩡�
            ///
            /// @note �ú����������ģ���������Ƿ�����һ��ʱ�̽��յ��ı任��
            geom::Transform GetTransform() const;

            /// ������Ϊ��ĵ�ǰ3D�ٶȡ�
            ///
            /// @note �ú����������ģ���������Ƿ�����һ��ʱ�̽��յ����ٶȡ�
            geom::Vector3D GetVelocity() const;

            /// ������Ϊ��ĵ�ǰ3D���ٶȡ�
            ///
            /// @note �ú����������ģ���������Ƿ�����һ��ʱ�̽��յ��Ľ��ٶȡ�
            geom::Vector3D GetAngularVelocity() const;

            /// ������Ϊ��ĵ�ǰ3D���ٶȡ�
            ///
            /// @note �ú����������ģ���������Ƿ��ظ�����Ϊ����ٶȼ���ļ��ٶȡ�
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

            /// ����Ϊ�崫�͵� @a location��
            void SetLocation(const geom::Location& location);

            /// ����Ϊ�崫�Ͳ���ת�� @a transform��
            void SetTransform(const geom::Transform& transform);

            /// ��Ӧ������֮ǰ������Ϊ����ٶȡ�
            void SetTargetVelocity(const geom::Vector3D& vector);

            /// ��Ӧ������֮ǰ������Ϊ��Ľ��ٶȡ�
            void SetTargetAngularVelocity(const geom::Vector3D& vector);

            /// ���ú㶨�ٶ�ģʽ��
            void EnableConstantVelocity(const geom::Vector3D& vector);

            /// ���ú㶨�ٶ�ģʽ��
            void DisableConstantVelocity();

            /// ����Ϊ�������ʩ�ӳ�����
            void AddImpulse(const geom::Vector3D& vector);

            /// ��ĳ��λ�ö���Ϊ��ʩ�ӳ�����
            void AddImpulse(const geom::Vector3D& impulse, const geom::Vector3D& location);

            /// ����Ϊ�������ʩ������
            void AddForce(const geom::Vector3D& force);

            /// ��ĳ��λ�ö���Ϊ��ʩ������
            void AddForce(const geom::Vector3D& force, const geom::Vector3D& location);

            /// ����Ϊ��ʩ�ӽǳ�����
            void AddAngularImpulse(const geom::Vector3D& vector);

            /// ����Ϊ��ʩ��Ť�ء�
            void AddTorque(const geom::Vector3D& vector);

            /// ���û���ø���Ϊ�������ģ�⡣
            void SetSimulatePhysics(bool enabled = true);

            /// ���û���ø���Ϊ�����ײ��
            void SetCollisions(bool enabled = true);

            /// ����Ϊ����Ϊ����������ʼ���������ڡ�
            void SetActorDead();

            /// ���û���ø���Ϊ���������
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

            /// ����ģ�������ٸ���Ϊ�壬��������Ϊ���Ƿ�ɹ������ٵĽ����
            ///
            /// @note �����Ϊ���Ѿ��ɹ����٣���ú���û��Ч����
            ///
            /// @warning �˺�����������ֱ��ģ����������ٲ�����
            virtual bool Destroy();

            const auto& Serialize() const {
                return Super::GetActorDescription();
            }

        };

    } // namespace client
} // namespace carla
