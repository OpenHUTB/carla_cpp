// Copyright (c) 2021 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "VehicleAckermannControl.generated.h"

// FVehicleAckermannControl结构体用于表示车辆阿克曼转向控制相关的参数。
// 它可以在虚幻引擎（Unreal Engine）的蓝图系统中使用（通过BlueprintType标记）。
USTRUCT(BlueprintType)
struct CARLA_API FVehicleAckermannControl
{
  GENERATED_BODY()

  // 车辆的转向角度，单位通常为弧度（具体取决于使用场景和相关设定），默认值为0.0f，表示车辆处于直线行驶状态。
  UPROPERTY(Category = "Vehicle Ackermann Control", EditAnywhere, BlueprintReadWrite)
  float Steer = 0.0f;

  // 转向速度，用于描述车辆转向角度变化的快慢程度，单位可能与时间相关（例如每秒变化的弧度数等），默认值为0.0f。
  UPROPERTY(Category = "Vehicle Ackermann Control", EditAnywhere, BlueprintReadWrite)
  float SteerSpeed = 0.0f;

  // 车辆的当前行驶速度，单位可能是米每秒等常见速度单位，默认值为0.0f，表示车辆静止。
  UPROPERTY(Category = "Vehicle Ackermann Control", EditAnywhere, BlueprintReadWrite)
  float Speed = 0.0f;

  // 车辆的加速度，描述速度变化的快慢，单位通常与速度单位和时间单位相关（例如米每二次方秒），默认值为0.0f。
  UPROPERTY(Category = "Vehicle Ackermann Control", EditAnywhere, BlueprintReadWrite)
  float Acceleration = 0.0f;

  // 加加速度（急动度），用于衡量加速度变化的快慢，单位通常与加速度单位和时间单位相关（例如米每三次方秒），默认值为0.0f。
  UPROPERTY(Category = "Vehicle Ackermann Control", EditAnywhere, BlueprintReadWrite)
  float Jerk = 0.0f;
};
