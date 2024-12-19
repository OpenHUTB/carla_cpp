// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Vehicle/CarlaWheeledVehicle.h"

#include "VehicleParameters.generated.h"

USTRUCT(BlueprintType)
struct CARLA_API FVehicleParameters
{
  GENERATED_BODY()

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  FString Make;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  FString Model;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  TSubclassOf<ACarlaWheeledVehicle> Class;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  int32 NumberOfWheels = 4;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  int32 Generation = 0;

  /// （可选）用于车辆的自定义分类
  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  FString ObjectType;

  /// （可选）用于车辆的基本分类（汽车/卡车/货车/自行车/摩托车）
  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  FString BaseType;

  /// （可选）用于对车辆进行更深入的分类（电动/应急/出租车等）
  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  FString SpecialType;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  bool HasDynamicDoors = false;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  bool HasLights = false;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  TArray<FColor> RecommendedColors;

  /// 此车辆支持的驾驶员（行人）的ID列表，如果没有任何驾驶员支持，则留空
  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  TArray<int32> SupportedDrivers;
};
