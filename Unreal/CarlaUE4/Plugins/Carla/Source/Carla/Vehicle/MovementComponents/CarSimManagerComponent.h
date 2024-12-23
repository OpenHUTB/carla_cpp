// Copyright (c) 2021 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
// Copyright (c) 2019 Intel Corporation
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

// 引入必要的头文件
#include "BaseCarlaMovementComponent.h" // 基础移动组件的定义
#include "Carla/Vehicle/VehicleControl.h" // 车辆控制相关定义

#ifdef WITH_CARSIM
#include "CarSimMovementComponent.h" // 只有启用 CarSim 时才包含的头文件
#endif

#include "CarSimManagerComponent.generated.h"

// 前向声明 ACarlaWheeledVehicle 类
class ACarlaWheeledVehicle;

// UCarSimManagerComponent 类声明
// 该类继承自 UBaseCarlaMovementComponent，表示 CarSim 管理组件
UCLASS(Blueprintable, meta=(BlueprintSpawnableComponent))
class CARLA_API UCarSimManagerComponent : public UBaseCarlaMovementComponent
{
  GENERATED_BODY()

  #ifdef WITH_CARSIM
  AActor* OffsetActor; // 用于存储偏移实体
  UCarSimMovementComponent* CarSimMovementComponent; // CarSim 的移动组件指针
  #endif

public:

  // 静态方法，用于为指定车辆创建 CarSim 组件
  static void CreateCarsimComponent(
      ACarlaWheeledVehicle* Vehicle, FString Simfile);

  FString SimfilePath = ""; // 存储 Simfile 的路径

  // 在游戏开始时调用
  virtual void BeginPlay() override;

  // 每帧调用，用于更新组件状态
  virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

  // 处理车辆控制输入
  void ProcessControl(FVehicleControl &Control) override;

  // 获取当前速度
  FVector GetVelocity() const override;

  // 在游戏结束时调用
  virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

  // 开启或关闭 CarSim 道路模式
  void UseCarSimRoad(bool bEnabled);

  // 获取当前车辆的挡位
  int32 GetVehicleCurrentGear() const override;

  // 获取车辆当前的前向速度
  float GetVehicleForwardSpeed() const override;

  // 禁用 CarSim 物理模拟
  void DisableCarSimPhysics();

  // 禁用特殊物理模拟（覆盖基类方法）
  virtual void DisableSpecialPhysics() override;

private:

  // 当车辆与其他物体发生碰撞时调用，仅在启用 CarSim 时生效
  UFUNCTION()
  void OnCarSimHit(AActor* Actor,
      AActor* OtherActor,
      FVector NormalImpulse,
      const FHitResult& Hit);

  // 当车辆与静态环境发生重叠时调用，仅在启用 CarSim 时生效
  UFUNCTION()
  void OnCarSimOverlap(UPrimitiveComponent* OverlappedComponent,
      AActor* OtherActor,
      UPrimitiveComponent* OtherComp,
      int32 OtherBodyIndex,
      bool bFromSweep,
      const FHitResult& SweepResult);

  // 当车辆与静态环境的重叠结束时调用，仅在启用 CarSim 时生效
  UFUNCTION()
  void OnCarSimEndOverlap(UPrimitiveComponent* OverlappedComponent,
      AActor* OtherActor,
      UPrimitiveComponent* OtherComp,
      int32 OtherBodyIndex);
};