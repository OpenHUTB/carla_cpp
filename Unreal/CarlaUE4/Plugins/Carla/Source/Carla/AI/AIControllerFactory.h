// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Actor/ActorSpawnResult.h" // 包含生成操作结果类型 FActorSpawnResult 的头文件
#include "Carla/Actor/CarlaActorFactory.h" // 包含 CARLA 仿真平台 actor 工厂的基类

#include "AIControllerFactory.generated.h"

UCLASS() // 声明这是一个 Unreal Engine 的类，用于反射和对象系统集成
class CARLA_API AAIControllerFactory final : public ACarlaActorFactory // 定义一个用于生成 AI 控制器的工厂类，继承自 ACarlaActorFactory
{
  GENERATED_BODY()

  TArray<FActorDefinition> GetDefinitions() final; // 返回该工厂支持生成的 actor 类型集合

  FActorSpawnResult SpawnActor(
      const FTransform &SpawnAtTransform, // 指定 actor 的生成位置和方向
      const FActorDescription &ActorDescription) final; // 提供 actor 的描述信息，例如类型和配置
};

