// Copyright (c) 2021 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
// Copyright (c) 2019 Intel Corporation
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
// 这是一个预处理指令，用于确保头文件只被编译一次，避免重复包含导致的编译错误

#include "BaseCarlaMovementComponent.h"
// 包含 "BaseCarlaMovementComponent.h" 头文件，引入相关的基类定义等内容，可能是这个类继承或依赖的基础类所在头文件
#include "DefaultMovementComponent.generated.h"
// 包含自动生成代码相关的头文件，通常用于和虚幻引擎的反射等机制配合，由虚幻引擎的代码生成工具生成一些必要的代码辅助类相关功能实现


UCLASS(Blueprintable, meta=(BlueprintSpawnableComponent) )
// 使用 UCLASS 宏来声明这是一个虚幻引擎中的类，Blueprintable 表示这个类可以在蓝图中使用，meta 里的 BlueprintSpawnableComponent 表示该类可以作为可在蓝图中生成的组件
class CARLA_API UDefaultMovementComponent : public UBaseCarlaMovementComponent
{
  GENERATED_BODY()
 // 这是一个宏，用于告诉虚幻引擎的代码生成工具在这里生成必要的类相关代码体，例如反射相关代码等

public:
 // 以下是类的公有成员部分

  static void CreateDefaultMovementComponent(ACarlaWheeledVehicle* Vehicle);
// 声明一个静态函数 CreateDefaultMovementComponent，它接受一个指向 ACarlaWheeledVehicle 类型对象的指针作为参数，通常用于创建默认的移动组件实例相关操作，静态函数可以通过类名直接调用而不需要类的对象实例

  virtual void BeginPlay() override;
 // 重写虚函数 BeginPlay，这个函数在游戏开始时（比如游戏关卡加载完成后、相关Actor进入游戏世界等时刻）被调用，用于执行一些初始化等操作

  void ProcessControl(FVehicleControl &Control) override;
    // 重写函数 ProcessControl，用于处理车辆的控制逻辑，接收一个 FVehicleControl 类型的引用作为参数，可能在函数内部根据传入的控制参数来调整车辆的行为等

  // FVector GetVelocity() const override;
// 这是一条被注释掉的函数声明，原本应该是用于获取车辆速度向量的函数声明，const 表示这个函数不会修改类的成员变量，并且应该返回一个 FVector 类型的值，这里被注释掉可能是暂时不需要或者后续再实现等情况


  int32 GetVehicleCurrentGear() const override;
 // 重写函数 GetVehicleCurrentGear，用于获取车辆当前挡位，返回一个 32 位整数类型的值，const 表示不会修改类的成员变量

  float GetVehicleForwardSpeed() const override;
// 重写函数 GetVehicleForwardSpeed，用于获取车辆的前进速度，返回一个单精度浮点数类型的值，const 同样表示不会修改类的成员变量

};

