// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once // 指示此头文件被包含一次，防止重复包含

#include "Carla/Actor/ActorInfo.h" // 包含CARLA中Actor信息的头文件
#include "Carla/Actor/ActorData.h" // 包含CARLA中Actor数据的头文件
#include "Carla/Vehicle/CarlaWheeledVehicle.h" // 包含CARLA中轮式车辆的头文件
#include "Carla/Vehicle/VehicleTelemetryData.h" // 包含车辆遥测数据的头文件
#include "Carla/Walker/WalkerController.h" // 包含行人控制器的头文件
#include "Carla/Traffic/TrafficLightState.h" // 包含交通灯状态的头文件
#include "carla/rpc/ActorState.h" // 包含RPC中Actor状态的头文件
#include "carla/rpc/AttachmentType.h" // 包含RPC中附件类型的头文件
#include "Carla/Server/CarlaServerResponse.h" // 包含CARLA服务器响应的头文件

class AActor; // 前向声明Unreal Engine中的Actor类

/// 查看一个参与者和它的属性
class FCarlaActor
{
public:
  using IdType = uint32; // 定义Actor ID的类型为无符号32位整数
  enum class ActorType : uint8 // 定义Actor类型的枚举
  {
    Other, // 其他类型
    Vehicle, // 车辆类型
    Walker, // 行人类型
    TrafficLight, // 交通灯类型
    TrafficSign, // 交通标志类型
    Sensor, // 传感器类型
    INVALID // 无效类型
  };
  FCarlaActor() = default; // 默认构造函数
  // FCarlaActor(const FCarlaActor &) = default; // 复制构造函数（被注释）
  // FCarlaActor(FCarlaActor &&) = default; // 移动构造函数（被注释）
  FCarlaActor( // 构造函数
      IdType ActorId,
      AActor* Actor,
      TSharedPtr<const FActorInfo> Info,
      carla::rpc::ActorState InState,
      UWorld* World);
  virtual ~FCarlaActor() {} // 虚析构函数
  bool IsInValid() const // 检查Actor是否无效
  {
    return (carla::rpc::ActorState::Invalid == State);
  }
  bool IsAlive() const // 检查Actor是否存活（非PendingKill和非Invalid）
  {
    return (carla::rpc::ActorState::PendingKill != State &&
            carla::rpc::ActorState::Invalid != State);
  }
  bool IsActive() const // 检查Actor是否处于活跃状态
  {
    return (carla::rpc::ActorState::Active == State);
  }
  bool IsDormant() const // 检查Actor是否处于休眠状态
  {
    return (carla::rpc::ActorState::Dormant == State);
  }
  bool IsPendingKill() const // 检查Actor是否待销毁
  {
    return (carla::rpc::ActorState::PendingKill == State);
  }
  IdType GetActorId() const // 获取Actor ID
  {
    return Id;
  }
  ActorType GetActorType() const // 获取Actor类型
  {
    return Type;
  }
  AActor *GetActor() // 获取Actor指针（非const版本）
  {
    return TheActor;
  }
  const AActor *GetActor() const // 获取Actor指针（const版本）
  {
    return TheActor;
  }
  const FActorInfo *GetActorInfo() const // 获取Actor信息指针
  {
    return Info.Get();
  }
  carla::rpc::ActorState GetActorState() const // 获取Actor状态
  {
    return State;
  }
  void SetActorState(carla::rpc::ActorState InState) // 设置Actor状态
  {
    State = InState;
  }
  void SetParent(IdType InParentId) // 设置父Actor ID
  {
    ParentId = InParentId;
  }
  IdType GetParent() const // 获取父Actor ID
  {
    return ParentId;
  }
  void AddChildren(IdType ChildId) // 添加子Actor ID

