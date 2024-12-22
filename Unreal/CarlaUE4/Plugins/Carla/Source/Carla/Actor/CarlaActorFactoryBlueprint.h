// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once // 指示此头文件被包含一次，防止重复包含

#include "Carla/Actor/ActorSpawnResult.h" // 包含Actor生成结果的声明
#include "Carla/Actor/CarlaActorFactory.h" // 包含CARLA Actor工厂的声明
#include "GameFramework/Actor.h" // 包含Unreal Engine Actor框架的头文件
#include "CarlaActorFactoryBlueprint.generated.h" // 包含由Unreal Engine生成的代码头文件

/// 用于实现 ACarlaActorFactory 接口的蓝图基类
/// 期望从此类派生的蓝图进行重写
/// GetDefinitions 和 SpawnActor 函数
UCLASS(Abstract, BlueprintType, Blueprintable) // 标记为抽象类，蓝图类型，可蓝图化
class CARLA_API ACarlaActorFactoryBlueprint : public ACarlaActorFactory // 继承自ACarlaActorFactory
{
  GENERATED_BODY() // 由Unreal Engine生成的代码，用于创建类的实例

public:
  TArray<FActorDefinition> GetDefinitions() final // 重写基类的GetDefinitions方法
  {
    return GenerateDefinitions(); // 调用蓝图可实现的事件以生成Actor定义
  }
  FActorSpawnResult SpawnActor( // 重写基类的SpawnActor方法
      const FTransform &SpawnAtTransform, // 指定生成Actor的变换
      const FActorDescription &ActorDescription) final // 指定生成Actor的描述
  {
    FActorSpawnResult Result; // 创建Actor生成结果对象
    SpawnActor(SpawnAtTransform, ActorDescription, Result); // 调用蓝图可实现的事件以生成Actor
    return Result; // 返回Actor生成结果
  }
protected:
  UFUNCTION(BlueprintImplementableEvent) // 标记为蓝图可实现的事件
  TArray<FActorDefinition> GenerateDefinitions(); // 蓝图可实现的事件，用于生成Actor定义
  UFUNCTION(BlueprintImplementableEvent) // 标记为蓝图可实现的事件
  void SpawnActor( // 蓝图可实现的事件，用于生成Actor
      const FTransform &SpawnAtTransform, // 指定生成Actor的变换
      const FActorDescription &ActorDescription, // 指定生成Actor的描述
      FActorSpawnResult &SpawnResult); // 引用传递Actor生成结果对象
};
