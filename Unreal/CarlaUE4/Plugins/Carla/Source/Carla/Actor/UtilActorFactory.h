// Copyright (c) 2023 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.
// 防止头文件被重复包含
#pragma once
// 包含Carla中Actor和ActorSpawnResult的定义
#include "Carla/Actor/ActorSpawnResult.h"
#include "Carla/Actor/CarlaActorFactory.h"
// 为AUtilActorFactory类生成必要的UCLASS宏和类元数据
#include "UtilActorFactory.generated.h"

/// 负责生成静态网格体的工厂。该工厂能够生成
/// 内容中的任何网格
UCLASS()
class CARLA_API AUtilActorFactory : public ACarlaActorFactory
{
  GENERATED_BODY()

  /// 获取静态网格体演员的定义
/// @return 返回一个包含所有静态网格体演员定义的数组
  TArray<FActorDefinition> GetDefinitions() final;
/// 在指定的位置生成一个演员
  /// @param SpawnAtTransform 演员生成的位置和旋转
  /// @param ActorDescription 演员的描述信息，包括网格体和其他属性
  /// @return 返回一个包含生成的演员和可能的错误信息的结构体
  FActorSpawnResult SpawnActor(
      const FTransform &SpawnAtTransform,
      const FActorDescription &ActorDescription) final;
};
