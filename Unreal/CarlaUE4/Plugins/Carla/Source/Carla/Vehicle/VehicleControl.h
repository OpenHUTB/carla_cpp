// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once//只包含一次

#include "VehicleControl.generated.h"//包含‘VehicleControl.generated.h’文件

USTRUCT(BlueprintType)//声明一个）蓝图类型的结构体
struct CARLA_API FVehicleControl//定义结构体FVehicleControl（在CARLA_API下）
{
  GENERATED_BODY()//生成的主体代码部分（由工具自动生成相关代码）

  UPROPERTY(Category = "Vehicle Control", EditAnywhere, BlueprintReadWrite)
  float Throttle = 0.0f;//
（声明一个）属性，分类为‘车辆控制’，可在任意处编辑，在蓝图中可读可写
  UPROPERTY(Category = "Vehicle Control", EditAnywhere, BlueprintReadWrite)
  float Steer = 0.0f;//浮点数类型的油门变量，初始化为 0.0

  UPROPERTY(Category = "Vehicle Control", EditAnywhere, BlueprintReadWrite)
  float Brake = 0.0f;//浮点数类型的转向变量，初始化为 0.0

  UPROPERTY(Category = "Vehicle Control", EditAnywhere, BlueprintReadWrite)
  bool bHandBrake = false;//浮点数类型的刹车变量，初始化为 0.0

  UPROPERTY(Category = "Vehicle Control", EditAnywhere, BlueprintReadWrite)
  bool bReverse = false;//布尔类型的手刹变量，初始化为假（即手刹初始未拉起）

  UPROPERTY(Category = "Vehicle Control", EditAnywhere, BlueprintReadWrite)
  bool bManualGearShift = false;//尔类型的倒车状态变量，初始化为假（即初始非倒车状态）

  UPROPERTY(Category = "Vehicle Control", EditAnywhere, BlueprintReadWrite, meta = (EditCondition = bManualGearShift))//（声明一个）属性，分类为‘车辆控制’，可在任意处编辑，在蓝图中可读可写，其编辑条件为 bManualGearShift（手动换挡模式变量）
  int32 Gear = 0;//32 位整数类型的挡位变量，初始化为 0