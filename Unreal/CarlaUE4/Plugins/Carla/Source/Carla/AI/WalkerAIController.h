// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "GameFramework/Actor.h"

#include "WalkerAIController.generated.h"

///Walker AI Controller 表示这个 actor 什么也不做，它只是客户端实际控制器的一个句柄。
UCLASS()
class CARLA_API AWalkerAIController : public AActor
{
  GENERATED_BODY()

public:

  AWalkerAIController(const FObjectInitializer &ObjectInitializer)
    : Super(ObjectInitializer)
  {
    PrimaryActorTick.bCanEverTick = false;

    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    RootComponent->bHiddenInGame = true;
  }
};
