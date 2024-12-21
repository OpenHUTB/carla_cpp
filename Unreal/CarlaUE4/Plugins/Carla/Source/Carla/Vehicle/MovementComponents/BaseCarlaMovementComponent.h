// Copyright (c) 2021 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
// Copyright (c) 2019 Intel Corporation
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
// 这是一个预处理指令，用于确保该头文件在整个编译过程中只会被包含一次，避免重复定义等问题

#include "GameFramework/MovementComponent.h"
// 引入虚幻引擎（Unreal Engine）中游戏框架（GameFramework）里的MovementComponent.h头文件，
// MovementComponent是用于处理游戏对象移动相关逻辑的基类，包含了如速度计算、移动更新等基础功能，
// 这里引入它是为了后续基于该类进行派生扩展，实现特定的移动相关功能。

#include "Carla/Vehicle/VehicleControl.h"
// 引入Carla项目里Vehicle目录下的VehicleControl.h头文件

#include "BaseCarlaMovementComponent.generated.h"
// 引入由虚幻引擎的代码生成工具生成的头文件
// 表明这个类可能会在虚幻引擎的编辑器环境中通过蓝图进行扩展或使用。

class ACarlaWheeledVehicle;
// 前置声明CarlaWheeledVehicle类，告知编译器后续会有这个类的完整定义，使得在当前代码位置可以使用该类的指针类型，
// 解决类之间相互引用时可能出现的编译顺序问题，这里可能在后续成员变量或者函数参数等地方会用到这个类的指针。

UCLASS(Blueprintable, meta=(BlueprintSpawnableComponent))
class CARLA_API UBaseCarlaMovementComponent : public UMovementComponent
{
// 使用UCLASS宏定义一个类，使其成为虚幻引擎中的一个可被识别的类
// Blueprintable表示这个类可以在虚幻引擎的蓝图编辑器中被扩展和使用，
// meta=(BlueprintSpawnableComponent)表示这个类可以作为一个组件在蓝图中被实例化并添加到游戏对象上。
// 该类继承自UMovementComponent类，意味着它会继承父类中关于移动相关的基础功能和接口，并在此基础上进行扩展。

  GENERATED_BODY()
 // 这是一个虚幻引擎代码生成相关的宏，用于指示编译器在这里生成必要的代码，例如反射相关的代码等，
 // 以支持类在虚幻引擎中的各种高级特性

protected:

  UPROPERTY()
  ACarlaWheeledVehicle* CarlaVehicle;
 // 使用UPROPERTY宏声明一个成员变量CarlaVehicle，它是指向ACarlaWheeledVehicle类的指针类型，
// UPROPERTY宏可以让这个成员变量被虚幻引擎的各种系统

public:

  virtual void BeginPlay() override;
// 重写父类（UMovementComponent）的BeginPlay虚函数，这个函数在游戏开始运行，游戏对象被创建并初始化后调用

  virtual void ProcessControl(FVehicleControl &Control);
// 定义一个虚函数ProcessControl，它接受一个FVehicleControl类型的引用参数Control
// 用于处理车辆控制相关的逻辑

  virtual FVector GetVelocity() const;
// 定义一个虚函数GetVelocity，用于获取车辆当前的速度向量

  virtual int32 GetVehicleCurrentGear() const;
// 定义一个虚函数GetVehicleCurrentGear，用于获取车辆当前所处的挡位信息，返回值为32位整数类型（int32）

  virtual float GetVehicleForwardSpeed() const;
// 定义一个虚函数GetVehicleForwardSpeed，用于获取车辆向前的速度大小

  virtual void DisableSpecialPhysics() {};
// 定义一个虚函数DisableSpecialPhysics，函数体为空

protected:

  void DisableUE4VehiclePhysics();
 // 定义一个保护类型的函数DisableUE4VehiclePhysics，用于禁用虚幻引擎4（UE4）中与车辆相关的物理效果

  void EnableUE4VehiclePhysics(bool bResetVelocity = true);
 // 定义一个保护类型的函数EnableUE4VehiclePhysics，用于启用虚幻引擎4（UE4）中与车辆相关的物理效果，
};