  {
    Children.Add(ChildId);
  }
  void RemoveChildren(IdType ChildId)
  {
    Children.Remove(ChildId);
  }
  const TArray<IdType>& GetChildren() const
  {
    return Children;
  }
  void SetAttachmentType(carla::rpc::AttachmentType InAttachmentType)
  {
    Attachment = InAttachmentType;
  }
  carla::rpc::AttachmentType GetAttachmentType() const
  {
    return Attachment;
  }
  void BuildActorData();
  void PutActorToSleep(UCarlaEpisode* CarlaEpisode);
  void WakeActorUp(UCarlaEpisode* CarlaEpisode);
  FActorData* GetActorData()
  {
    return ActorData.Get();
  }
  const FActorData* GetActorData() const
  {
    return ActorData.Get();
  }
  template<typename T>
  T* GetActorData()
  {
    return dynamic_cast<T*>(ActorData.Get());
  }
  template<typename T>
  const T* GetActorData() const
  {
    return dynamic_cast<T*>(ActorData.Get());
  }
  // 参与者功能接口 ----------------------
  // 通用功能
  FTransform GetActorLocalTransform() const;
  FTransform GetActorGlobalTransform() const;
  FVector GetActorLocalLocation() const;
  FVector GetActorGlobalLocation() const;
  void SetActorLocalLocation(
      const FVector& Location,
      ETeleportType Teleport = ETeleportType::TeleportPhysics);

  void SetActorGlobalLocation(
      const FVector& Location,
      ETeleportType Teleport = ETeleportType::TeleportPhysics);

  void SetActorLocalTransform(
      const FTransform& Transform,
      ETeleportType Teleport = ETeleportType::TeleportPhysics);

  void SetActorGlobalTransform(
      const FTransform& Transform,
      ETeleportType Teleport = ETeleportType::TeleportPhysics);

  FVector GetActorVelocity() const;

  FVector GetActorAngularVelocity() const;

  ECarlaServerResponse SetActorTargetVelocity(const FVector& Velocity);

  ECarlaServerResponse SetActorTargetAngularVelocity(const FVector& AngularVelocity);

  ECarlaServerResponse AddActorImpulse(const FVector& Impulse);

  ECarlaServerResponse AddActorImpulseAtLocation(const FVector& Impulse, const FVector& Location);

  ECarlaServerResponse AddActorForce(const FVector& Force);

  ECarlaServerResponse AddActorForceAtLocation(const FVector& Force, const FVector& Location);

  ECarlaServerResponse AddActorAngularImpulse(const FVector& AngularInpulse);

  ECarlaServerResponse AddActorTorque(const FVector& Torque);

  virtual ECarlaServerResponse SetActorSimulatePhysics(bool bEnabled);

  virtual ECarlaServerResponse SetActorCollisions(bool bEnabled);

  virtual ECarlaServerResponse SetActorEnableGravity(bool bEnabled);

  // 车辆功能
  virtual ECarlaServerResponse EnableActorConstantVelocity(const FVector&)
  {
    return ECarlaServerResponse::ActorTypeMismatch;
  }

  virtual ECarlaServerResponse DisableActorConstantVelocity()
  {
    return ECarlaServerResponse::ActorTypeMismatch;
  }

  virtual ECarlaServerResponse GetPhysicsControl(FVehiclePhysicsControl&)
  {
    return ECarlaServerResponse::ActorTypeMismatch;
  }

  virtual ECarlaServerResponse GetFailureState(carla::rpc::VehicleFailureState&)
  {
    return ECarlaServerResponse::ActorTypeMismatch;
  }

  virtual ECarlaServerResponse GetVehicleLightState(FVehicleLightState&)
  {
    return ECarlaServerResponse::ActorTypeMismatch;
  }

  virtual ECarlaServerResponse OpenVehicleDoor(const EVehicleDoor)
  {
    return ECarlaServerResponse::ActorTypeMismatch;
  }

  virtual ECarlaServerResponse CloseVehicleDoor(const EVehicleDoor)
  {
    return ECarlaServerResponse::ActorTypeMismatch;
  }

  virtual ECarlaServerResponse ApplyPhysicsControl(const FVehiclePhysicsControl&)
  {
    return ECarlaServerResponse::ActorTypeMismatch;
  }

  virtual ECarlaServerResponse SetVehicleLightState(const FVehicleLightState&)
  {
    return ECarlaServerResponse::ActorTypeMismatch;
  }

