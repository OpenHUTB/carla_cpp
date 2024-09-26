// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/client/Walker.h"

#include "carla/client/detail/Simulator.h"

namespace carla {
    namespace client {

        // Ӧ�ÿ��Ƶ�������
        void Walker::ApplyControl(const Control& control) {
            // �������ָ���뵱ǰ���Ʋ�ͬ
            if (control != _control) {
                // ������Ӧ�õ�������
                GetEpisode().Lock()->ApplyControlToWalker(*this, control);
                // ���µ�ǰ����Ϊ�µĿ���
                _control = control;
            }
        }

        // ��ȡ�����ߵĿ�����Ϣ
        Walker::Control Walker::GetWalkerControl() const {
            // ���������ߵĿ���״̬
            return GetEpisode().Lock()->GetActorSnapshot(*this).state.walker_control;
        }

        // ��ȡ�����ߵĹ����任��Ϣ
        Walker::BoneControlOut Walker::GetBonesTransform() {
            // ���ص�ǰ�����ߵĹ����任
            return GetEpisode().Lock()->GetBonesTransform(*this);
        }

        // ���������ߵĹ����任��Ϣ
        void Walker::SetBonesTransform(const Walker::BoneControlIn& bones) {
            // ���µĹ����任Ӧ�õ�������
            return GetEpisode().Lock()->SetBonesTransform(*this, bones);
        }

        // ��ϵ�ǰ����
        void Walker::BlendPose(float blend) {
            // Ӧ�û�����Ƶ�������
            return GetEpisode().Lock()->BlendPose(*this, blend);
        }

        // �Ӷ����л�ȡ����
        void Walker::GetPoseFromAnimation() {
            // ���������ߵ����ƴӶ����л�ȡ
            return GetEpisode().Lock()->GetPoseFromAnimation(*this);
        }

    } // namespace client
} // namespace carla
