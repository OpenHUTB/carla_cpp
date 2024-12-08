// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Components/ActorComponent.h"
#include "Components/PrimitiveComponent.h"
#include "CoreMinimal.h"

#include "VehicleVelocityControl.generated.h"

/// 控制actor速度恒定的组件
UCLASS(Blueprintable, BlueprintType, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CARLA_API UVehicleVelocityControl : public UActorComponent
{
  GENERATED_BODY()

  // ===========================================================================
  /// @name 构造函数和析构函数
  // ===========================================================================
  /// @{
public:
  UVehicleVelocityControl();

  /// @}
  // ===========================================================================
  /// @name 获取函数
  // ===========================================================================
  /// @{
public:

  // 开始播放时调用
  void BeginPlay() override;

  // 每帧调用的Tick函数
  virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

  // 激活组件并设置目标速度
  virtual void Activate(bool bReset=false) override;

  // 激活组件并设置指定的目标速度
  virtual void Activate(FVector Velocity, bool bReset=false);

  // 停用组件
  virtual void Deactivate() override;

private:
  // 目标速度
  UPROPERTY(Category = "Vehicle Velocity Control", VisibleAnywhere)
  FVector TargetVelocity;

  // 原始组件指针
  UPrimitiveComponent* PrimitiveComponent;

  // 拥有该组件的车辆Actor
  AActor* OwnerVehicle;

};