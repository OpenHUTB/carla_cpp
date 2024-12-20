// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.
// 防止头文件被多次包含
#pragma once
// 包含CARLA模拟器中Walker相关的头文件
#include "Carla/Walker/WalkerBoneControlIn.h"
#include "Carla/Walker/WalkerBoneControlOut.h"
#include "Carla/Walker/WalkerControl.h"
// 包含Unreal Engine的核心头文件和游戏框架控制器头文件
#include "CoreMinimal.h"
#include "GameFramework/Controller.h"

#include <compiler/disable-ue4-macros.h>// 禁用UE4特定的宏，以便包含CARLA RPC头文件
#include <carla/rpc/WalkerBoneControlIn.h>// 包含CARLA RPC库中WalkerBoneControlIn的定义
#include <compiler/enable-ue4-macros.h>// 重新启用UE4特定的宏

#include "WalkerController.generated.h"// 自动生成类的元数据和函数

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
  void ApplyWalkerControl(const FWalkerControl &InControl);

  UFUNCTION(BlueprintCallable)
  const FWalkerControl GetWalkerControl() const
  {
    return Control;
  }

  UFUNCTION(BlueprintCallable)
  void GetBonesTransform(FWalkerBoneControlOut &WalkerBones);

  UFUNCTION(BlueprintCallable)
  void SetBonesTransform(const FWalkerBoneControlIn &WalkerBones);

  UFUNCTION(BlueprintCallable)
  void BlendPose(float Blend);

  UFUNCTION(BlueprintCallable)
  void GetPoseFromAnimation();

private:

  FWalkerControl Control;
};
