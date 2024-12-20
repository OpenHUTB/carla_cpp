// Copyright (c) 2023 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "UtilActorFactory.h"

#include "Carla/Actor/ActorBlueprintFunctionLibrary.h"
#include "Carla/Util/EmptyActor.h"
#include "Carla/Game/CarlaEpisode.h"

// AUtilActorFactory类的方法，用于获取定义的Actor数组
TArray<FActorDefinition> AUtilActorFactory::GetDefinitions()
{
  // 使用UActorBlueprintFunctionLibrary的别名ABFL
  using ABFL = UActorBlueprintFunctionLibrary;
  // 创建一个通用的Actor定义，用于util类别中的actor和empty类型
  auto StaticMeshDefinition = ABFL::MakeGenericDefinition(
      TEXT("util"),
      TEXT("actor"),
      TEXT("empty"));
   // 设置这个定义对应的类为AEmptyActor
  StaticMeshDefinition.Class = AEmptyActor::StaticClass();

   // 返回包含这个定义的数组
  return { StaticMeshDefinition };
}

// AUtilActorFactory类的方法，用于生成Actor
FActorSpawnResult AUtilActorFactory::SpawnActor(
    const FTransform &SpawnAtTransform,
    const FActorDescription &ActorDescription)
{
   // 使用UActorBlueprintFunctionLibrary的别名ABFL
  using ABFL = UActorBlueprintFunctionLibrary;
  // 获取当前世界对象
  auto *World = GetWorld();
  // 如果世界对象为空，则记录错误日志并返回空结果
  if (World == nullptr)
  {
    UE_LOG(LogCarla, Error, TEXT
        ("AUtilActorFactory: cannot spawn mesh into an empty world."));
    return {};
  }

  // 设置生成Actor的参数，覆盖碰撞处理方法为AlwaysSpawn
  FActorSpawnParameters SpawnParameters;
  SpawnParameters.SpawnCollisionHandlingOverride =
      ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

   // 在指定的世界中生成AEmptyActor类型的Actor
  auto *StaticMeshActor = World->SpawnActor<AEmptyActor>(
      ActorDescription.Class, SpawnAtTransform, SpawnParameters);

  // 返回生成的Actor结果
  return FActorSpawnResult(StaticMeshActor);
}