  virtual ECarlaServerResponse SetWheelSteerDirection(const EVehicleWheelLocation&, float)
  {
    return ECarlaServerResponse::ActorTypeMismatch;
  }

  virtual ECarlaServerResponse GetWheelSteerAngle(const EVehicleWheelLocation&, float&)
  {
    return ECarlaServerResponse::ActorTypeMismatch;
  }

  virtual ECarlaServerResponse ApplyControlToVehicle(
      const FVehicleControl&, const EVehicleInputPriority&)
  {
    return ECarlaServerResponse::ActorTypeMismatch;
  }

  virtual ECarlaServerResponse ApplyAckermannControlToVehicle(
      const FVehicleAckermannControl&, const EVehicleInputPriority&)
  {
    return ECarlaServerResponse::ActorTypeMismatch;
  }

  virtual ECarlaServerResponse GetVehicleControl(FVehicleControl&)
  {
    return ECarlaServerResponse::ActorTypeMismatch;
  }

  virtual ECarlaServerResponse GetVehicleAckermannControl(FVehicleAckermannControl&)
  {
    return ECarlaServerResponse::ActorTypeMismatch;
  }

  virtual ECarlaServerResponse GetAckermannControllerSettings(FAckermannControllerSettings&)
  {
    return ECarlaServerResponse::ActorTypeMismatch;
  }

  virtual ECarlaServerResponse ApplyAckermannControllerSettings(const FAckermannControllerSettings&)
  {
    return ECarlaServerResponse::ActorTypeMismatch;
  }

  virtual ECarlaServerResponse SetActorAutopilot(bool, bool bKeepState = false)
  {
    return ECarlaServerResponse::ActorTypeMismatch;
  }

  virtual ECarlaServerResponse GetVehicleTelemetryData(FVehicleTelemetryData&)
  {
    return ECarlaServerResponse::ActorTypeMismatch;
  }

  virtual ECarlaServerResponse ShowVehicleDebugTelemetry(bool)
  {
    return ECarlaServerResponse::ActorTypeMismatch;
  }

  virtual ECarlaServerResponse EnableCarSim(const FString&)
  {
    return ECarlaServerResponse::ActorTypeMismatch;
  }

  virtual ECarlaServerResponse UseCarSimRoad(bool)
  {
    return ECarlaServerResponse::ActorTypeMismatch;
  }

  virtual ECarlaServerResponse EnableChronoPhysics(uint64_t, float,
      const FString&, const FString&, const FString&, const FString&)
  {
    return ECarlaServerResponse::ActorTypeMismatch;
  }

  virtual ECarlaServerResponse RestorePhysXPhysics()
  {
    return ECarlaServerResponse::ActorTypeMismatch;
  }

  // 交通信号灯功能

  virtual ECarlaServerResponse SetTrafficLightState(const ETrafficLightState&)
  {
    return ECarlaServerResponse::ActorTypeMismatch;
  }

  virtual ETrafficLightState GetTrafficLightState() const
  {
    return ETrafficLightState::Off;
  }

  virtual UTrafficLightController* GetTrafficLightController()
  {
    return nullptr;
  }

  virtual ECarlaServerResponse SetLightGreenTime(float)
  {
    return ECarlaServerResponse::ActorTypeMismatch;
  }

  virtual ECarlaServerResponse SetLightYellowTime(float)
  {
    return ECarlaServerResponse::ActorTypeMismatch;
  }

  virtual ECarlaServerResponse SetLightRedTime(float)
  {
    return ECarlaServerResponse::ActorTypeMismatch;
  }

  //交通标志功能

  // Walker函数
  virtual ECarlaServerResponse SetWalkerState(
      const FTransform& Transform,
      carla::rpc::WalkerControl WalkerControl)
  {
    return ECarlaServerResponse::ActorTypeMismatch;
  }

  virtual ECarlaServerResponse ApplyControlToWalker(const FWalkerControl&)
  {
    return ECarlaServerResponse::ActorTypeMismatch;
  }

