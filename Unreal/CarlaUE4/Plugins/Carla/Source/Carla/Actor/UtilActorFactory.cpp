// Copyright (c) 2023 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "UtilActorFactory.h"

#include "Carla/Actor/ActorBlueprintFunctionLibrary.h" // 包含Actor蓝图函数库，用于创建和管理Actor定义
#include "Carla/Util/EmptyActor.h" // 包含空Actor类，用于生成不可见的Actor
#include "Carla/Game/CarlaEpisode.h" // 包含Carla游戏环节类，用于访问游戏世界信息

// AUtilActorFactory类的方法，用于获取定义的Actor数组
TArray<FActorDefinition> AUtilActorFactory::GetDefinitions()
{
  using ABFL = UActorBlueprintFunctionLibrary; // 使用别名ABFL简化UActorBlueprintFunctionLibrary的调用
  // 使用Actor蓝图函数库创建一个通用的Actor定义
  auto StaticMeshDefinition = ABFL::MakeGenericDefinition(
      TEXT("util"), // Actor类别标识
      TEXT("actor"), // Actor子类别标识
      TEXT("empty")); // Actor名称
  
  // 设置Actor定义对应的类为AEmptyActor，即空Actor
  StaticMeshDefinition.Class = AEmptyActor::StaticClass();
  
  // 返回包含空Actor定义的数组
  return { StaticMeshDefinition };
}

// AUtilActorFactory类的方法，用于根据给定的变换和描述生成Actor
FActorSpawnResult AUtilActorFactory::SpawnActor(
    const FTransform &SpawnAtTransform, // 指定Actor生成的位置和方向
    const FActorDescription &ActorDescription) // 指定Actor的描述信息
{
  using ABFL = UActorBlueprintFunctionLibrary; // 使用别名ABFL简化UActorBlueprintFunctionLibrary的调用
  auto *World = GetWorld(); // 获取当前Actor工厂所在的世界
  if (World == nullptr) // 如果世界为空，则无法生成Actor
  {
    UE_LOG(LogCarla, Error, TEXT("AUtilActorFactory: cannot spawn mesh into an empty world."));
    return {}; // 返回空的Actor生成结果
  }

  // 设置Actor生成参数，覆盖碰撞处理方法，始终生成Actor
  FActorSpawnParameters SpawnParameters;
  SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

  // 在世界中生成AEmptyActor类型的Actor
  auto *StaticMeshActor = World->SpawnActor<AEmptyActor>(
      ActorDescription.Class, SpawnAtTransform, SpawnParameters);

  // 返回Actor生成结果，包含生成的Actor指针
  return FActorSpawnResult(StaticMeshActor);
}
