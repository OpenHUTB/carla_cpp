// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// 该作品根据MIT许可证进行授权。
// 有关许可证的副本，请参阅<https://opensource.org/licenses/MIT>。

#include "Carla.h"
#include "Carla/AI/AIControllerFactory.h"

#include "Carla/AI/WalkerAIController.h"
#include "Carla/Actor/ActorBlueprintFunctionLibrary.h"

// 获取所有可用的AI控制器定义
TArray<FActorDefinition> AAIControllerFactory::GetDefinitions()
{
  // 使用UActorBlueprintFunctionLibrary中的函数来创建通用的Actor定义
  using ABFL = UActorBlueprintFunctionLibrary;
  
  // 创建一个通用的AI walker控制器定义
  auto WalkerController = ABFL::MakeGenericDefinition(
      TEXT("controller"),  // 类别：控制器
      TEXT("ai"),          // 子类别：AI
      TEXT("walker"));     // 具体类型：walker
  
  // 设置该定义对应的类为AWalkerAIController
  WalkerController.Class = AWalkerAIController::StaticClass();
  
  // 返回包含该AI控制器定义的数组
  return { WalkerController };
}

// 在指定Transform和Description下生成一个Actor
FActorSpawnResult AAIControllerFactory::SpawnActor(
    const FTransform &Transform,  // Actor的生成位置和方向
    const FActorDescription &Description)  // Actor的描述信息
{
  // 获取当前的世界场景
  auto *World = GetWorld();
  
  // 检查世界是否为空
  if (World == nullptr)
  {
    // 如果世界为空，记录错误日志并返回空的Actor生成结果
    UE_LOG(LogCarla, Error, TEXT("AAIControllerFactory: cannot spawn controller into an empty world."));
    return {};
  }

  // 设置Actor生成时的参数
  FActorSpawnParameters SpawnParameters;
  
  // 设置生成时的碰撞处理方式为始终生成，忽略碰撞
  SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
  
  // 使用指定的类、Transform和生成参数在世界中生成Actor
  auto *Controller = World->SpawnActor<AActor>(Description.Class, Transform, SpawnParameters);

  // 检查生成的Actor是否为空
  if (Controller == nullptr)
  {
    // 如果Actor生成失败，记录错误日志
    UE_LOG(LogCarla, Error, TEXT("AAIControllerFactory: spawn controller failed."));
  }
  
  // 返回包含生成Actor的结果
  return FActorSpawnResult{Controller};
}
