// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
// 这是一个预处理指令，用于确保该头文件在整个编译单元中只会被编译一次，避免重复包含导致的编译错误，例如重复定义结构体等情况。

#include "VehicleTelemetryData.generated.h"
// 包含一个由代码生成工具自动生成的头文件，里面包含了本结构体（FVehicleTelemetryData 以及其相关结构体）相关的一些额外代码，
// 例如反射相关的代码、序列化相关代码等，帮助项目能更好地在虚幻引擎等环境下管理和使用这些结构体，比如在蓝图中进行可视化操作等。

USTRUCT(BlueprintType)
// 使用 USTRUCT 宏定义一个结构体，同时指定了该结构体具有 BlueprintType 属性，意味着这个结构体可以在虚幻引擎的蓝图可视化编程环境中使用，
// 例如可以作为变量类型在蓝图中声明、传递以及进行相关属性的读写操作等，将结构体与蓝图相关功能进行了集成。
struct FWheelTelemetryData
 // 定义一个名为 FWheelTelemetryData 的结构体，用于存储与车轮相关的遥测数据信息，后续在结构体内部定义了多个成员变量来分别表示不同的具体数据。
{
  GENERATED_USTRUCT_BODY()
// 这是一个由虚幻引擎代码生成系统使用的宏，用于自动生成结构体相关的必要代码，比如默认构造函数、析构函数等默认实现部分，以及其他一些和虚幻引擎内部机制相关的代码，
    // 使得结构体能符合虚幻引擎的要求正常工作。

  UPROPERTY(Category = "Wheel Telemetry Data", EditAnywhere, BlueprintReadWrite)
// 使用 UPROPERTY 宏标记一个成员变量，用于设置该变量在虚幻引擎中的相关属性。
    // Category 属性指定了在虚幻引擎编辑器的属性面板中该变量所属的分类为 "Wheel Telemetry Data"，方便对相关属性进行分类查看和管理；
    // EditAnywhere 表示这个变量在编辑器中可以在任何合适的地方进行编辑修改；
    // BlueprintReadWrite 说明该变量在蓝图中既可以读取其值也可以对其进行写入操作，提供了蓝图与代码之间对该变量交互的便利性。

  float TireFriction = 0.0f;
// 定义一个名为 TireFriction 的浮点型成员变量，用于存储轮胎摩擦力相关的数据，初始化为 0.0f，单位可能需要根据具体项目的物理模拟设置等来确定，
    // 在这里它可以反映车轮与地面等接触表面之间的摩擦力情况，对车辆的行驶、操控等物理特性有影响。

  UPROPERTY(Category = "Wheel Telemetry Data", EditAnywhere, BlueprintReadWrite)
  float LatSlip = 0.0f;  // degrees
// 定义 LatSlip 浮点型成员变量，用于存储横向滑移量的数据，初始化为 0.0f，注释中注明单位是“度（degrees）”，
    // 横向滑移量可以体现车轮在横向方向上相对于理想行驶轨迹的滑动情况，常用于车辆操控性、稳定性相关的物理模拟和分析中。

  UPROPERTY(Category = "Wheel Telemetry Data", EditAnywhere, BlueprintReadWrite)
  float LongSlip = 0.0f;
// 定义 LongSlip 浮点型成员变量，用于存储纵向滑移量的数据，初始化为 0.0f，
    // 纵向滑移量反映车轮在车辆行驶方向（纵向）上的滑动情况，比如在加速、刹车过程中车轮是否出现打滑等现象，对车辆动力传递、制动效果等方面的模拟很重要。

  UPROPERTY(Category = "Wheel Telemetry Data", EditAnywhere, BlueprintReadWrite)
  float Omega = 0.0f;
// 定义 Omega 浮点型成员变量，用于存储车轮角速度相关的数据，初始化为 0.0f，
    // 车轮角速度可以用于计算车轮的转速等信息，与车辆的行驶速度、挡位等因素相关联，在车辆物理模拟中用于描述车轮的转动状态。

  UPROPERTY(Category = "Wheel Telemetry Data", EditAnywhere, BlueprintReadWrite)
  float TireLoad = 0.0f;
// 定义 TireLoad 浮点型成员变量，用于存储轮胎负载相关的数据，初始化为 0.0f，
    // 轮胎负载表示轮胎所承受的重量等作用力情况，会影响轮胎的摩擦力、变形等特性，进而影响车辆的整体行驶性能和操控性能。

  UPROPERTY(Category = "Wheel Telemetry Data", EditAnywhere, BlueprintReadWrite)
  float NormalizedTireLoad = 0.0f;
// 定义 NormalizedTireLoad 浮点型成员变量，用于存储归一化的轮胎负载相关的数据，初始化为 0.0f，
    // 归一化处理通常是将实际的负载值映射到某个特定的范围（比如 0 到 1 之间等），方便在一些相对比较、比例计算等场景中使用，使得数据处理更加标准化和统一。


  UPROPERTY(Category = "Wheel Telemetry Data", EditAnywhere, BlueprintReadWrite)
  float Torque = 0.0f;  // [Nm]
 // 定义 Torque 浮点型成员变量，用于存储扭矩相关的数据，初始化为 0.0f，注释中注明单位是“牛米（Nm）”，
    // 车轮扭矩与车辆的动力输出、传动系统等相关，决定了车轮转动的驱动力大小，对车辆的加速、爬坡等性能有直接影响。


  UPROPERTY(Category = "Wheel Telemetry Data", EditAnywhere, BlueprintReadWrite)
  float LongForce = 0.0f;  // [N]
// 定义 LongForce 浮点型成员变量，用于存储纵向力相关的数据，初始化为 0.0f，注释中注明单位是“牛（N）”，
    // 纵向力通常是指车辆行驶方向上作用在车轮上的力，比如驱动力、制动力等，是分析车辆动力性能和制动性能的重要参数。

  UPROPERTY(Category = "Wheel Telemetry Data", EditAnywhere, BlueprintReadWrite)
  float LatForce  = 0.0f;  // [N]
// 定义 LatForce 浮点型成员变量，用于存储横向力相关的数据，初始化为 0.0f，注释中注明单位是“牛（N）”，
    // 横向力是作用在车轮横向方向上的力，对于车辆的转向、操控稳定性等方面起着关键作用，例如在车辆转弯时，横向力决定了车辆能否保持稳定的转弯轨迹。


  UPROPERTY(Category = "Wheel Telemetry Data", EditAnywhere, BlueprintReadWrite)
  float NormalizedLongForce = 0.0f;
// 定义 NormalizedLongForce 浮点型成员变量，用于存储归一化的纵向力相关的数据，初始化为 0.0f，
    // 与前面的归一化轮胎负载类似，将纵向力进行归一化处理，便于在一些相对比较、统一分析等场景中使用该数据。

  UPROPERTY(Category = "Wheel Telemetry Data", EditAnywhere, BlueprintReadWrite)
  float NormalizedLatForce  = 0.0f;
 // 定义 NormalizedLatForce 浮点型成员变量，用于存储归一化的横向力相关的数据，初始化为 0.0f，
    // 同样是对横向力进行归一化处理，方便在不同条件下对横向力进行相对比较、综合分析等操作，以更好地评估车辆的操控性能等。
};