  virtual ECarlaServerResponse GetWalkerControl(FWalkerControl&)
  {
    return ECarlaServerResponse::ActorTypeMismatch;
  }

  virtual ECarlaServerResponse GetBonesTransform(FWalkerBoneControlOut&)
  {
    return ECarlaServerResponse::ActorTypeMismatch;
  }

  virtual ECarlaServerResponse SetBonesTransform(const FWalkerBoneControlIn&)
  {
    return ECarlaServerResponse::ActorTypeMismatch;
  }

  virtual ECarlaServerResponse BlendPose(float Blend)
  {
    return ECarlaServerResponse::ActorTypeMismatch;
  }

  virtual ECarlaServerResponse GetPoseFromAnimation()
  {
    return ECarlaServerResponse::ActorTypeMismatch;
  }

  virtual ECarlaServerResponse SetActorDead()
  {
    return ECarlaServerResponse::ActorTypeMismatch;
  }

  virtual ECarlaServerResponse FreezeTrafficLight(bool)
  {
    return ECarlaServerResponse::ActorTypeMismatch;
  }

  virtual ECarlaServerResponse ResetTrafficLightGroup()
  {
    return ECarlaServerResponse::ActorTypeMismatch;
  }

  //传感器功能

  static TSharedPtr<FCarlaActor> ConstructCarlaActor(
      IdType ActorId,
      AActor* Actor,
      TSharedPtr<const FActorInfo> Info,
      ActorType Type,
      carla::rpc::ActorState InState,
      UWorld* World);

private:

  friend class FActorRegistry;

  AActor *TheActor = nullptr;

  TSharedPtr<const FActorInfo> Info = nullptr;

  IdType Id = 0u;

  IdType ParentId = 0u;

  carla::rpc::ActorState State = carla::rpc::ActorState::Invalid;

  carla::rpc::AttachmentType Attachment = carla::rpc::AttachmentType::INVALID;

  TArray<IdType> Children;

protected:

  ActorType Type = ActorType::INVALID;

  TSharedPtr<FActorData> ActorData = nullptr;

  UWorld *World = nullptr;

};

class FVehicleActor : public FCarlaActor
{
public:
  FVehicleActor(
      IdType ActorId,
      AActor* Actor,
      TSharedPtr<const FActorInfo> Info,
      carla::rpc::ActorState InState,
      UWorld* World);

  virtual ECarlaServerResponse EnableActorConstantVelocity(const FVector& Velocity) final;

  virtual ECarlaServerResponse DisableActorConstantVelocity() final;

  virtual ECarlaServerResponse GetPhysicsControl(FVehiclePhysicsControl& PhysicsControl) final;

  virtual ECarlaServerResponse GetFailureState(carla::rpc::VehicleFailureState&) final;

  virtual ECarlaServerResponse GetVehicleLightState(FVehicleLightState& LightState) final;

  virtual ECarlaServerResponse OpenVehicleDoor(const EVehicleDoor DoorIdx) final;

  virtual ECarlaServerResponse CloseVehicleDoor(const EVehicleDoor DoorIdx) final;

  virtual ECarlaServerResponse ApplyPhysicsControl(
      const FVehiclePhysicsControl& PhysicsControl) final;

  virtual ECarlaServerResponse SetVehicleLightState(
      const FVehicleLightState& LightState) final;

  virtual ECarlaServerResponse SetWheelSteerDirection(
      const EVehicleWheelLocation& WheelLocation, float AngleInDeg) final;

  virtual ECarlaServerResponse GetWheelSteerAngle(
      const EVehicleWheelLocation& WheelLocation, float& Angle);

  virtual ECarlaServerResponse SetActorSimulatePhysics(bool bSimulatePhysics) final;

  virtual ECarlaServerResponse ApplyControlToVehicle(
      const FVehicleControl&, const EVehicleInputPriority&) final;

  virtual ECarlaServerResponse ApplyAckermannControlToVehicle(
      const FVehicleAckermannControl&, const EVehicleInputPriority&) final;

  virtual ECarlaServerResponse GetVehicleControl(FVehicleControl&) final;

