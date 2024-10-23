// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/client/Actor.h"
#include "carla/rpc/WalkerControl.h"
#include "carla/rpc/WalkerBoneControlIn.h"
#include "carla/rpc/WalkerBoneControlOut.h"

namespace carla {
namespace client {

  class Walker : public Actor {
  public:

    using Control = rpc::WalkerControl;
    using BoneControlIn = rpc::WalkerBoneControlIn;
    using BoneControlOut = rpc::WalkerBoneControlOut;

    explicit Walker(ActorInitializer init) : Actor(std::move(init)) {}

    /// 应用 @a control 给这个行人。
    void ApplyControl(const Control &control);

    /// 把最后应用的控制返回给Walker对象.
    ///
    /// @note 这个函数没有调用模拟器, 它返回给 Control
    /// 在最后一个节拍返回接收到的数据.
    Control GetWalkerControl() const;

    BoneControlOut GetBonesTransform();
    void SetBonesTransform(const BoneControlIn &bones);
    void BlendPose(float blend);
    void ShowPose() { BlendPose(1.0f); };
    void HidePose() { BlendPose(0.0f); };
    void GetPoseFromAnimation();

  private:

    Control _control;
  };

} // namespace client
} // namespace carla
