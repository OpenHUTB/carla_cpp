// Copyright (c) 2021 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

// 引入核心最小化相关头文件，包含了一些基础的UE4常用的类型和功能等
#include "CoreMinimal.h"
// 引入标志组件相关头文件，推测这个头文件定义了与标志相关的一些基础类或者功能，当前类可能会基于其进行扩展
#include "SignComponent.h"
// 引入速度限制组件的生成相关头文件，表明这个类是可以被UE4的蓝图系统生成的组件类
#include "SpeedLimitComponent.generated.h"

// USpeedLimitComponent类定义，它继承自USignComponent类，意味着它会拥有USignComponent的所有公有和受保护的成员及功能，并且可以在此基础上进行扩展
// 该类被标记为可以在UE4蓝图中生成的组件（通过BlueprintSpawnableComponent元数据标记），属于Custom组（通过ClassGroup=(Custom)指定）
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CARLA_API USpeedLimitComponent : public USignComponent
{
  GENERATED_BODY()

public:
  // 重写的函数，用于初始化标志（从父类继承而来的虚函数）
  // 该函数接收一个carla::road::Map类型的参数，用于根据地图信息来初始化速度限制相关的标志
  // 具体的初始化逻辑需要在这个函数中实现，以适配当前速度限制组件的功能需求
  virtual void InitializeSign(const carla::road::Map &Map) override;

  // 设置速度限制值的函数，外部可以调用这个函数来改变速度限制组件的速度限制数值
  void SetSpeedLimit(float Limit);

private:
  // 根据给定的变换（BoxTransform）和盒子尺寸（BoxSize）生成速度限制相关的盒子（可能用于可视化或者碰撞检测等用途，具体要看后续实现）
  void GenerateSpeedBox(const FTransform BoxTransform, float BoxSize);

  // 当与速度限制盒子开始重叠时触发的函数（被标记为可以在蓝图中调用，通过BlueprintCallable标记）
  // 这个函数接收多个参数，用于描述重叠事件的详细信息，比如重叠的组件、涉及的其他Actor、其他组件、身体索引以及是否是扫掠碰撞等信息
  // 可以在这个函数中编写当重叠发生时需要执行的逻辑，例如触发速度限制相关的提示或者限制车辆速度等操作
  UFUNCTION(BlueprintCallable)
  void OnOverlapBeginSpeedLimitBox(UPrimitiveComponent *OverlappedComp,
      AActor *OtherActor,
      UPrimitiveComponent *OtherComp,
      int32 OtherBodyIndex,
      bool bFromSweep,
      const FHitResult &SweepResult);

  // 速度限制值，单位可能是某种速度单位（比如千米每小时或者英里每小时等，取决于具体项目设定）
  // 该变量被标记为可以在编辑器中编辑（通过EditAnywhere标记），默认值为30，可根据实际需求在编辑器中调整这个速度限制的值
  UPROPERTY(Category = "Speed Limit", EditAnywhere)
  float SpeedLimit = 30;

};
