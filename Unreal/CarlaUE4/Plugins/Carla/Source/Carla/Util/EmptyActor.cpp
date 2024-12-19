// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "EmptyActor.h"

// 为 AEmptyActor 类设置一个静态（不需要 Tick）的根组件，并确保根组件是可移动的。
AEmptyActor::AEmptyActor(const FObjectInitializer &ObjectInitializer)
  : Super(ObjectInitializer)
{
  PrimaryActorTick.bCanEverTick = false;
  RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRootComponent"));
  RootComponent = RootSceneComponent;
  RootComponent->SetMobility(EComponentMobility::Movable);
}
