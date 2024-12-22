// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
// Copyright (c) 2019 Intel Corporation
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Vehicle/WheelPhysicsControl.h"
#include "VehiclePhysicsControl.generated.h"

USTRUCT(BlueprintType)
struct FGearPhysicsControl
{
  GENERATED_USTRUCT_BODY()

  UPROPERTY(Category = "Gear Physics Control", EditAnywhere, BlueprintReadWrite)
  float Ratio = 1.0f;

  UPROPERTY(Category = "Gear Physics Control", EditAnywhere, BlueprintReadWrite)
  float DownRatio = 0.5f;

  UPROPERTY(Category = "Gear Physics Control", EditAnywhere, BlueprintReadWrite)
  float UpRatio = 0.65f;
};

USTRUCT(BlueprintType)
struct CARLA_API FVehiclePhysicsControl
{
  GENERATED_BODY()

  // 机械设置

  // 引擎设置
  FRichCurve TorqueCurve;

  UPROPERTY(Category = "Vehicle Engine Physics Control", EditAnywhere, BlueprintReadWrite)
  float MaxRPM = 0.0f;

  UPROPERTY(Category = "Vehicle Engine Physics Control", EditAnywhere, BlueprintReadWrite)
  float MOI = 0.0f;

  UPROPERTY(Category = "Vehicle Engine Physics Control", EditAnywhere, BlueprintReadWrite)
  float DampingRateFullThrottle = 0.0f;

  UPROPERTY(Category = "Vehicle Engine Physics Control", EditAnywhere, BlueprintReadWrite)
  float DampingRateZeroThrottleClutchEngaged = 0.0f;

  UPROPERTY(Category = "Vehicle Engine Physics Control", EditAnywhere, BlueprintReadWrite)
  float DampingRateZeroThrottleClutchDisengaged = 0.0f;

  // // 传输设置
  UPROPERTY(Category = "Vehicle Engine Physics Control", EditAnywhere, BlueprintReadWrite)
  bool bUseGearAutoBox = 0.0f;

  UPROPERTY(Category = "Vehicle Engine Physics Control", EditAnywhere, BlueprintReadWrite)
  float GearSwitchTime = 0.0f;

  UPROPERTY(Category = "Vehicle Engine Physics Control", EditAnywhere, BlueprintReadWrite)
  float ClutchStrength = 0.0f;

  UPROPERTY(Category = "Vehicle Engine Physics Control", EditAnywhere, BlueprintReadWrite)
  float FinalRatio = 1.0f;

  UPROPERTY(Category = "Vehicle Engine Physics Control", EditAnywhere, BlueprintReadWrite)
  TArray<FGearPhysicsControl> ForwardGears;

  // 车辆设置
  UPROPERTY(Category = "Vehicle Engine Physics Control", EditAnywhere, BlueprintReadWrite)
  float Mass = 0.0f;

  UPROPERTY(Category = "Vehicle Engine Physics Control", EditAnywhere, BlueprintReadWrite)
  float DragCoefficient = 0.0f;

  // 转向设置
  FRichCurve SteeringCurve;

  // 质心
  UPROPERTY(Category = "Vehicle Center Of Mass", EditAnywhere, BlueprintReadWrite)
  FVector CenterOfMass;

  // 车轮设置
  TArray<FWheelPhysicsControl> Wheels;

  UPROPERTY(Category = "Vehicle Wheels Configuration", EditAnywhere, BlueprintReadWrite)
  bool UseSweepWheelCollision = false;
};
