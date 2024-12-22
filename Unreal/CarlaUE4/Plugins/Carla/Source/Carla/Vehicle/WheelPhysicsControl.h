// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
// 确保头文件只被编译一次，避免重复包含导致的编译错误


#include "WheelPhysicsControl.generated.h"
// 包含自动生成的相关头文件，通常与虚幻引擎的反射等机制相关的代码生成有关


USTRUCT(BlueprintType)
struct CARLA_API FWheelPhysicsControl
{
  GENERATED_BODY()
// 这是一个由虚幻引擎代码生成工具使用的宏，用于生成必要的反射等相关代码，不需要手动实现其具体内容

    // 定义一个属性（UPROPERTY宏用于将成员变量暴露给虚幻引擎的反射系统等），所属分类为"Wheel Tire Friction"，表示轮胎摩擦力相关
    // 该属性可在编辑器中任意位置编辑（EditAnywhere），并且在蓝图中可读可写（BlueprintReadWrite）

  UPROPERTY(Category = "Wheel Tire Friction", EditAnywhere, BlueprintReadWrite)
  float TireFriction = 3.5f;
// 定义轮胎摩擦力系数，初始值设为3.5f

    // 定义一个属性，所属分类为"Wheel Damping Rate"，即车轮阻尼率相关
    // 可在编辑器中任意位置编辑，在蓝图中可读可写

  UPROPERTY(Category = "Wheel Damping Rate", EditAnywhere, BlueprintReadWrite)
  float DampingRate = 1.0f;
// 定义车轮阻尼率，初始值设为1.0f

    // 定义一个属性，所属分类为"Wheel Max Steer Angle"，也就是车轮最大转向角度相关

  UPROPERTY(Category = "Wheel Max Steer Angle", EditAnywhere, BlueprintReadWrite)
  float MaxSteerAngle = 70.0f;
 // 定义车轮最大转向角度，初始值设为70.0f

    // 定义一个属性，所属分类为"Wheel Shape Radius"，表示车轮形状半径相关
    // 可在编辑器中任意位置编辑，在蓝图中可读可写

  UPROPERTY(Category = "Wheel Shape Radius", EditAnywhere, BlueprintReadWrite)
  float Radius = 30.0f;
// 定义车轮形状半径，初始值设为30.0f

    // 定义一个属性，所属分类为"Wheel Max Brake Torque (Nm)"，即车轮最大刹车扭矩（单位为牛米）相关
    // 可在编辑器中任意位置编辑，在蓝图中可读可写

  UPROPERTY(Category = "Wheel Max Brake Torque (Nm)", EditAnywhere, BlueprintReadWrite)
  float MaxBrakeTorque = 1500.0f;
// 定义车轮最大刹车扭矩，初始值设为1500.0f

    // 定义一个属性，所属分类为"Wheel Max Handbrake Torque (Nm)"，也就是车轮最大手刹扭矩（单位为牛米）相关
// 可在编辑器中任意位置编辑，在蓝图中可读可写
  UPROPERTY(Category = "Wheel Max Handbrake Torque (Nm)", EditAnywhere, BlueprintReadWrite)
  float MaxHandBrakeTorque = 3000.0f;
 // 定义车轮最大手刹扭矩，初始值设为3000.0f

    // 定义一个属性，注释说明了它代表的含义：在该最大归一化轮胎负载下，无论再给轮胎施加多少额外负载，轮胎都无法再提供更多的横向刚度

  UPROPERTY(Category = "Max normalized tire load at which the tire can deliver no more lateral stiffness no matter how much extra load is applied to the tire", EditAnywhere, BlueprintReadWrite)
  float LatStiffMaxLoad = 2.0f;
// 定义上述相关的最大负载值，初始值设为2.0f

    // 定义一个属性，所属分类为"Lateral Stiffness Value"，即横向刚度值相关
    // 可在编辑器中任意位置编辑，在蓝图中可读可写

  UPROPERTY(Category = "Lateral Stiffness Value", EditAnywhere, BlueprintReadWrite)
  float LatStiffValue = 17.0f;
// 定义横向刚度值，初始值设为17.0f

    // 定义一个属性，所属分类为"Longitudinal Stiffness Value"，也就是纵向刚度值相关
    // 可在编辑器中任意位置编辑，在蓝图中可读可写

  UPROPERTY(Category = "Longitudinal Stiffness Value", EditAnywhere, BlueprintReadWrite)
  float LongStiffValue = 1000.0f;
// 定义纵向刚度值，初始值设为1000.0f

    // 定义一个属性，所属分类为"Wheel Position"，表示车轮位置相关
    // 可在编辑器中任意位置编辑，在蓝图中可读可写

  UPROPERTY(Category = "Wheel Position", EditAnywhere, BlueprintReadWrite)
  FVector Position = FVector::ZeroVector;
// 定义车轮位置，初始化为零向量，表示默认位置在原点
};
