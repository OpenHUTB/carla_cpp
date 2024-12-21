// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

// 确保头文件只被包含一次
#pragma once

// 包含 Actor.h 头文件
#include "GameFramework/Actor.h"

#include "WalkerAIController.generated.h"

///Walker AI Controller 表示这个 actor 什么也不做，它只是客户端实际控制器的一个句柄。
// 定义类 AWalkerAIController，继承自 AActor
UCLASS()
class CARLA_API AWalkerAIController : public AActor
{
// 这是一个生成的类体，不要手动修改
  GENERATED_BODY()

public:
// 构造函数，使用对象初始化器

  AWalkerAIController(const FObjectInitializer &ObjectInitializer)
    : Super(ObjectInitializer)
  {
    // 设置主角色Tick时是否可以Tick，这里设置为false
    PrimaryActorTick.bCanEverTick = false;

    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    // 设置该组件在游戏中隐藏
    RootComponent->bHiddenInGame = true;
  }
};
