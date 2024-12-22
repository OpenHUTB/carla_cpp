// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

//定义了一个 AAIControllerFactory 类，
//它负责生成 AI 控制器 Actor。GetDefinitions 函数返回一个包含 AI 控制器定义的数组，
//而 SpawnActor 函数则负责根据给定的变换和描述信息在游戏世界中生成这个 Actor。
//如果生成过程中出现任何错误，例如世界对象为空或 Actor 生成失败，代码会输出错误日志。


#include "Carla.h"
#include "Carla/AI/AIControllerFactory.h"

#include "Carla/AI/WalkerAIController.h"
#include "Carla/Actor/ActorBlueprintFunctionLibrary.h"

TArray<FActorDefinition> AAIControllerFactory::GetDefinitions()
{
  // 使用 UActorBlueprintFunctionLibrary 类，并为其创建别名 ABFL 以简化代码。
  using ABFL = UActorBlueprintFunctionLibrary;
  // 使用 ABFL 类的 MakeGenericDefinition 静态函数创建一个通用的 Actor 定义。
  auto WalkerController = ABFL::MakeGenericDefinition(
      // 定义的名称。
     TEXT("controller"),
      // 定义的类别。
      TEXT("ai"),
      // 定义的子类别。
      TEXT("walker")); 
  // 设置 WalkerController 的 Class 成员，指向 AWalkerAIController 类的类对象。
  WalkerController.Class = AWalkerAIController::StaticClass();
  // 返回包含 WalkerController 的数组。
  return { WalkerController }; 
}
// 传入的参数，包含 Actor 的变换信息（位置、旋转和缩放）。
FActorSpawnResult AAIControllerFactory::SpawnActor(
   const FTransform &Transform,
    // 传入的参数，包含 Actor 的描述信息。
   const FActorDescription &Description)
{
  // 获取当前世界（Unreal Engine 中的世界对象）。
   auto *World = GetWorld();
  // 如果世界对象为空，则输出错误日志。
   if (World == nullptr)
  {
    // 设置碰撞处理方式为总是生成 Actor，忽略碰撞。
    UE_LOG(LogCarla, Error, TEXT("AAIControllerFactory: cannot spawn controller into an empty world."));
    // 返回一个空的 FActorSpawnResult 对象。
    return {};
  }
  
  // 创建 FActorSpawnParameters 对象，用于设置 Actor 生成时的参数。
  FActorSpawnParameters SpawnParameters;
  // 设置碰撞处理方式为总是生成 Actor，忽略碰撞。
  SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
  // 使用 World 对象的 SpawnActor 函数生成一个新的 Actor，传入 Actor 类型、变换和生成参数。
  auto *Controller = World->SpawnActor<AActor>(Description.Class, Transform, SpawnParameters);
  // 如果生成的 Controller 为空，则输出错误日志。
  if (Controller == nullptr)
  {
    UE_LOG(LogCarla, Error, TEXT("AAIControllerFactory: spawn controller failed."));
  }
  // 返回包含生成的 Controller 的 FActorSpawnResult 对象。
  return FActorSpawnResult{Controller};
  
}
