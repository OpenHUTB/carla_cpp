// Copyright (c) 2021 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// 该作品根据MIT许可证进行授权。
// 有关许可证的副本，请参阅<https://opensource.org/licenses/MIT>。

#pragma once

#include "Actor/ActorInfo.h"
#include "Math/DVector.h"
#include "Carla/Vehicle/AckermannControllerSettings.h"
#include "Carla/Vehicle/VehicleAckermannControl.h"
#include "Carla/Vehicle/VehicleControl.h"
#include "Carla/Vehicle/VehicleLightState.h"
#include "Vehicle/VehicleInputPriority.h"
#include "Vehicle/VehiclePhysicsControl.h"
#include "Carla/Sensor/DataStream.h"
#include "Carla/Traffic/TrafficLightState.h"

#include <compiler/disable-ue4-macros.h>
#include <carla/rpc/WalkerControl.h>
#include <compiler/enable-ue4-macros.h>

class UCarlaEpisode;
class UTrafficLightController;
class FCarlaActor;

// 基础类，存储Actor的通用数据
class FActorData
{
public:

  FDVector Location;  // Actor的位置

  FQuat Rotation;  // Actor的旋转信息

  FVector Scale;  // Actor的缩放信息

  FVector Velocity;  // Actor的速度矢量

  FVector AngularVelocity = FVector(0,0,0);  // Actor的角速度矢量，默认为(0,0,0)

  bool bSimulatePhysics = false;  // 是否启用物理模拟

  // 记录Actor的数据
  virtual void RecordActorData(FCarlaActor* CarlaActor, UCarlaEpisode* CarlaEpisode);

  // 恢复Actor的数据
  virtual void RestoreActorData(FCarlaActor* CarlaActor, UCarlaEpisode* CarlaEpisode);

  // 重新生成Actor
  virtual AActor* RespawnActor(UCarlaEpisode* CarlaEpisode, const FActorInfo& Info);

  // 获取Actor的局部变换信息
  FTransform GetLocalTransform(UCarlaEpisode* CarlaEpisode) const;

  // 虚析构函数
  virtual ~FActorData(){};
};

// 用于存储车辆数据的类
class FVehicleData : public FActorData
{
public:

  FVehiclePhysicsControl PhysicsControl;  // 车辆的物理控制设置

  FVehicleControl Control;  // 车辆的基本控制指令

  FVehicleAckermannControl AckermannControl;  // Ackermann控制指令

  bool bAckermannControlActive = false;  // 是否启用Ackermann控制

  FAckermannControllerSettings AckermannControllerSettings;  // Ackermann控制器设置

  FVehicleLightState LightState;  // 车辆灯光状态

  float SpeedLimit = 30;  // 车辆的速度限制，默认30

  carla::rpc::VehicleFailureState FailureState;  // 车辆的故障状态

  // 重写记录Actor数据的方法
  virtual void RecordActorData(FCarlaActor* CarlaActor, UCarlaEpisode* CarlaEpisode) override;

  // 重写恢复Actor数据的方法
  virtual void RestoreActorData(FCarlaActor* CarlaActor, UCarlaEpisode* CarlaEpisode) override;
};

// 用于存储行人数据的类
class FWalkerData : public FActorData
{
public:

  carla::rpc::WalkerControl WalkerControl;  // 行人的控制指令

  bool bAlive = true;  // 行人是否存活

  // 重写记录Actor数据的方法
  virtual void RecordActorData(FCarlaActor* CarlaActor, UCarlaEpisode* CarlaEpisode) override;

  // 重写恢复Actor数据的方法
  virtual void RestoreActorData(FCarlaActor* CarlaActor, UCarlaEpisode* CarlaEpisode) override;
};

// 用于存储交通标志数据的类
class FTrafficSignData : public FActorData
{
public:
  FString SignId;  // 交通标志的ID

  TSubclassOf<AActor> Model;  // 交通标志的Actor类

  TSubclassOf<UObject> SignModel;  // 交通标志的模型类

  // 重写重新生成Actor的方法
  virtual AActor* RespawnActor(UCarlaEpisode* CarlaEpisode, const FActorInfo& Info) override;

  // 重写记录Actor数据的方法
  virtual void RecordActorData(FCarlaActor* CarlaActor, UCarlaEpisode* CarlaEpisode) override;

  // 重写恢复Actor数据的方法
  virtual void RestoreActorData(FCarlaActor* CarlaActor, UCarlaEpisode* CarlaEpisode) override;
};

// 用于存储交通灯数据的类
class FTrafficLightData : public FActorData
{
public:

  UTrafficLightController* Controller;  // 交通灯控制器

  ETrafficLightState LightState;  // 交通灯状态

  FString SignId;  // 交通灯的ID

  TSubclassOf<AActor> Model;  // 交通灯的Actor类

  int PoleIndex;  // 交通灯的灯杆索引

  // 重写重新生成Actor的方法
  virtual AActor* RespawnActor(UCarlaEpisode* CarlaEpisode, const FActorInfo& Info) override;

  // 重写记录Actor数据的方法
  virtual void RecordActorData(FCarlaActor* CarlaActor, UCarlaEpisode* CarlaEpisode) override;

  // 重写恢复Actor数据的方法
  virtual void RestoreActorData(FCarlaActor* CarlaActor, UCarlaEpisode* CarlaEpisode) override;

};

// 用于存储传感器数据的类
class FActorSensorData : public FActorData
{
public:

  FDataStream Stream;  // 传感器数据流

  // 重写记录Actor数据的方法
  virtual void RecordActorData(FCarlaActor* CarlaActor, UCarlaEpisode* CarlaEpisode) override;

  // 重写恢复Actor数据的方法
  virtual void RestoreActorData(FCarlaActor* CarlaActor, UCarlaEpisode* CarlaEpisode) override;
};
