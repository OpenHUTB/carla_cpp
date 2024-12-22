// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Actor/ActorSpawnResult.h"
#include "Carla/Actor/CarlaActorFactory.h"

#include "SensorFactory.generated.h"

/// 工厂负责生成传感器。此工厂能够生成在
/// carla：：sensor：：SensorRegistry 中注册的每个传感器。
UCLASS()
class CARLA_API ASensorFactory : public ACarlaActorFactory
{
  GENERATED_BODY()

  /// 检索在 SensorRegistry 中注册的所有传感器的定义。
  /// 传感器必须实现 GetSensorDefinition（） 
  /// 静态方法。
  TArray<FActorDefinition> GetDefinitions() final;

  FActorSpawnResult SpawnActor(
      const FTransform &SpawnAtTransform,
      const FActorDescription &ActorDescription) final;
};
