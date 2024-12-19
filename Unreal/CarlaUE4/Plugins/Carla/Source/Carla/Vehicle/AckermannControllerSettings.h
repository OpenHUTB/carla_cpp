// Copyright (c) 2021 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "AckermannControllerSettings.generated.h"
// ����һ����ΪFAckermannControllerSettings�Ľṹ�壬���ڴ洢������������������
USTRUCT(BlueprintType)
struct CARLA_API FAckermannControllerSettings
{
  GENERATED_BODY()
  // �ٶȿ��Ƶı���ϵ��
  UPROPERTY(Category = "Ackermann Controller Settings", EditAnywhere, BlueprintReadWrite)
  float SpeedKp = 0.0f;
  // �ٶȿ��ƵĻ���ϵ��
  UPROPERTY(Category = "Ackermann Controller Settings", EditAnywhere, BlueprintReadWrite)
  float SpeedKi = 0.0f;
  // �ٶȿ��Ƶ�΢��ϵ��
  UPROPERTY(Category = "Ackermann Controller Settings", EditAnywhere, BlueprintReadWrite)
  float SpeedKd = 0.0f;
  // ���ٶȿ��Ƶı���ϵ��
  UPROPERTY(Category = "Ackermann Controller Settings", EditAnywhere, BlueprintReadWrite)
  float AccelKp = 0.0f;
  // ���ٶȿ��ƵĻ���ϵ��
  UPROPERTY(Category = "Ackermann Controller Settings", EditAnywhere, BlueprintReadWrite)
  float AccelKi = 0.0f;
  // ���ٶȿ��Ƶ�΢��ϵ��
  UPROPERTY(Category = "Ackermann Controller Settings", EditAnywhere, BlueprintReadWrite)
  float AccelKd = 0.0f;
};
