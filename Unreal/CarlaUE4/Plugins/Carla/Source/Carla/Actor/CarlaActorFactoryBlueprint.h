// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Actor/ActorSpawnResult.h"
#include "Carla/Actor/CarlaActorFactory.h"

#include "GameFramework/Actor.h"

#include "CarlaActorFactoryBlueprint.generated.h"

/// 用于实现 ACarlaActorFactory 接口的蓝图基类
///
/// 期望从此类派生的蓝图进行重写
/// GetDefinitions 和 SpawnActor 函数
UCLASS(Abstract, BlueprintType, Blueprintable)
class CARLA_API ACarlaActorFactoryBlueprint : public ACarlaActorFactory
{
  GENERATED_BODY()

public:

  TArray<FActorDefinition> GetDefinitions() final
  {
    return GenerateDefinitions();
  }

  FActorSpawnResult SpawnActor(
      const FTransform &SpawnAtTransform,
      const FActorDescription &ActorDescription) final
  {
    FActorSpawnResult Result;
    SpawnActor(SpawnAtTransform, ActorDescription, Result);
    return Result;
  }

protected:

  UFUNCTION(BlueprintImplementableEvent)
  TArray<FActorDefinition> GenerateDefinitions();

  UFUNCTION(BlueprintImplementableEvent)
  void SpawnActor(
      const FTransform &SpawnAtTransform,
      const FActorDescription &ActorDescription,
      FActorSpawnResult &SpawnResult);
};
