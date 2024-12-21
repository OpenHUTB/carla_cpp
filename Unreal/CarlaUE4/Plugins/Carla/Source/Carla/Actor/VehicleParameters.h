// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
// 包含Carla中轮式车辆的定义
#include "Carla/Vehicle/CarlaWheeledVehicle.h"
// 为FVehicleParameters结构体生成必要的USTRUCT宏和结构体元数据
#include "VehicleParameters.generated.h"
// 定义一个用于车辆参数的结构体，可以在蓝图（Blueprint）中使用
USTRUCT(BlueprintType)
struct CARLA_API FVehicleParameters
{
  GENERATED_BODY()
// 车辆制造商名称，可在编辑器中编辑，也可在蓝图中读写
  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  FString Make;
// 车辆型号名称，可在编辑器中编辑，也可在蓝图中读写
  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  FString Model;
// 车辆类，继承自ACarlaWheeledVehicle，可在编辑器中编辑，也可在蓝图中读写
  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  TSubclassOf<ACarlaWheeledVehicle> Class;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  int32 NumberOfWheels = 4;
// 车辆代数，可在编辑器中编辑，也可在蓝图中读写
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
// 车辆是否有动态门，可在编辑器中编辑，也可在蓝图中读写
  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  bool HasDynamicDoors = false;
// 车辆是否有灯光，可在编辑器中编辑，也可在蓝图中读
  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  bool HasLights = false;
// 推荐的车辆颜色数组，可在编辑器中编辑，也可在蓝图中读写
  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  TArray<FColor> RecommendedColors;

  /// 此车辆支持的驾驶员（行人）的ID列表，如果没有任何驾驶员支持，则留空
  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  TArray<int32> SupportedDrivers;
};
