// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// 激光雷达传感器的基本描述
// 
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "LidarDescription.generated.h"

USTRUCT()
struct CARLA_API FLidarDescription
{
  GENERATED_BODY()

  /// 激光雷达的线数
  UPROPERTY(EditAnywhere)
  uint32 Channels = 32u;  // 默认是32线激光雷达

  /// 测量距离，单位：厘米
  UPROPERTY(EditAnywhere)
  float Range = 1000.0f;

  /// 每秒钟所有激光产生的点。
  UPROPERTY(EditAnywhere)  // UPROPERTY(EditAnywhere)宏将属性公开给UE编辑器，使得可以在UE编辑器中对这些属性进行修改，避免了多次编译的繁琐
  uint32 PointsPerSecond = 56000u;

  /// 激光雷达旋转频率
  UPROPERTY(EditAnywhere)
  float RotationFrequency = 10.0f;  // 默认每秒钟旋转10圈

  /// 最高激光的角度（以度为单位），从水平线开始计数，正值表示水平线以上。
  UPROPERTY(EditAnywhere)
  float UpperFovLimit = 10.0f;

  /// 最低激光的角度（以度为单位），从水平开始计数，负值表示在水平线以下。
  UPROPERTY(EditAnywhere)
  float LowerFovLimit = -30.0f;
  
  /// 水平视野（以度为单位），0 - 360。
  UPROPERTY(EditAnywhere)
  float HorizontalFov = 360.0f;

  /// 大气中的衰减率（单位：m^-1）
  UPROPERTY(EditAnywhere)
  float AtmospAttenRate = 0.004f;

  /// 该传感器使用的噪声/丢弃的随机种子。
  UPROPERTY(EditAnywhere)
  int RandomSeed = 0;

  /// 随机丢弃的点的一般比例。
  UPROPERTY(EditAnywhere)
  float DropOffGenRate = 0.45f;

  /// 对于基于强度的下降，高于该阈值的强度值没有任何点被下降。
  UPROPERTY(EditAnywhere)
  float DropOffIntensityLimit = 0.8f;

  /// 对于基于强度的下降，每个零强度点被下降的概率。
  UPROPERTY(EditAnywhere)
  float DropOffAtZeroIntensity = 0.4f;

  /// 是否在模拟器中显示激光命中的调试点。
  UPROPERTY(EditAnywhere)
  bool ShowDebugPoints = false;

  UPROPERTY(EditAnywhere)
  float NoiseStdDev = 0.0f;
};