USTRUCT(BlueprintType)
// 再次使用 USTRUCT 宏定义一个结构体，并指定其具有 BlueprintType 属性，使其能在虚幻引擎蓝图环境中使用，与前面定义结构体的方式类似，用于定义另一个更综合的结构体。
struct CARLA_API FVehicleTelemetryData
// 定义名为 FVehicleTelemetryData 的结构体，从命名推测是用于存储车辆整体的遥测数据信息，CARLA_API 可能是用于指定该结构体在整个 Carla 项目（推测是基于虚幻引擎开发的某个项目）中的对外接口访问权限等相关特性。
{
  GENERATED_BODY()
// 同样是由虚幻引擎代码生成系统使用的宏，用于自动生成结构体相关的必要代码，确保结构体符合虚幻引擎的使用要求。

  UPROPERTY(Category = "Vehicle Telemetry Data", EditAnywhere, BlueprintReadWrite)
  float Speed = 0.0f;  // [m/s]
// 定义 Speed 浮点型成员变量，用于存储车辆速度相关的数据，初始化为 0.0f，注释中注明单位是“米每秒（m/s）”，这是表示车辆行驶快慢的基本物理量，
    // 在车辆的运动模拟、性能分析以及各种与行驶状态相关的逻辑判断中都会用到。

  UPROPERTY(Category = "Vehicle Telemetry Data", EditAnywhere, BlueprintReadWrite)
  float Steer = 0.0f;
// 定义 Steer 浮点型成员变量，用于存储车辆转向相关的数据，初始化为 0.0f，
    // 其数值可能表示转向的角度、转向盘的转角等相关信息，用于控制车辆的行驶方向，反映车辆的转向状态，在车辆操控模拟中起着关键作用。


  UPROPERTY(Category = "Vehicle Telemetry Data", EditAnywhere, BlueprintReadWrite)
  float Throttle = 0.0f;
// 定义 Throttle 浮点型成员变量，用于存储油门相关的数据，初始化为 0.0f，
    // 油门开度大小通常由该变量表示，它决定了发动机的动力输出，进而影响车辆的加速性能，是车辆动力控制方面的重要参数。

  UPROPERTY(Category = "Vehicle Telemetry Data", EditAnywhere, BlueprintReadWrite)
  float Brake = 0.0f;
// 定义 Brake 浮点型成员变量，用于存储刹车相关的数据，初始化为 0.0f，
    // 刹车力度、刹车踏板行程等与刹车相关的信息可能通过该变量体现，用于控制车辆的减速和停车操作，是保障车辆安全行驶以及模拟制动过程的关键参数。

  UPROPERTY(Category = "Vehicle Telemetry Data", EditAnywhere, BlueprintReadWrite)
  float EngineRPM = 0.0f;
// 定义 EngineRPM 浮点型成员变量，用于存储发动机转速相关的数据，初始化为 0.0f，
    // 发动机转速是反映发动机工作状态的重要指标，与车辆的挡位、速度、动力输出等多个因素相互关联，在车辆动力系统模拟和性能分析中具有重要意义。

  UPROPERTY(Category = "Vehicle Telemetry Data", EditAnywhere, BlueprintReadWrite)
  int32 Gear = 0.0f;
// 定义 Gear 整型成员变量，用于存储挡位相关的数据，初始化为 0，
    // 挡位信息决定了车辆的传动比等参数，影响发动机动力传递到车轮的效果，不同挡位适用于不同的行驶速度和工况，对车辆的整体性能有着重要影响。

  UPROPERTY(Category = "Vehicle Telemetry Data", EditAnywhere, BlueprintReadWrite)
  float Drag = 0.0f;  // [N]
// 定义 Drag 浮点型成员变量，用于存储空气阻力相关的数据，初始化为 0.0f，注释中注明单位是“牛（N）”，
    // 空气阻力是车辆在行驶过程中受到的与空气相互作用产生的阻碍力，会影响车辆的能耗、最高速度等性能指标，在车辆物理模拟中需要考虑该因素。


  UPROPERTY(Category = "Vehicle Engine Physics Control", EditAnywhere, BlueprintReadWrite)
  TArray<FWheelTelemetryData> Wheels;
// 定义 Wheels 成员变量，类型是 TArray<FWheelTelemetryData>，即一个存储 FWheelTelemetryData 结构体的动态数组，
    // 通过 UPROPERTY 宏设置其属性，所属分类为 "Vehicle Engine Physics Control"，同样具有可编辑和在蓝图中读写的特性，
    // 用于存储车辆所有车轮的遥测数据信息，通过这个数组可以方便地管理和访问每个车轮的详细物理数据，实现对车辆整体行驶状态更全面的监控和模拟。
};
};
