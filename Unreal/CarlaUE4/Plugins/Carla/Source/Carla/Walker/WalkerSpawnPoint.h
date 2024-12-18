// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Engine/TargetPoint.h"
#include "WalkerSpawnPoint.generated.h"

/// Base class for spawner locations for walkers.
UCLASS(Abstract)
class CARLA_API AWalkerSpawnPointBase : public ATargetPoint
{
  GENERATED_BODY()
};

/// 用于设置关卡中行人的生成器位置。这些位置将仅用于在游戏开始时生成行人。
UCLASS()
class CARLA_API AWalkerStartSpawnPoint : public AWalkerSpawnPointBase
{
  GENERATED_BODY()
};

/// 用于设置关卡中行人的生成点位置。这些位置将用作行人的生成点和目的地点。
UCLASS()
class CARLA_API AWalkerSpawnPoint : public AWalkerSpawnPointBase
{
  GENERATED_BODY()
};
