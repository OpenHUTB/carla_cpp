// ��Ȩ���� (c) 2017 �����������δ�ѧ (UAB) ������Ӿ����� (CVC)��
//
// ����Ʒ���� MIT ���֤��������Ȩ��
// ���踱��������� <https://opensource.org/licenses/MIT>��

#pragma once

#include "WheelPhysicsControl.generated.h"

// ����������ƽṹ��
USTRUCT(BlueprintType)
struct CARLA_API FWheelPhysicsControl
{
  GENERATED_BODY()

  // ��̥Ħ����
  UPROPERTY(Category = "Wheel Tire Friction", EditAnywhere, BlueprintReadWrite)
  float TireFriction = 3.5f;

  // ����������
  UPROPERTY(Category = "Wheel Damping Rate", EditAnywhere, BlueprintReadWrite)
  float DampingRate = 1.0f;

  // �������ת��Ƕȣ��ȣ�
  UPROPERTY(Category = "Wheel Max Steer Angle", EditAnywhere, BlueprintReadWrite)
  float MaxSteerAngle = 70.0f;

  // ������״�뾶�����ף�
  UPROPERTY(Category = "Wheel Shape Radius", EditAnywhere, BlueprintReadWrite)
  float Radius = 30.0f;

  // ��������ƶ�Ť�أ�ţ���ף�
  UPROPERTY(Category = "Wheel Max Brake Torque (Nm)", EditAnywhere, BlueprintReadWrite)
  float MaxBrakeTorque = 1500.0f;

  // ���������ɲŤ�أ�ţ���ף�
  UPROPERTY(Category = "Wheel Max Handbrake Torque (Nm)", EditAnywhere, BlueprintReadWrite)
  float MaxHandBrakeTorque = 3000.0f;

  // ��̥�޷��ṩ�������նȵ�����׼����̥����
  UPROPERTY(Category = "Max normalized tire load at which the tire can deliver no more lateral stiffness no matter how much extra load is applied to the tire", EditAnywhere, BlueprintReadWrite)
  float LatStiffMaxLoad = 2.0f;

  // ����ն�ֵ
  UPROPERTY(Category = "Lateral Stiffness Value", EditAnywhere, BlueprintReadWrite)
  float LatStiffValue = 17.0f;

  // ����ն�ֵ
  UPROPERTY(Category = "Longitudinal Stiffness Value", EditAnywhere, BlueprintReadWrite)
  float LongStiffValue = 1000.0f;

  // ����λ��
  UPROPERTY(Category = "Wheel Position", EditAnywhere, BlueprintReadWrite)
  FVector Position = FVector::ZeroVector;
};