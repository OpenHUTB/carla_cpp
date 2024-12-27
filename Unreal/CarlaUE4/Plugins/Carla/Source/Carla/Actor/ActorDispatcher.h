// 版权所有 (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// 本工作根据 MIT 许可证授权。
// 许可证副本请参见 <https://opensource.org/licenses/MIT>。

#pragma once

#include "Carla/Actor/ActorDefinition.h"
#include "Carla/Actor/ActorDescription.h"
#include "Carla/Actor/ActorRegistry.h"
#include "Carla/Actor/ActorSpawnResult.h"

#include "Containers/Array.h"
#include "Templates/Function.h"

#include "ActorDispatcher.generated.h"

class ACarlaActorFactory;

/// 负责将 ActorDefinition 绑定到 spawn 函数，并维护所有已生成的 Actor 的注册表。
UCLASS()
class CARLA_API UActorDispatcher : public UObject
{
  GENERATED_BODY()

public:

  using SpawnFunctionType = TFunction<FActorSpawnResult(const FTransform &, const FActorDescription &)>;

  /// 将一个 ActorDefinition 绑定到一个 spawn 函数。当使用匹配的描述调用 SpawnActor 时，@a Functor 将被调用。
  ///
  /// @警告 无效的定义将被忽略。
  void Bind(FActorDefinition Definition, SpawnFunctionType SpawnFunction);

  /// 将 @a ActorFactory 的所有定义绑定到其 spawn 函数。
  ///
  /// @警告 无效的定义将被忽略。
  void Bind(ACarlaActorFactory &ActorFactory);

  /// 根据 @a ActorDescription 在 @a Transform 位置生成一个 Actor。要正确销毁通过此函数创建的 Actor，请调用 DestroyActor。
  ///
  /// @return 一个包含 spawn 函数结果和 Actor 及其属性的视图的 pair。如果状态不是 Success，则视图无效。
  TPair<EActorSpawnResultStatus, FCarlaActor*> SpawnActor(
      const FTransform &Transform,
      FActorDescription ActorDescription,
      FCarlaActor::IdType DesiredId = 0);

  /// 根据 @a ActorDescription 在 @a Transform 位置重新生成一个 Actor。用于重新生成休眠的 Actor。
  ///
  /// @return 将要重新生成的 Actor
  AActor* ReSpawnActor(
      const FTransform &Transform,
      FActorDescription ActorDescription);

  /// 使指定 ID 的 Actor 进入休眠状态
  void PutActorToSleep(FCarlaActor::IdType Id, UCarlaEpisode* CarlaEpisode);

  /// 唤醒指定 ID 的 Actor
  void WakeActorUp(FCarlaActor::IdType Id, UCarlaEpisode* CarlaEpisode);

  /// 销毁一个 Actor，并从注册表中正确移除它。
  ///
  /// 如果 @a Actor 被销毁或已经标记为销毁，则返回 true，如果不可销毁或为 nullptr，则返回 false。
  bool DestroyActor(FCarlaActor::IdType ActorId);

  /// 注册一个不是使用 "SpawnActor" 函数创建的 Actor，但它应该保留在注册表中。
  FCarlaActor* RegisterActor(
      AActor &Actor,
      FActorDescription ActorDescription,
      FActorRegistry::IdType DesiredId = 0);

  /// 获取所有的 ActorDefinition 列表
  const TArray<FActorDefinition> &GetActorDefinitions() const
  {
    return Definitions;
  }

  /// 获取 Actor 注册表的只读视图
  const FActorRegistry &GetActorRegistry() const
  {
    return Registry;
  }

  /// 获取可修改的 Actor 注册表
  FActorRegistry &GetActorRegistry()
  {
    return Registry;
  }

private:

  UFUNCTION()
  void OnActorDestroyed(AActor *Actor);

  /// 存储所有 ActorDefinition 的数组
  TArray<FActorDefinition> Definitions;

  /// 存储所有 spawn 函数的数组
  TArray<SpawnFunctionType> SpawnFunctions;

  /// 存储所有 Actor 类的数组
  TArray<TSubclassOf<AActor>> Classes;

  /// Actor 注册表
  FActorRegistry Registry;

};
