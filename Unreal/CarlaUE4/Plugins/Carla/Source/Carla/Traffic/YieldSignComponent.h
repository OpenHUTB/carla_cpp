// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once// 确保头文件只被包含一次，避免重复定义

#include "CoreMinimal.h"  // 核心极小的：包含UE4核心编程环境的普遍存在类型(包括FString / FName / TArray 等)
#include "SignComponent.h"
#include "Carla/Vehicle/WheeledVehicleAIController.h"// 包含Carla中关于轮式车辆AI控制器的定义
#include "YieldSignComponent.generated.h"// 包含UYieldSignComponent类的生成代码

// UCLASS宏定义了一个新的类UYieldSignComponent，继承自USignComponent
// ClassGroup=(Custom)将该类归入自定义类别
// meta=(BlueprintSpawnableComponent)允许这个组件在虚幻编辑器中被蓝图脚本生成
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CARLA_API UYieldSignComponent : public USignComponent
{
  GENERATED_BODY()

public:
// 覆盖基类的InitializeSign函数，用于初始化交通标志
  virtual void InitializeSign(const carla::road::Map &Map) override;

private:
// 生成让路标志的盒子
  void GenerateYieldBox(const FTransform BoxTransform,
      const FVector BoxSize);

  void GenerateCheckBox(const FTransform BoxTransform,
      float BoxSize);

  /// 尽量给“让路”车辆让路，并检查复选框中的车辆数量
  UFUNCTION(BlueprintCallable)
  void GiveWayIfPossible();
// 延迟让路函数
  void DelayedGiveWay(float Delay);

  UFUNCTION(BlueprintCallable)
  void OnOverlapBeginYieldEffectBox(UPrimitiveComponent *OverlappedComp,
      AActor *OtherActor,
      UPrimitiveComponent *OtherComp,
      int32 OtherBodyIndex,
      bool bFromSweep,
      const FHitResult &SweepResult);
// 当车辆停止与让路效果盒子重叠时调用
  UFUNCTION(BlueprintCallable)
  void OnOverlapEndYieldEffectBox(UPrimitiveComponent *OverlappedComp,
      AActor *OtherActor,
      UPrimitiveComponent *OtherComp,
      int32 OtherBodyIndex);
// 当有车辆开始与复选框重叠时调用
  UFUNCTION(BlueprintCallable)
  void OnOverlapBeginYieldCheckBox(UPrimitiveComponent *OverlappedComp,
      AActor *OtherActor,
      UPrimitiveComponent *OtherComp,
      int32 OtherBodyIndex,
      bool bFromSweep,
      const FHitResult &SweepResult);
// 当车辆停止与复选框重叠时调用
  UFUNCTION(BlueprintCallable)
  void OnOverlapEndYieldCheckBox(UPrimitiveComponent *OverlappedComp,
      AActor *OtherActor,
      UPrimitiveComponent *OtherComp,
      int32 OtherBodyIndex);
// 从两个列表中移除相同的车辆
  void RemoveSameVehicleInBothLists();
// 存储在让路标志中的车辆
  UPROPERTY()
  TSet<ACarlaWheeledVehicle*> VehiclesInYield;
// 存储需要检查的车辆及其数量
  UPROPERTY()
  TMap<ACarlaWheeledVehicle*, int> VehiclesToCheck;
};