  virtual ECarlaServerResponse GetVehicleAckermannControl(FVehicleAckermannControl&) final;

  virtual ECarlaServerResponse GetAckermannControllerSettings(FAckermannControllerSettings&) final;

  virtual ECarlaServerResponse ApplyAckermannControllerSettings(const FAckermannControllerSettings&) final;

  virtual ECarlaServerResponse SetActorAutopilot(bool bEnabled, bool bKeepState = false) final;

  virtual ECarlaServerResponse GetVehicleTelemetryData(FVehicleTelemetryData&) final;

  virtual ECarlaServerResponse ShowVehicleDebugTelemetry(bool bEnabled) final;

  virtual ECarlaServerResponse EnableCarSim(const FString& SimfilePath) final;

  virtual ECarlaServerResponse UseCarSimRoad(bool bEnabled) final;

  virtual ECarlaServerResponse EnableChronoPhysics(
      uint64_t MaxSubsteps, float MaxSubstepDeltaTime,
      const FString& VehicleJSON, const FString& PowertrainJSON,
      const FString& TireJSON, const FString& BaseJSONPath) final;

  virtual ECarlaServerResponse RestorePhysXPhysics();
};

class FSensorActor : public FCarlaActor
{
public:
  FSensorActor(
      IdType ActorId,
      AActor* Actor,
      TSharedPtr<const FActorInfo> Info,
      carla::rpc::ActorState InState,
      UWorld* World);

};

class FTrafficSignActor : public FCarlaActor
{
public:
  FTrafficSignActor(
      IdType ActorId,
      AActor* Actor,
      TSharedPtr<const FActorInfo> Info,
      carla::rpc::ActorState InState,
      UWorld* World);
};

class FTrafficLightActor : public FCarlaActor
{
public:
  FTrafficLightActor(
      IdType ActorId,
      AActor* Actor,
      TSharedPtr<const FActorInfo> Info,
      carla::rpc::ActorState InState,
      UWorld* World);

  virtual ECarlaServerResponse SetTrafficLightState(const ETrafficLightState& State) final;

  virtual ETrafficLightState GetTrafficLightState() const final;

  virtual UTrafficLightController* GetTrafficLightController() final;

  virtual ECarlaServerResponse SetLightGreenTime(float time) final;

  virtual ECarlaServerResponse SetLightYellowTime(float time) final;

  virtual ECarlaServerResponse SetLightRedTime(float time) final;

  virtual ECarlaServerResponse FreezeTrafficLight(bool bFreeze) final;

  virtual ECarlaServerResponse ResetTrafficLightGroup() final;

};

class FWalkerActor : public FCarlaActor
{
public:
  FWalkerActor(
      IdType ActorId,
      AActor* Actor,
      TSharedPtr<const FActorInfo> Info,
      carla::rpc::ActorState InState,
      UWorld* World);

  virtual ECarlaServerResponse SetWalkerState(
      const FTransform& Transform,
      carla::rpc::WalkerControl WalkerControl) final;

  virtual ECarlaServerResponse SetActorSimulatePhysics(bool bSimulatePhysics) final;

  virtual ECarlaServerResponse SetActorEnableGravity(bool bEnabled) final;

  virtual ECarlaServerResponse ApplyControlToWalker(const FWalkerControl&) final;

  virtual ECarlaServerResponse GetWalkerControl(FWalkerControl&) final;

  virtual ECarlaServerResponse GetBonesTransform(FWalkerBoneControlOut&) final;

  virtual ECarlaServerResponse SetBonesTransform(const FWalkerBoneControlIn&) final;

  virtual ECarlaServerResponse BlendPose(float Blend);

  virtual ECarlaServerResponse GetPoseFromAnimation();

  virtual ECarlaServerResponse SetActorDead();
};

class FOtherActor : public FCarlaActor
{
public:
  FOtherActor(
      IdType ActorId,
      AActor* Actor,
      TSharedPtr<const FActorInfo> Info,
      carla::rpc::ActorState InState,
      UWorld* World);

};
