// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
// 预处理指令，确保该头文件在整个编译过程中只会被包含一次

#include "VehicleTelemetryData.generated.h"
// 引入由虚幻引擎的代码生成工具生成的头文件

USTRUCT(BlueprintType)
struct FWheelTelemetryData
{
// 使用USTRUCT宏定义一个结构体，使其能被虚幻引擎识别

  GENERATED_USTRUCT_BODY()
// 这是一个虚幻引擎代码生成相关的宏，用于指示编译器在这里生成必要的代码

  UPROPERTY(Category = "Wheel Telemetry Data", EditAnywhere, BlueprintReadWrite)
  float TireFriction = 0.0f;
// 使用UPROPERTY宏声明一个成员变量TireFriction，它是浮点型
// UPROPERTY宏的参数指定了这个属性所属的分类（Category）为"Wheel Telemetry Data"，
 // EditAnywhere表示在虚幻引擎的编辑器中可以在任何地方编辑这个属性的值，
// BlueprintReadWrite表示这个属性在蓝图中既能被读取也能被写入

  UPROPERTY(Category = "Wheel Telemetry Data", EditAnywhere, BlueprintReadWrite)
  float LatSlip = 0.0f;  // degrees
// 使用UPROPERTY宏声明一个成员变量LatSlip，浮点型

  UPROPERTY(Category = "Wheel Telemetry Data", EditAnywhere, BlueprintReadWrite)
  float LongSlip = 0.0f;
 // 使用UPROPERTY宏声明一个成员变量LongSlip，浮点型，初始化为0.0f

  UPROPERTY(Category = "Wheel Telemetry Data", EditAnywhere, BlueprintReadWrite)
  float Omega = 0.0f;
// 使用UPROPERTY宏声明一个成员变量Omega，浮点型，初始化为0.0f
//与轮胎相关的角速度等物理量有关
  UPROPERTY(Category = "Wheel Telemetry Data", EditAnywhere, BlueprintReadWrite)
  float TireLoad = 0.0f;
// 使用UPROPERTY宏声明一个成员变量TireLoad，浮点型，初始化为0.0f
//用于表示轮胎所承受的负载相关信息，遵循相应的属性编辑和蓝图读写规则。

  UPROPERTY(Category = "Wheel Telemetry Data", EditAnywhere, BlueprintReadWrite)
  float NormalizedTireLoad = 0.0f;
// 使用UPROPERTY宏声明一个成员变量NormalizedTireLoad，浮点型，初始化为0.0f，是经过归一化处理后的轮胎负载相关量

  UPROPERTY(Category = "Wheel Telemetry Data", EditAnywhere, BlueprintReadWrite)
  float Torque = 0.0f;  // [Nm]
// 使用UPROPERTY宏声明一个成员变量Torque，浮点型，初始化为0.0f，单位注释为“[Nm]”（表示牛顿米，是扭矩的单位），表示轮胎相关的扭矩信息

  UPROPERTY(Category = "Wheel Telemetry Data", EditAnywhere, BlueprintReadWrite)
  float LongForce = 0.0f;  // [N]
 // 用于表示轮胎纵向受力相关信息

  UPROPERTY(Category = "Wheel Telemetry Data", EditAnywhere, BlueprintReadWrite)
  float LatForce  = 0.0f;  // [N]
// 用于表示轮胎横向受力相关信息

  UPROPERTY(Category = "Wheel Telemetry Data", EditAnywhere, BlueprintReadWrite)
  float NormalizedLongForce = 0.0f;
//是经过归一化处理后的轮胎纵向受力相关量

  UPROPERTY(Category = "Wheel Telemetry Data", EditAnywhere, BlueprintReadWrite)
  float NormalizedLatForce  = 0.0f;
//是经过归一化处理后的轮胎横向受力相关量
};

USTRUCT(BlueprintType)
struct CARLA_API FVehicleTelemetryData
{
// 再次使用USTRUCT宏定义一个结构体，名为FVehicleTelemetryData，同样具备在虚幻引擎蓝图中使用的相关特性
// CARLA_API可能是用于指定这个结构体在库中的导出属性等情况，使其可以在外部正确访问

  GENERATED_BODY()
// 虚幻引擎代码生成相关的宏，用于生成支持结构体在虚幻引擎中高级特性的必要代码。

  UPROPERTY(Category = "Vehicle Telemetry Data", EditAnywhere, BlueprintReadWrite)
  float Speed = 0.0f;  // [m/s]
 // 使用UPROPERTY宏声明一个成员变量Speed，浮点型，初始化为0.0f，单位注释为“[m/s]”（表示米每秒，是速度的单位）

  UPROPERTY(Category = "Vehicle Telemetry Data", EditAnywhere, BlueprintReadWrite)
  float Steer = 0.0f;
// 使用UPROPERTY宏声明一个成员变量Steer，浮点型，初始化为0.0f，用于表示车辆的转向角度等相关信息

  UPROPERTY(Category = "Vehicle Telemetry Data", EditAnywhere, BlueprintReadWrite)
  float Throttle = 0.0f;
// 使用UPROPERTY宏声明一个成员变量Throttle，浮点型，初始化为0.0f，用于表示车辆的油门开度相关信息

  UPROPERTY(Category = "Vehicle Telemetry Data", EditAnywhere, BlueprintReadWrite)
  float Brake = 0.0f;
// 使用UPROPERTY宏声明一个成员变量Brake，浮点型，初始化为0.0f，
// 用于表示车辆的刹车力度等相关信息

  UPROPERTY(Category = "Vehicle Telemetry Data", EditAnywhere, BlueprintReadWrite)
  float EngineRPM = 0.0f;
// 使用UPROPERTY宏声明一个成员变量EngineRPM，浮点型，初始化为0.0f，用于表示车辆发动机的每分钟转数

  UPROPERTY(Category = "Vehicle Telemetry Data", EditAnywhere, BlueprintReadWrite)
  int32 Gear = 0.0f;
// 使用UPROPERTY宏声明一个成员变量Gear，32位整数型，用于表示车辆当前所处的挡位信息

  UPROPERTY(Category = "Vehicle Telemetry Data", EditAnywhere, BlueprintReadWrite)
  float Drag = 0.0f;  // [N]
// 使用UPROPERTY宏声明一个成员变量Drag，浮点型，初始化为0.0f，单位注释为“[N]”，用于表示车辆受到的空气阻力等相关的力的大小信息

  UPROPERTY(Category = "Vehicle Engine Physics Control", EditAnywhere, BlueprintReadWrite)
  TArray<FWheelTelemetryData> Wheels;
// 使用UPROPERTY宏声明一个成员变量Wheels，类型是TArray<FWheelTelemetryData>，即一个存储FWheelTelemetryData结构体的数组，
// 所属分类为"Vehicle Engine Physics Control"，在编辑器中可随处编辑，在蓝图中可读可写，
// 用于存储车辆各个车轮的相关遥测数据（通过FWheelTelemetryData结构体来表示每个车轮的详细数据）。
};
