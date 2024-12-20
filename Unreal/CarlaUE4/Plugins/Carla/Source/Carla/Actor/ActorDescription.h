// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Actor/ActorAttribute.h"

#include "ActorDescription.generated.h"

/// carla参与者及其所有变体的描述
USTRUCT(BlueprintType)
struct FActorDescription
{
  GENERATED_BODY()

  /// 此描述所基于的定义的UId
  uint32 UId = 0u;

  /// 用于标识参与者的显示ID
  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  FString Id;

  /// 要生成的参与者类
  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  TSubclassOf<AActor> Class;

  /// 用户选择了参与者的变化版本。请注意，此时是
  /// 由不可修改的属性表示
  ///
  ///   关键：属性的标识符
  ///   价值：属性
  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  TMap<FString, FActorAttribute> Variations;
};
