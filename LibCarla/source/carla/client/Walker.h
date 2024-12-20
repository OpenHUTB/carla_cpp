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
    /**
       * @class Walker
       * @brief 行人角色类，继承自Actor类。
       *
       * Walker类代表CARLA模拟器中的一个行人角色，可以通过该类控制行人的行为和动作。
       */
  class Walker : public Actor {
  public:
      /**
           * @typedef Control
           * @brief 行人控制指令的类型别名，对应rpc::WalkerControl。
           */
    using Control = rpc::WalkerControl;
    /**
     * @typedef BoneControlIn
     * @brief 行人骨骼控制输入的类型别名，对应rpc::WalkerBoneControlIn。
     */
    using BoneControlIn = rpc::WalkerBoneControlIn;
    /**
     * @typedef BoneControlOut
     * @brief 行人骨骼控制输出的类型别名，对应rpc::WalkerBoneControlOut。
     */
    using BoneControlOut = rpc::WalkerBoneControlOut;
    /**
     * @brief 构造函数，使用ActorInitializer初始化Walker对象。
     *
     * @param init ActorInitializer对象，用于初始化Walker对象。
     */
    explicit Walker(ActorInitializer init) : Actor(std::move(init)) {}
    /**
     * @brief 应用控制指令给这个行人。
     *
     * @param control 控制指令，类型为Control。
     */
    void ApplyControl(const Control &control);
    /**
     * @brief 获取最后应用给Walker对象的控制指令。
     *
     * @note 这个函数不会调用模拟器，它返回的是在上一个节拍接收到的控制指令数据。
     *
     * @return 返回最后应用的控制指令，类型为Control。
     */
    Control GetWalkerControl() const;
    /**
     * @brief 获取行人骨骼的变换信息。
     *
     * @return 返回行人骨骼的变换信息，类型为BoneControlOut。
     */
    BoneControlOut GetBonesTransform();
    /**
     * @brief 设置行人骨骼的变换信息。
     *
     * @param bones 行人骨骼的变换信息，类型为BoneControlIn。
     */
    void SetBonesTransform(const BoneControlIn &bones);
    /**
     * @brief 混合姿态，通过给定的混合因子调整当前姿态和动画姿态之间的比例。
     *
     * @param blend 混合因子，范围在0.0到1.0之间。0.0表示完全隐藏动画姿态，1.0表示完全显示动画姿态。
     */
    void BlendPose(float blend);
    /**
     * @brief 显示动画姿态。
     *
     * 这是一个便捷函数，等价于BlendPose(1.0f)。
     */
    void ShowPose() { BlendPose(1.0f); };
    /**
     * @brief 隐藏动画姿态。
     *
     * 这是一个便捷函数，等价于BlendPose(0.0f)。
     */
    void HidePose() { BlendPose(0.0f); };
    /**
     * @brief 从动画中获取姿态。
     *
     * 这个函数会更新行人的姿态以匹配当前动画的状态。
     */
    void GetPoseFromAnimation();

  private:

      /**
       * @brief 最后应用给Walker对象的控制指令。
       */
    Control _control;
  };

} // namespace client
} // namespace carla
