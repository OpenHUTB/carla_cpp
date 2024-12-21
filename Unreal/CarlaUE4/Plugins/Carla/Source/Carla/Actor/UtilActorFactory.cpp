// Copyright (c) 2023 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "UtilActorFactory.h" // 包含自定义的UtilActorFactory类头文件
#include "Carla/Actor/ActorBlueprintFunctionLibrary.h" // 包含CARLA的Actor蓝图函数库，提供创建Actor定义的接口
#include "Carla/Util/EmptyActor.h" // 包含CARLA的EmptyActor类，一个不可见的Actor，常用于逻辑触发而非渲染
#include "Carla/Game/CarlaEpisode.h" // 包含CARLA的游戏环节类，用于访问游戏世界的相关信息

// AUtilActorFactory类的方法，用于获取定义的Actor数组
TArray<FActorDefinition> AUtilActorFactory::GetDefinitions()
{
  using ABFL = UActorBlueprintFunctionLibrary; // 使用别名ABFL简化UActorBlueprintFunctionLibrary的调用
  // 使用Actor蓝图函数库创建一个通用的Actor定义，这里的参数分别代表类别、子类别和名称
  auto StaticMeshDefinition = ABFL::MakeGenericDefinition(
      TEXT("util"), // 道具的类别标识符，用于分类和检索
      TEXT("actor"), // 道具的子类别标识符，进一步细分类别
      TEXT("empty")); // 道具的名称，用于具体标识
  
  // 设置Actor定义对应的类为AEmptyActor，即空Actor，这里主要用于逻辑触发而非可视化对象
  StaticMeshDefinition.Class = AEmptyActor::StaticClass();
  
  // 返回包含空Actor定义的数组，这里只有一个元素
  return { StaticMeshDefinition };
}

// AUtilActorFactory类的方法，用于根据给定的变换和描述生成Actor
FActorSpawnResult AUtilActorFactory::SpawnActor(
    const FTransform &SpawnAtTransform, // 指定Actor生成的位置和方向
    const FActorDescription &ActorDescription) // 指定Actor的描述信息，包含Actor的类等元数据
{
  using ABFL = UActorBlueprintFunctionLibrary; // 使用别名ABFL简化UActorBlueprintFunctionLibrary的调用
  auto *World = GetWorld(); // 获取当前Actor工厂所在的世界，用于生成Actor
  
  // 如果世界为空，则无法生成Actor，并记录错误日志
  if (World == nullptr)
  {
    UE_LOG(LogCarla, Error, TEXT("AUtilActorFactory: cannot spawn mesh into an empty world."));
    return {}; // 返回空的Actor生成结果
  }

  // 设置Actor生成参数，这里指定无论是否发生碰撞都要生成Actor
  FActorSpawnParameters SpawnParameters;
  SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

  // 在世界中生成AEmptyActor类型的Actor，使用提供的变换和生成参数
  auto *StaticMeshActor = World->SpawnActor<AEmptyActor>(
      ActorDescription.Class, SpawnAtTransform, SpawnParameters);

  // 返回Actor生成结果，包含生成的Actor指针
  return FActorSpawnResult(StaticMeshActor);
}

