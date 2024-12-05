// 版权所有 (c) 2017 巴塞罗那自治大学 (UAB) 计算机视觉中心 (CVC)。
//
// 本作品根据 MIT 许可证的条款授权。
// 如需副本，请访问 <https://opensource.org/licenses/MIT>。

#pragma once

#include "WheelPhysicsControl.generated.h"

// 车轮物理控制结构体
USTRUCT(BlueprintType)
struct CARLA_API FWheelPhysicsControl
{
  GENERATED_BODY()

  // 轮胎摩擦力
  UPROPERTY(Category = "Wheel Tire Friction", EditAnywhere, BlueprintReadWrite)
  float TireFriction = 3.5f;

  // 车轮阻尼率
  UPROPERTY(Category = "Wheel Damping Rate", EditAnywhere, BlueprintReadWrite)
  float DampingRate = 1.0f;

  // 车轮最大转向角度（度）
  UPROPERTY(Category = "Wheel Max Steer Angle", EditAnywhere, BlueprintReadWrite)
  float MaxSteerAngle = 70.0f;

  // 车轮形状半径（厘米）
  UPROPERTY(Category = "Wheel Shape Radius", EditAnywhere, BlueprintReadWrite)
  float Radius = 30.0f;

  // 车轮最大制动扭矩（牛顿米）
  UPROPERTY(Category = "Wheel Max Brake Torque (Nm)", EditAnywhere, BlueprintReadWrite)
  float MaxBrakeTorque = 1500.0f;

  // 车轮最大手刹扭矩（牛顿米）
  UPROPERTY(Category = "Wheel Max Handbrake Torque (Nm)", EditAnywhere, BlueprintReadWrite)
  float MaxHandBrakeTorque = 3000.0f;

  // 轮胎无法提供更多横向刚度的最大标准化轮胎负载
  UPROPERTY(Category = "Max normalized tire load at which the tire can deliver no more lateral stiffness no matter how much extra load is applied to the tire", EditAnywhere, BlueprintReadWrite)
  float LatStiffMaxLoad = 2.0f;

  // 横向刚度值
  UPROPERTY(Category = "Lateral Stiffness Value", EditAnywhere, BlueprintReadWrite)
  float LatStiffValue = 17.0f;

  // 纵向刚度值
  UPROPERTY(Category = "Longitudinal Stiffness Value", EditAnywhere, BlueprintReadWrite)
  float LongStiffValue = 1000.0f;

  // 车轮位置
  UPROPERTY(Category = "Wheel Position", EditAnywhere, BlueprintReadWrite)
  FVector Position = FVector::ZeroVector;
};