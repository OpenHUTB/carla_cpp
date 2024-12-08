// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "VehicleTelemetryData.generated.h"

// ����ң�����ݽṹ
USTRUCT(BlueprintType)
struct FWheelTelemetryData
{
  GENERATED_USTRUCT_BODY()

  // ��̥Ħ����
  UPROPERTY(Category = "Wheel Telemetry Data", EditAnywhere, BlueprintReadWrite)
  float TireFriction = 0.0f;

  // �����ƽǶȣ��ȣ�
  UPROPERTY(Category = "Wheel Telemetry Data", EditAnywhere, BlueprintReadWrite)
  float LatSlip = 0.0f;  // degrees

  // ������
  UPROPERTY(Category = "Wheel Telemetry Data", EditAnywhere, BlueprintReadWrite)
  float LongSlip = 0.0f;

  // ���ٶ�
  UPROPERTY(Category = "Wheel Telemetry Data", EditAnywhere, BlueprintReadWrite)
  float Omega = 0.0f;

  // ��̥����
  UPROPERTY(Category = "Wheel Telemetry Data", EditAnywhere, BlueprintReadWrite)
  float TireLoad = 0.0f;

  // ��׼����̥����
  UPROPERTY(Category = "Wheel Telemetry Data", EditAnywhere, BlueprintReadWrite)
  float NormalizedTireLoad = 0.0f;

  // Ť�أ�ţ�ף�
  UPROPERTY(Category = "Wheel Telemetry Data", EditAnywhere, BlueprintReadWrite)
  float Torque = 0.0f;  // [Nm]

  // ��������ţ�٣�
  UPROPERTY(Category = "Wheel Telemetry Data", EditAnywhere, BlueprintReadWrite)
  float LongForce = 0.0f;  // [N]

  // ��������ţ�٣�
  UPROPERTY(Category = "Wheel Telemetry Data", EditAnywhere, BlueprintReadWrite)
  float LatForce  = 0.0f;  // [N]

  // ��׼��������
  UPROPERTY(Category = "Wheel Telemetry Data", EditAnywhere, BlueprintReadWrite)
  float NormalizedLongForce = 0.0f;

  // ��׼��������
  UPROPERTY(Category = "Wheel Telemetry Data", EditAnywhere, BlueprintReadWrite)
  float NormalizedLatForce  = 0.0f;
};

// ����ң�����ݽṹ
USTRUCT(BlueprintType)
struct CARLA_API FVehicleTelemetryData
{
  GENERATED_BODY()

  // �ٶȣ���/�룩
  UPROPERTY(Category = "Vehicle Telemetry Data", EditAnywhere, BlueprintReadWrite)
  float Speed = 0.0f;  // [m/s]

  // ת��Ƕ�
  UPROPERTY(Category = "Vehicle Telemetry Data", EditAnywhere, BlueprintReadWrite)
  float Steer = 0.0f;

  // ����
  UPROPERTY(Category = "Vehicle Telemetry Data", EditAnywhere, BlueprintReadWrite)
  float Throttle = 0.0f;

  // ɲ��
  UPROPERTY(Category = "Vehicle Telemetry Data", EditAnywhere, BlueprintReadWrite)
  float Brake = 0.0f;

  // ������ת��
  UPROPERTY(Category = "Vehicle Telemetry Data", EditAnywhere, BlueprintReadWrite)
  float EngineRPM = 0.0f;

  // ��λ
  UPROPERTY(Category = "Vehicle Telemetry Data", EditAnywhere, BlueprintReadWrite)
  int32 Gear = 0.0f;

  // ����������ţ�٣�
  UPROPERTY(Category = "Vehicle Telemetry Data", EditAnywhere, BlueprintReadWrite)
  float Drag = 0.0f;  // [N]

  // ����ң����������
  UPROPERTY(Category = "Vehicle Engine Physics Control", EditAnywhere, BlueprintReadWrite)
  TArray<FWheelTelemetryData> Wheels;
};