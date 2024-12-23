// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/client/Walker.h"

#include "carla/client/detail/Simulator.h"

namespace carla {
    namespace client {

        // 应用控制到行走者
        void Walker::ApplyControl(const Control& control) {
            // 如果控制指令与当前控制不同
            if (control != _control) {
                // 将控制应用到行走者
                GetEpisode().Lock()->ApplyControlToWalker(*this, control);
                // 更新当前控制为新的控制
                _control = control;
            }
        }

        // 获取行走者的控制信息
        Walker::Control Walker::GetWalkerControl() const {
            // 返回行走者的控制状态
            return GetEpisode().Lock()->GetActorSnapshot(*this).state.walker_control;
        }

        // 获取行走者的骨骼变换信息
        Walker::BoneControlOut Walker::GetBonesTransform() {
            // 返回当前行走者的骨骼变换
            return GetEpisode().Lock()->GetBonesTransform(*this);
        }

        // 设置行走者的骨骼变换信息
        void Walker::SetBonesTransform(const Walker::BoneControlIn& bones) {
            // 将新的骨骼变换应用到行走者
            return GetEpisode().Lock()->SetBonesTransform(*this, bones);
        }

        // 混合当前姿势
        void Walker::BlendPose(float blend) {
            // 应用混合姿势到行走者
            return GetEpisode().Lock()->BlendPose(*this, blend);
        }

        // 从动画中获取姿势
        void Walker::GetPoseFromAnimation() {
            // 更新行走者的姿势从动画中获取
            return GetEpisode().Lock()->GetPoseFromAnimation(*this);
        }

    } // namespace client
} // namespace carla
