// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "VehicleTelemetryData.generated.h"

// 车轮遥测数据结构
USTRUCT(BlueprintType)
struct FWheelTelemetryData
{
  GENERATED_USTRUCT_BODY()

  // 轮胎摩擦力
  UPROPERTY(Category = "Wheel Telemetry Data", EditAnywhere, BlueprintReadWrite)
  float TireFriction = 0.0f;

  // 横向滑移角度（度）
  UPROPERTY(Category = "Wheel Telemetry Data", EditAnywhere, BlueprintReadWrite)
  float LatSlip = 0.0f;  // degrees

  // 纵向滑移
  UPROPERTY(Category = "Wheel Telemetry Data", EditAnywhere, BlueprintReadWrite)
  float LongSlip = 0.0f;

  // 角速度
  UPROPERTY(Category = "Wheel Telemetry Data", EditAnywhere, BlueprintReadWrite)
  float Omega = 0.0f;

  // 轮胎负载
  UPROPERTY(Category = "Wheel Telemetry Data", EditAnywhere, BlueprintReadWrite)
  float TireLoad = 0.0f;

  // 标准化轮胎负载
  UPROPERTY(Category = "Wheel Telemetry Data", EditAnywhere, BlueprintReadWrite)
  float NormalizedTireLoad = 0.0f;

  // 扭矩（牛米）
  UPROPERTY(Category = "Wheel Telemetry Data", EditAnywhere, BlueprintReadWrite)
  float Torque = 0.0f;  // [Nm]

  // 纵向力（牛顿）
  UPROPERTY(Category = "Wheel Telemetry Data", EditAnywhere, BlueprintReadWrite)
  float LongForce = 0.0f;  // [N]

  // 横向力（牛顿）
  UPROPERTY(Category = "Wheel Telemetry Data", EditAnywhere, BlueprintReadWrite)
  float LatForce  = 0.0f;  // [N]

  // 标准化纵向力
  UPROPERTY(Category = "Wheel Telemetry Data", EditAnywhere, BlueprintReadWrite)
  float NormalizedLongForce = 0.0f;

  // 标准化横向力
  UPROPERTY(Category = "Wheel Telemetry Data", EditAnywhere, BlueprintReadWrite)
  float NormalizedLatForce  = 0.0f;
};

// 车辆遥测数据结构
USTRUCT(BlueprintType)
struct CARLA_API FVehicleTelemetryData
{
  GENERATED_BODY()

  // 速度（米/秒）
  UPROPERTY(Category = "Vehicle Telemetry Data", EditAnywhere, BlueprintReadWrite)
  float Speed = 0.0f;  // [m/s]

  // 转向角度
  UPROPERTY(Category = "Vehicle Telemetry Data", EditAnywhere, BlueprintReadWrite)
  float Steer = 0.0f;

  // 油门
  UPROPERTY(Category = "Vehicle Telemetry Data", EditAnywhere, BlueprintReadWrite)
  float Throttle = 0.0f;

  // 刹车
  UPROPERTY(Category = "Vehicle Telemetry Data", EditAnywhere, BlueprintReadWrite)
  float Brake = 0.0f;

  // 发动机转速
  UPROPERTY(Category = "Vehicle Telemetry Data", EditAnywhere, BlueprintReadWrite)
  float EngineRPM = 0.0f;

  // 档位
  UPROPERTY(Category = "Vehicle Telemetry Data", EditAnywhere, BlueprintReadWrite)
  int32 Gear = 0.0f;

  // 空气阻力（牛顿）
  UPROPERTY(Category = "Vehicle Telemetry Data", EditAnywhere, BlueprintReadWrite)
  float Drag = 0.0f;  // [N]

  // 车轮遥测数据数组
  UPROPERTY(Category = "Vehicle Engine Physics Control", EditAnywhere, BlueprintReadWrite)
  TArray<FWheelTelemetryData> Wheels;
};