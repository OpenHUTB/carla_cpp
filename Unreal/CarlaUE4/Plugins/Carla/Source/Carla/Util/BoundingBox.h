// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "BoundingBox.generated.h"

USTRUCT(BlueprintType)
struct CARLA_API FBoundingBox
{
  GENERATED_BODY()

  /// 边界框相对于其所有者的原点
  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  FVector Origin = {0.0f, 0.0f, 0.0f};

  /// 边界框的半径范围
  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  FVector Extent = {0.0f, 0.0f, 0.0f};

  /// 边界框的旋转
  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  FRotator Rotation = {0.0f, 0.0f, 0.0f};
};
