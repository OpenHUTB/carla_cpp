// 版权所有(c) 巴塞罗那自治大学（UAB）2023计算机视觉中心(CVC)
//
// 本作品根据MIT许可证的条款进行许可
// 有关副本，请访问 <https://opensource.org/licenses/MIT>.

#include "UtilActorFactory.h"

#include "Carla/Actor/ActorBlueprintFunctionLibrary.h"
#include "Carla/Util/EmptyActor.h"
#include "Carla/Game/CarlaEpisode.h"

//定义了一个静态方法GetDefinitions()用于返回一个TArray类型的数组，其中包含了所有可用的actor定义。
//使用ABFL库创建了一个静态网格定义，并将其添加到结果数组中。

TArray<FActorDefinition> AUtilActorFactory::GetDefinitions()
{
  using ABFL = UActorBlueprintFunctionLibrary;
  auto StaticMeshDefinition = ABFL::MakeGenericDefinition(
      TEXT("util"),
      TEXT("actor"),
      TEXT("empty"));
  StaticMeshDefinition.Class = AEmptyActor::StaticClass();
  
  return { StaticMeshDefinition };
}

//定义了一个SpawnActor()函数，它接受两个参数：Transform &SpawnAtTransform 和 ActorDescription &ActorDescription。
//函数首先检查World对象是否为nullptr，如果是则打印错误日志并返回空的FActorSpawnResult。
//如果World不为nullptr，则设置碰撞处理方式为总是允许生成(SpawnCollisionHandlingMethod::AlwaysSpawn)，然后尝试在指定的位置生成一个AEmptyActor类型的actor。
//最后返回生成的actor的结果。

FActorSpawnResult AUtilActorFactory::SpawnActor(
    const FTransform &SpawnAtTransform,
    const FActorDescription &ActorDescription)
{
  using ABFL = UActorBlueprintFunctionLibrary;
  auto *World = GetWorld();
  //使用UE_LOG宏记录错误信息。
  //使用World->SpawnActor<>模板函数来生成actor实例。
  if (World == nullptr)
  {
    UE_LOG(LogCarla, Error, TEXT
        ("AUtilActorFactory: cannot spawn mesh into an empty world."));
    return {};
  }

  FActorSpawnParameters SpawnParameters;
  SpawnParameters.SpawnCollisionHandlingOverride =
      ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

  auto *StaticMeshActor = World->SpawnActor<AEmptyActor>(
      ActorDescription.Class, SpawnAtTransform, SpawnParameters);

  return FActorSpawnResult(StaticMeshActor);
}
