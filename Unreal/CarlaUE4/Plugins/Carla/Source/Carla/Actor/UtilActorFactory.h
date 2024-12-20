// Copyright (c) 2023 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Actor/ActorSpawnResult.h"
#include "Carla/Actor/CarlaActorFactory.h"

#include "UtilActorFactory.generated.h"

/// 负责生成静态网格体的工厂。该工厂能够生成
/// 内容中的任何网格
UCLASS()
class CARLA_API AUtilActorFactory : public ACarlaActorFactory
{
  GENERATED_BODY()

  /// 获取静态网格体演员的定义
  TArray<FActorDefinition> GetDefinitions() final;

  FActorSpawnResult SpawnActor(
      const FTransform &SpawnAtTransform,
      const FActorDescription &ActorDescription) final;
};
