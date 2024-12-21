// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Walker/WalkerBoneControlIn.h"
#include "Carla/Walker/WalkerBoneControlOut.h"
#include "Carla/Walker/WalkerControl.h"

#include "CoreMinimal.h"
#include "GameFramework/Controller.h"

#include <compiler/disable-ue4-macros.h>
#include <carla/rpc/WalkerBoneControlIn.h>
#include <compiler/enable-ue4-macros.h>

#include "WalkerController.generated.h"

UCLASS()
class CARLA_API AWalkerController : public AController
{
  GENERATED_BODY()

public:

  AWalkerController(const FObjectInitializer &ObjectInitializer);

  void OnPossess(APawn *InPawn) override;

  void Tick(float DeltaSeconds) override;

  /// 允许的最大步行速度（以厘米/秒为单位）。
  UFUNCTION(BlueprintCallable)
  float GetMaximumWalkSpeed() const
  {
    return 4096.0f; // ~147 km/h（40 米/秒）
  }

  UFUNCTION(BlueprintCallable)
  void ApplyWalkerControl(const FWalkerControl &InControl);//接受一个FWalkerControl类型的参数InControl，用于设置或应用某种控制到行走者（walker）上

  UFUNCTION(BlueprintCallable)
  const FWalkerControl GetWalkerControl() const//返回一个FWalkerControl类型的对象，代表当前的行走者控制状态。这个函数也是蓝图可调用的，允许蓝图系统获取当前的行走者控制信息
  {
    return Control;
  }

  UFUNCTION(BlueprintCallable)
  void GetBonesTransform(FWalkerBoneControlOut &WalkerBones);//接受一个引用参数WalkerBones，类型为FWalkerBoneControlOut。它用于获取行走者的骨骼变换信息，例如骨骼的位置、旋转和缩放

  UFUNCTION(BlueprintCallable)
  void SetBonesTransform(const FWalkerBoneControlIn &WalkerBones);//接受一个FWalkerBoneControlIn类型的参数WalkerBones，用于设置行走者的骨骼变换信息

  UFUNCTION(BlueprintCallable)
  void BlendPose(float Blend);//用于混合当前的行走者姿态与另一种姿态。Blend参数可能表示混合的比例或程度

  UFUNCTION(BlueprintCallable)
  void GetPoseFromAnimation();//用于从动画中获取当前的行走者姿态。这意味着它会根据当前播放的动画更新行走者的姿态

private:

  FWalkerControl Control;
};
