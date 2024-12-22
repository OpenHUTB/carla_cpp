// Copyright (c) 2021 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once // 指示编译器此头文件被包含一次，防止重复包含

#include "Actor/ActorInfo.h" // 包含Actor信息相关的声明和定义
#include "Math/DVector.h" // 包含数学向量库，用于表示向量和相关操作
#include "Carla/Vehicle/AckermannControllerSettings.h" // 包含Ackermann控制器设置的相关声明和定义
#include "Carla/Vehicle/VehicleAckermannControl.h" // 包含Ackermann控制相关的声明和定义
#include "Carla/Vehicle/VehicleControl.h" // 包含车辆控制相关的声明和定义
#include "Carla/Vehicle/VehicleLightState.h" // 包含车辆灯光状态的相关声明和定义
#include "Vehicle/VehicleInputPriority.h" // 包含车辆输入优先级的相关声明和定义
#include "Vehicle/VehiclePhysicsControl.h" // 包含车辆物理控制相关的声明和定义
#include "Carla/Sensor/DataStream.h" // 包含数据流类的相关声明和定义
#include "Carla/Traffic/TrafficLightState.h" // 包含交通信号灯状态的相关声明和定义

#include <compiler/disable-ue4-macros.h> // 包含关闭UE4宏的头文件，防止宏冲突
#include <carla/rpc/WalkerControl.h> // 包含行人控制类的相关声明和定义
#include <compiler/enable-ue4-macros.h> // 包含启用UE4宏的头文件，恢复宏定义

class UCarlaEpisode; // 前向声明CARLA Episode类，用于表示仿真会话
class UTrafficLightController; // 前向声明交通灯控制器类，用于控制交通灯
class FCarlaActor; // 前向声明CARLA Actor类，用于表示仿真中的Actor

class FActorData
{
public:
  FDVector Location; // 存储Actor的位置向量

  FQuat Rotation; // 存储Actor的旋转四元数

  FVector Scale; // 存储Actor的缩放向量

  FVector Velocity; // 存储Actor的速度向量

  FVector AngularVelocity = FVector(0,0,0); // 存储Actor的角速度，默认值为(0,0,0)

  bool bSimulatePhysics = false; // 指示是否模拟物理行为，默认为false

  virtual void RecordActorData(FCarlaActor* CarlaActor, UCarlaEpisode* CarlaEpisode); // 虚函数，用于记录Actor数据
  virtual void RestoreActorData(FCarlaActor* CarlaActor, UCarlaEpisode* CarlaEpisode); // 虚函数，用于恢复Actor数据
  virtual AActor* RespawnActor(UCarlaEpisode* CarlaEpisode, const FActorInfo& Info); // 虚函数，用于重新生成Actor
  FTransform GetLocalTransform(UCarlaEpisode* CarlaEpisode) const; // 函数，用于获取Actor的局部变换
  virtual ~FActorData(){}; // 虚析构函数，确保派生类能正确析构
};

class FVehicleData : public FActorData
{
public:
  FVehiclePhysicsControl PhysicsControl; // 存储车辆物理控制数据
  FVehicleControl Control; // 存储车辆控制数据
  FVehicleAckermannControl AckermannControl; // 存储Ackermann控制数据
};


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
