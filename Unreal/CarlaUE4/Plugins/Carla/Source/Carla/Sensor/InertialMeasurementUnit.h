// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.
// 确保头文件只被包含一次
#pragma once
// 包含 Carla 传感器基类的头文件
#include "Carla/Sensor/Sensor.h"

#include "Carla/Actor/ActorDefinition.h"
#include "Carla/Actor/ActorDescription.h"
// 用于禁用 UE4 宏的头文件
#include <compiler/disable-ue4-macros.h>
// 包含 Carla 的几何 3D 向量类的头文件
#include "carla/geom/Vector3D.h"
// 用于启用 UE4 宏的头文件
#include <compiler/enable-ue4-macros.h>
// 包含标准库中的 array 头文件
#include <array>
// 为类生成代码的宏，表明此头文件会被 Unreal Engine 的反射系统处理
#include "InertialMeasurementUnit.generated.h"
// 将类声明为 Unreal Engine 的 UCLASS，以便使用 UE 的反射和序列化等功能
UCLASS()
UCLASS()
class CARLA_API AInertialMeasurementUnit : public ASensor
{
  GENERATED_BODY()

public:
  // 构造函数，使用 FObjectInitializer 初始化对象
  AInertialMeasurementUnit(const FObjectInitializer &ObjectInitializer);
  // 获取传感器的定义，通常用于创建该类型的新实例
  static FActorDefinition GetSensorDefinition();
  // 重写基类的设置函数，根据演员描述来设置传感器
  void Set(const FActorDescription &ActorDescription) override;
  // 重写基类的设置所有者函数，设置传感器的所有者
  void SetOwner(AActor *Owner) override;
  // 在物理更新后调用，进行后续的物理相关处理
  virtual void PostPhysTick(UWorld *World, ELevelTick TickType, float DeltaTime) override;
  // 计算加速度计噪声，根据输入的加速度向量
  const carla::geom::Vector3D ComputeAccelerometerNoise(
      const FVector &Accelerometer);
   // 计算陀螺仪噪声，根据输入的陀螺仪向量
  const carla::geom::Vector3D ComputeGyroscopeNoise(
      const FVector &Gyroscope);

// 计算加速度计的值，输入是时间间隔，结果以 m/s² 为单位  carla::geom::Vector3D ComputeAccelerometer(const float DeltaTime);

  // 计算陀螺仪的值，结果以 rad/sec 为单位
  carla::geom::Vector3D ComputeGyroscope();

  // 计算罗盘的值，结果是相对于北方的角度，以 rad 为单位
  float ComputeCompass();
  // 设置加速度的标准偏差
  void SetAccelerationStandardDeviation(const FVector &Vec);
  // 设置陀螺仪的标准偏差
  void SetGyroscopeStandardDeviation(const FVector &Vec);
  // 设置陀螺仪的偏差
  void SetGyroscopeBias(const FVector &Vec);
  // 获取加速度的标准偏差，返回一个常引用
  const FVector &GetAccelerationStandardDeviation() const;
  // 获取陀螺仪的标准偏差，返回一个常引用
  const FVector &GetGyroscopeStandardDeviation() const;
  // 获取陀螺仪的偏差，返回一个常引用
  const FVector &GetGyroscopeBias() const;

  // 基于 OpenDRIVE 的经纬度定义北方的向量，在 (0.0f, -1.0f, 0.0f) 方向
  static const FVector CarlaNorthVector;

private:
// 在游戏开始时调用，可用于初始化一些变量或执行一些操作
  void BeginPlay() override;

    // 加速度的标准偏差
  FVector StdDevAccel;

    // 加速度的标准偏差
  FVector StdDevGyro;

// 陀螺仪的偏差
  FVector BiasGyro;

  // 用于计算加速度的数组，存储上两个位置信息
  std::array<FVector, 2> PrevLocation;

 // 用于计算加速度的上一个时间间隔
  float PrevDeltaTime;

};
