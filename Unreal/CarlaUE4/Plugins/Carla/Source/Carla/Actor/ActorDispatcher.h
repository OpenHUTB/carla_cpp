// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Actor/ActorDefinition.h"
#include "Carla/Actor/ActorDescription.h"
#include "Carla/Actor/ActorRegistry.h"
#include "Carla/Actor/ActorSpawnResult.h"

#include "Containers/Array.h"
#include "Templates/Function.h"

#include "ActorDispatcher.generated.h"

class ACarlaActorFactory;

/// 负责将ActorDefinitions绑定到生成函数，以及
/// 维护所有已生成参与者的注册表
UCLASS()
class CARLA_API UActorDispatcher : public UObject
{
  GENERATED_BODY()

public:

  using SpawnFunctionType = TFunction<FActorSpawnResult(const FTransform &, const FActorDescription &)>;

  /// 将定义绑定到一个生成函数。当使用一个
  /// 匹配描述的Functer被称为
  ///
  /// 警告：无效的定义将被忽略
  void Bind(FActorDefinition Definition, SpawnFunctionType SpawnFunction);

  /// 将所有@a ActorFactory 的定义绑定到其 spawn 函数
  ///
  ///警告 无效的定义将被忽略
  void Bind(ACarlaActorFactory &ActorFactory);

  ///在 @a Transform 位置基于 @a ActorDescription 生成一个角色。为了正确地
  /// 使用此函数创建的actor可以通过调用DestroyActor来销毁
  ///
  /// 回复包含 spawn 函数结果和视图的一对
  /// 参与者及其属性。如果状态不是成功，则
  /// 视图无效
  TPair<EActorSpawnResultStatus, FCarlaActor*> SpawnActor(
      const FTransform &Transform,
      FActorDescription ActorDescription,
      FCarlaActor::IdType DesiredId = 0);

  /// 在给定的变换位置@a Transform 处重新生成一个基于@a ActorDescription 的演员。为了正确地
  /// 使用此函数创建的 actor 可以通过调用 DestroyActor 来销毁
  /// 用于重新生成休眠的参与者
  ///
  ///返回要重生的角色
  AActor* ReSpawnActor(
      const FTransform &Transform,
      FActorDescription ActorDescription);

  void PutActorToSleep(FCarlaActor::IdType Id, UCarlaEpisode* CarlaEpisode);

  void WakeActorUp(FCarlaActor::IdType Id, UCarlaEpisode* CarlaEpisode);

  /// 销毁一个角色，并将其从注册表中正确移除
  ///
  /// 如果@a Actor已被销毁或已标记为销毁，则返回true
  /// 如果不可破坏或为空指针，则返回 false
  //bool DestroyActor(AActor *Actor);

  bool DestroyActor(FCarlaActor::IdType ActorId);

  /// 注册一个未使用“SpawnActor”函数创建但应保留在注册表中的角色
  FCarlaActor* RegisterActor(
      AActor &Actor,
      FActorDescription ActorDescription,
      FActorRegistry::IdType DesiredId = 0);

  const TArray<FActorDefinition> &GetActorDefinitions() const
  {
    return Definitions;
  }

  const FActorRegistry &GetActorRegistry() const
  {
    return Registry;
  }

  FActorRegistry &GetActorRegistry()
  {
    return Registry;
  }

private:

  UFUNCTION()
  void OnActorDestroyed(AActor *Actor);

  TArray<FActorDefinition> Definitions;

  TArray<SpawnFunctionType> SpawnFunctions;

  TArray<TSubclassOf<AActor>> Classes;

  FActorRegistry Registry;

};
