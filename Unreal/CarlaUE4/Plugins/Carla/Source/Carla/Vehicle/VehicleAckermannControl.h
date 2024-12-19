// Copyright (c) 2021 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "VehicleAckermannControl.generated.h"
// ���峵�����������ƽṹ��
USTRUCT(BlueprintType)
struct CARLA_API FVehicleAckermannControl
{
  GENERATED_BODY()
  // ת��Ƕȣ���Χͨ���� [-1, 1] ֮��
  UPROPERTY(Category = "Vehicle Ackermann Control", EditAnywhere, BlueprintReadWrite)
  float Steer = 0.0f;
  // ת���ٶȣ���ʾ������ת��������
  UPROPERTY(Category = "Vehicle Ackermann Control", EditAnywhere, BlueprintReadWrite)
  float SteerSpeed = 0.0f;
  // �����ٶȣ�ͨ������/��Ϊ��λ
  UPROPERTY(Category = "Vehicle Ackermann Control", EditAnywhere, BlueprintReadWrite)
  float Speed = 0.0f;
  // ���ٶȣ���ʾ�ٶȱ仯�ʣ�ͨ������/���ƽ�� Ϊ��λ
  UPROPERTY(Category = "Vehicle Ackermann Control", EditAnywhere, BlueprintReadWrite)
  float Acceleration = 0.0f;
  // �Ӽ��ٶȣ����ٶȵı仯�ʣ���ͨ������/������η� Ϊ��λ
  UPROPERTY(Category = "Vehicle Ackermann Control", EditAnywhere, BlueprintReadWrite)
  float Jerk = 0.0f;
};

