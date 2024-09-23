// Copyright (c) 2021 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/geom/Transform.h" // ��������任��
#include "carla/geom/Vector3D.h" // ������ά������
#include "carla/rpc/ActorId.h" // �����ɫID��
#include "carla/rpc/ActorState.h" // �����ɫ״̬��
#include "carla/sensor/data/ActorDynamicState.h" // �����ɫ��̬״̬������

namespace carla {
    namespace client {

        struct ActorSnapshot { // �����ɫ���սṹ��
            ActorId id = 0u; // ��ɫ��Ψһ��ʶ����Ĭ��Ϊ0
            rpc::ActorState actor_state; // ��ɫ��״̬��Ϣ
            geom::Transform transform; // ��ɫ��λ����Ϣ�ͷ���
            geom::Vector3D velocity; // ��ɫ�����ٶ�
            geom::Vector3D angular_velocity; // ��ɫ�Ľ��ٶ�
            geom::Vector3D acceleration; // ��ɫ�ļ��ٶ�
            sensor::data::ActorDynamicState::TypeDependentState state; // ��ɫ�Ķ�̬״̬
        };

    } // namespace client
} // namespace carla
