// Copyright (c) 2021 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Actor/ActorInfo.h" // Actor 信息头文件
#include "Math/DVector.h" // 数学向量库
#include "Carla/Vehicle/AckermannControllerSettings.h" // Ackermann 控制器设置
#include "Carla/Vehicle/VehicleAckermannControl.h" // Ackermann 控制
#include "Carla/Vehicle/VehicleControl.h" // 车辆控制
#include "Carla/Vehicle/VehicleLightState.h" // 车辆灯光状态
#include "Vehicle/VehicleInputPriority.h" // 车辆输入优先级
#include "Vehicle/VehiclePhysicsControl.h" // 车辆物理控制
#include "Carla/Sensor/DataStream.h" // 数据流类
#include "Carla/Traffic/TrafficLightState.h" // 交通信号灯状态

#include <compiler/disable-ue4-macros.h> // 关闭 UE4 宏的头文件
#include <carla/rpc/WalkerControl.h> // 行人控制类
#include <compiler/enable-ue4-macros.h> // 启用 UE4 宏的头文件

class UCarlaEpisode; // 声明 CARLA Episode 类
class UTrafficLightController; // 声明交通灯控制器类
class FCarlaActor; // 声明 CARLA Actor 类

class FActorData
{
public:

  FDVector Location; // 位置向量

  FQuat Rotation; // 旋转四元数

  FVector Scale; // 缩放向量

  FVector Velocity; // 速度向量

  FVector AngularVelocity = FVector(0,0,0); // 角速度，默认为 (0,0,0)

  bool bSimulatePhysics = false; // 是否模拟物理，默认为 false

  virtual void RecordActorData(FCarlaActor* CarlaActor, UCarlaEpisode* CarlaEpisode); // 记录 Actor 数据

  virtual void RestoreActorData(FCarlaActor* CarlaActor, UCarlaEpisode* CarlaEpisode); // 恢复 Actor 数据

  virtual AActor* RespawnActor(UCarlaEpisode* CarlaEpisode, const FActorInfo& Info); // 重新生成 Actor

  FTransform GetLocalTransform(UCarlaEpisode* CarlaEpisode) const; // 获取局部变换

  virtual ~FActorData(){}; // 虚析构函数
};

class FVehicleData : public FActorData
{
public:

  FVehiclePhysicsControl PhysicsControl; // 车辆物理控制数据

  FVehicleControl Control; // 车辆控制数据

  FVehicleAckermannControl AckermannControl; // Ackermann 控制数据

  bool bAckermannControlActive = false; // Ackermann 控制是否激活

  FAckermannControllerSettings AckermannControllerSettings; // Ackermann 控制器设置

  FVehicleLightState LightState; // 车辆灯光状态

  float SpeedLimit = 30; // 车辆速度限制，默认为 30

  carla::rpc::VehicleFailureState FailureState; // 车辆故障状态

  virtual void RecordActorData(FCarlaActor* CarlaActor, UCarlaEpisode* CarlaEpisode) override; // 记录车辆数据

  virtual void RestoreActorData(FCarlaActor* CarlaActor, UCarlaEpisode* CarlaEpisode) override; // 恢复车辆数据
};

class FWalkerData : public FActorData
{
public:

  carla::rpc::WalkerControl WalkerControl; // 行人控制数据

  bool bAlive = true; // 行人是否存活，默认为 true

  virtual void RecordActorData(FCarlaActor* CarlaActor, UCarlaEpisode* CarlaEpisode) override; // 记录行人数据

  virtual void RestoreActorData(FCarlaActor* CarlaActor, UCarlaEpisode* CarlaEpisode) override; // 恢复行人数据
};

class FTrafficSignData : public FActorData
{
public:
  FString SignId; // 交通标志 ID

  TSubclassOf<AActor> Model; // 标志模型

  TSubclassOf<UObject> SignModel; // 标志对象模型

  virtual AActor* RespawnActor(UCarlaEpisode* CarlaEpisode, const FActorInfo& Info) override; // 重新生成交通标志

  virtual void RecordActorData(FCarlaActor* CarlaActor, UCarlaEpisode* CarlaEpisode) override; // 记录交通标志数据

  virtual void RestoreActorData(FCarlaActor* CarlaActor, UCarlaEpisode* CarlaEpisode) override; // 恢复交通标志数据
};

class FTrafficLightData : public FActorData
{
public:

  UTrafficLightController* Controller; // 交通信号灯控制器

  ETrafficLightState LightState; // 信号灯状态

  FString SignId; // 信号灯 ID

  TSubclassOf<AActor> Model; // 信号灯模型

  int PoleIndex; // 信号灯的杆子索引

  virtual AActor* RespawnActor(UCarlaEpisode* CarlaEpisode, const FActorInfo& Info) override; // 重新生成信号灯

  virtual void RecordActorData(FCarlaActor* CarlaActor, UCarlaEpisode* CarlaEpisode) override; // 记录信号灯数据

  virtual void RestoreActorData(FCarlaActor* CarlaActor, UCarlaEpisode* CarlaEpisode) override; // 恢复信号灯数据
};

class FActorSensorData : public FActorData
{
public:

  FDataStream Stream; // 传感器数据流

  virtual void RecordActorData(FCarlaActor* CarlaActor, UCarlaEpisode* CarlaEpisode) override; // 记录传感器数据

  virtual void RestoreActorData(FCarlaActor* CarlaActor, UCarlaEpisode* CarlaEpisode) override; // 恢复传感器数据
};
sode) override;

  virtual void RestoreActorData(FCarlaActor* CarlaActor, UCarlaEpisode* CarlaEpisode) override;
};
