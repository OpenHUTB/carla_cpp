// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Actor/ActorAttribute.h"

#include "GameFramework/Actor.h"

#include "ActorDefinition.generated.h"

///carla参与者的定义，包含所有变化和属性
USTRUCT(BlueprintType)
struct FActorDefinition
{
  GENERATED_BODY()

  /// 唯一标识定义（无需填写）
  uint32 UId = 0u;

  ///用于标识参与者的显示ID
  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  FString Id;

  /// 要生成的参与者类（可选）
  ///
  /// 请注意，此参数未在客户端公开，仅用于
  /// 生成器本身
  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  TSubclassOf<AActor> Class;

  /// 一个逗号分隔的标签列表
  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  FString Tags;

  /// 变体代表了用户可以修改的变量，以生成不同的变体
  /// 关于参与者
  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  TArray<FActorVariation> Variations;

  /// 属性表示参与者可能具有的不可修改的特性
  /// 帮助用户识别和过滤参与者
  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  TArray<FActorAttribute> Attributes;
};

/// 一个包含所有变体和属性的车辆角色定义
USTRUCT(BlueprintType)
struct FVehicleActorDefinition
{
  GENERATED_BODY()

  /// 一个逗号分隔的标签列表
  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  FString Tags;

  /// 将静态网格体分配给该参与者
  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  UStaticMesh* mesh;

  /// 颜色代表每个参与者的可用颜色
  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  TArray<FColor> colors;
};
