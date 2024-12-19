// Copyright (c) 2021 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "AckermannControllerSettings.generated.h"
// 定义一个名为FAckermannControllerSettings的结构体，用于存储阿克曼控制器的设置
USTRUCT(BlueprintType)
struct CARLA_API FAckermannControllerSettings
{
  GENERATED_BODY()
  // 速度控制的比例系数
  UPROPERTY(Category = "Ackermann Controller Settings", EditAnywhere, BlueprintReadWrite)
  float SpeedKp = 0.0f;
  // 速度控制的积分系数
  UPROPERTY(Category = "Ackermann Controller Settings", EditAnywhere, BlueprintReadWrite)
  float SpeedKi = 0.0f;
  // 速度控制的微分系数
  UPROPERTY(Category = "Ackermann Controller Settings", EditAnywhere, BlueprintReadWrite)
  float SpeedKd = 0.0f;
  // 加速度控制的比例系数
  UPROPERTY(Category = "Ackermann Controller Settings", EditAnywhere, BlueprintReadWrite)
  float AccelKp = 0.0f;
  // 加速度控制的积分系数
  UPROPERTY(Category = "Ackermann Controller Settings", EditAnywhere, BlueprintReadWrite)
  float AccelKi = 0.0f;
  // 加速度控制的微分系数
  UPROPERTY(Category = "Ackermann Controller Settings", EditAnywhere, BlueprintReadWrite)
  float AccelKd = 0.0f;
};
