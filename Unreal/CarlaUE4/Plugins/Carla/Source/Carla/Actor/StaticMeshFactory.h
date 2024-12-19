// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Actor/ActorSpawnResult.h"
#include "Carla/Actor/CarlaActorFactory.h"

#include "StaticMeshFactory.generated.h"

//负责生成静态网格的工厂。此工厂能够在内容中生成任何网格。
UCLASS()
class CARLA_API AStaticMeshFactory : public ACarlaActorFactory
{
  GENERATED_BODY()

  //检索静态网格 actor 的定义
  TArray<FActorDefinition> GetDefinitions() final;

  FActorSpawnResult SpawnActor(
      const FTransform &SpawnAtTransform,
      const FActorDescription &ActorDescription) final;
};
