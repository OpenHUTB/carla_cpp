// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "WheeledVehicle.h"

#include "Vehicle/AckermannController.h"
#include "Vehicle/AckermannControllerSettings.h"
#include "Vehicle/CarlaWheeledVehicleState.h"
#include "Vehicle/VehicleAckermannControl.h"
#include "Vehicle/VehicleControl.h"
#include "Vehicle/VehicleLightState.h"
#include "Vehicle/VehicleInputPriority.h"
#include "Vehicle/VehiclePhysicsControl.h"
#include "Vehicle/VehicleTelemetryData.h"
#include "VehicleVelocityControl.h"
#include "WheeledVehicleMovementComponent4W.h"
#include "WheeledVehicleMovementComponentNW.h"
#include "VehicleAnimInstance.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "MovementComponents/BaseCarlaMovementComponent.h"


#include "FoliageInstancedStaticMeshComponent.h"
#include "CoreMinimal.h"

//-----CARSIM--------------------------------
#ifdef WITH_CARSIM
#include "CarSimMovementComponent.h"
#endif
//-------------------------------------------

#include <utility>

#include "carla/rpc/VehicleFailureState.h"
#include "CarlaWheeledVehicle.generated.h"

class UBoxComponent;

UENUM()
enum class EVehicleWheelLocation : uint8 {

  FL_Wheel = 0,
  FR_Wheel = 1,
  BL_Wheel = 2,
  BR_Wheel = 3,
  ML_Wheel = 4,
  MR_Wheel = 5,
  // 用于自行车和摩托车
  Front_Wheel = 0,
  Back_Wheel = 1,
};

/// 要打开/关闭的门的类型
// 添加新的门类型时，确保所有类型都是最后一种
UENUM(BlueprintType)
enum class EVehicleDoor : uint8 {
  FL = 0,
  FR = 1,
  RL = 2,
  RR = 3,
  Hood = 4,
  Trunk = 5,
  All = 6
};

/// CARLA轮式车辆的基类
UCLASS()
class CARLA_API ACarlaWheeledVehicle : public AWheeledVehicle
{
  GENERATED_BODY()

  // ===========================================================================
  /// @name Constructor and destructor
  // ===========================================================================
  /// @{
public:

  ACarlaWheeledVehicle(const FObjectInitializer &ObjectInitializer);

  ~ACarlaWheeledVehicle();

  /// @}
  // ===========================================================================
  /// @name Get functions
  // ===========================================================================
  /// @{
public:

  /// 当前应用于此车辆的车辆控制
  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  const FVehicleControl &GetVehicleControl() const
  {
    return LastAppliedControl;
  }

  /// 车辆阿克曼控制目前应用于该车辆
  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  const FVehicleAckermannControl &GetVehicleAckermannControl() const
  {
    return LastAppliedAckermannControl;
  }

  /// 车辆的变换。位置被移动以匹配车辆边界的中心，而不是 actor 的位置
  /// vehicle bounds rather than the actor's location.
  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  FTransform GetVehicleTransform() const
  {
    return GetActorTransform();
  }

  /// 前进速度，单位为cm/s。如果倒车可能为负值。
  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  float GetVehicleForwardSpeed() const;

  /// 车辆的方向向量，指向前方
  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  FVector GetVehicleOrientation() const;

  /// 车辆的当前档位
  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  int32 GetVehicleCurrentGear() const;

  /// 车辆边界框相对于车辆的变换
  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  FTransform GetVehicleBoundingBoxTransform() const;

  /// 车辆边界框的范围
  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  FVector GetVehicleBoundingBoxExtent() const;

  /// 获取车辆的边界框组件
  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  UBoxComponent *GetVehicleBoundingBox() const
  {
    return VehicleBounds;
  }

  /// 获取前轮可以转向的最大角度
  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  float GetMaximumSteerAngle() const;

  /// @}
  // ===========================================================================
  /// @name AI debug state
  // ===========================================================================
  /// @{
public:

  /// @todo 这个函数应该是 AWheeledVehicleAIController 的私有函数
  void SetAIVehicleState(ECarlaWheeledVehicleState InState)
  {
    State = InState;
  }

  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  ECarlaWheeledVehicleState GetAIVehicleState() const
  {
    return State;
  }

  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  FVehiclePhysicsControl GetVehiclePhysicsControl() const;

  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  FAckermannControllerSettings GetAckermannControllerSettings() const {
    return AckermannController.GetSettings();
  }

  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  void RestoreVehiclePhysicsControl();

  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  FVehicleLightState GetVehicleLightState() const;

  void ApplyVehiclePhysicsControl(const FVehiclePhysicsControl &PhysicsControl);

  void ApplyAckermannControllerSettings(const FAckermannControllerSettings &AckermannControllerSettings) {
    return AckermannController.ApplySettings(AckermannControllerSettings);
  }

  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  void SetSimulatePhysics(bool enabled);

  void SetWheelCollision(UWheeledVehicleMovementComponent4W *Vehicle4W, const FVehiclePhysicsControl &PhysicsControl);

  void SetWheelCollisionNW(UWheeledVehicleMovementComponentNW *VehicleNW, const FVehiclePhysicsControl &PhysicsControl);

  void SetVehicleLightState(const FVehicleLightState &LightState);

  void SetFailureState(const carla::rpc::VehicleFailureState &FailureState);

  UFUNCTION(BlueprintNativeEvent)
  bool IsTwoWheeledVehicle();
  virtual bool IsTwoWheeledVehicle_Implementation() {
    return false;
  }

  /// @}
  // ===========================================================================
  /// @name Vehicle input control
  // ===========================================================================
  /// @{
public:

  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  void ApplyVehicleControl(const FVehicleControl &Control, EVehicleInputPriority Priority)
  {
    if (bAckermannControlActive) {
      AckermannController.Reset();
    }
    bAckermannControlActive = false;

    if (InputControl.Priority <= Priority)
    {
      InputControl.Control = Control;
      InputControl.Priority = Priority;
    }
  }

  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  void ApplyVehicleAckermannControl(const FVehicleAckermannControl &AckermannControl, EVehicleInputPriority Priority)
  {
    bAckermannControlActive = true;
    LastAppliedAckermannControl = AckermannControl;
    AckermannController.SetTargetPoint(AckermannControl);
  }

  bool IsAckermannControlActive() const
  {
    return bAckermannControlActive;
  }

  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  void ActivateVelocityControl(const FVector &Velocity);

  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  void DeactivateVelocityControl();

  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  FVehicleTelemetryData GetVehicleTelemetryData() const;

  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  void ShowDebugTelemetry(bool Enabled);

  /// @todo 这个函数应该是 AWheeledVehicleAIController 的私有函数
  void FlushVehicleControl();

  /// @}
  // ===========================================================================
  /// @name DEPRECATED Set functions
  // ===========================================================================
  /// @{
public:

  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  void SetThrottleInput(float Value);

  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  void SetSteeringInput(float Value);

  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  void SetBrakeInput(float Value);

  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  void SetReverse(bool Value);

  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  void ToggleReverse()
  {
    SetReverse(!LastAppliedControl.bReverse);
  }

  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  void SetHandbrakeInput(bool Value);

  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  void HoldHandbrake()
  {
    SetHandbrakeInput(true);
  }

  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  void ReleaseHandbrake()
  {
    SetHandbrakeInput(false);
  }

  TArray<float> GetWheelsFrictionScale();

  void SetWheelsFrictionScale(TArray<float> &WheelsFrictionScale);

  void SetCarlaMovementComponent(UBaseCarlaMovementComponent* MoementComponent);

  template<typename T = UBaseCarlaMovementComponent>
  T* GetCarlaMovementComponent() const
  {
    return Cast<T>(BaseMovementComponent);
  }

  /// @}
  // ===========================================================================
  /// @name Overriden from AActor
  // ===========================================================================
  /// @{

protected:

  virtual void BeginPlay() override;
  virtual void EndPlay(const EEndPlayReason::Type EndPlayReason);

  UFUNCTION(BlueprintImplementableEvent)
  void RefreshLightState(const FVehicleLightState &VehicleLightState);

  UFUNCTION(BlueprintCallable, CallInEditor)
  void AdjustVehicleBounds();

  UPROPERTY(Category="Door Animation", EditAnywhere, BlueprintReadWrite)
  TArray<FName> ConstraintComponentNames;

  UPROPERTY(Category="Door Animation", EditAnywhere, BlueprintReadWrite)
  float DoorOpenStrength = 100.0f;

  UFUNCTION(BlueprintCallable, CallInEditor)
  void ResetConstraints();

private:

  /// 车辆控制器的当前状态（用于调试目的）
  UPROPERTY(Category = "AI Controller", VisibleAnywhere)
  ECarlaWheeledVehicleState State = ECarlaWheeledVehicleState::UNKNOWN;

  UPROPERTY(Category = "CARLA Wheeled Vehicle", EditAnywhere)
  UVehicleVelocityControl* VelocityControl;

  struct
  {
    EVehicleInputPriority Priority = EVehicleInputPriority::INVALID;
    FVehicleControl Control;
    FVehicleLightState LightState;
  }
  InputControl;

  FVehicleControl LastAppliedControl;
  FVehicleAckermannControl LastAppliedAckermannControl;
  FVehiclePhysicsControl LastPhysicsControl;

  bool bAckermannControlActive = false;
  FAckermannController AckermannController;

  float RolloverBehaviorForce = 0.35;
  int RolloverBehaviorTracker = 0;
  float RolloverFlagTime = 5.0f;

  carla::rpc::VehicleFailureState FailureState = carla::rpc::VehicleFailureState::None;

public:
  UPROPERTY(Category = "CARLA Wheeled Vehicle", EditDefaultsOnly)
  float DetectionSize { 750.0f };

  UPROPERTY(Category = "CARLA Wheeled Vehicle", VisibleAnywhere, BlueprintReadOnly)
  FBox FoliageBoundingBox;

  UPROPERTY(Category = "CARLA Wheeled Vehicle", EditAnywhere)
  UBoxComponent *VehicleBounds;

  UFUNCTION()
  FBox GetDetectionBox() const;

  UFUNCTION()
  float GetDetectionSize() const;

  UFUNCTION()
  void UpdateDetectionBox();

  UFUNCTION()
  const TArray<int32> GetFoliageInstancesCloseToVehicle(const UInstancedStaticMeshComponent* Component) const;

  UFUNCTION(BlueprintCallable)
  void DrawFoliageBoundingBox() const;

  UFUNCTION()
  FBoxSphereBounds GetBoxSphereBounds() const;

  UFUNCTION()
  bool IsInVehicleRange(const FVector& Location) const;

  /// 设置用户指定的车轮旋转
  /// 0 = FL_VehicleWheel, 1 = FR_VehicleWheel, 2 = BL_VehicleWheel, 3 = BR_VehicleWheel
  /// 注意：这仅仅是视觉效果。它不会以任何方式修改车辆的物理特性
  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  void SetWheelSteerDirection(EVehicleWheelLocation WheelLocation, float AngleInDeg);

  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  float GetWheelSteerAngle(EVehicleWheelLocation WheelLocation);

  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  void OpenDoor(const EVehicleDoor DoorIdx);

  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  void CloseDoor(const EVehicleDoor DoorIdx);

  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  void OpenDoorPhys(const EVehicleDoor DoorIdx);

  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  void CloseDoorPhys(const EVehicleDoor DoorIdx);

  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  void RecordDoorChange(const EVehicleDoor DoorIdx, const bool bIsOpen); 

  virtual FVector GetVelocity() const override;

//-----CARSIM--------------------------------
  UPROPERTY(Category="CARLA Wheeled Vehicle", EditAnywhere)
  float CarSimOriginOffset = 150.f;
//-------------------------------------------

  UPROPERTY(Category="CARLA Wheeled Vehicle", VisibleAnywhere)
  bool bIsNWVehicle = false;

  void SetRolloverFlag();

  carla::rpc::VehicleFailureState GetFailureState() const;

  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  static FRotator GetPhysicsConstraintAngle(UPhysicsConstraintComponent* Component);
  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  static void SetPhysicsConstraintAngle(
      UPhysicsConstraintComponent*Component, const FRotator &NewAngle);
 
private:

  UPROPERTY(Category="CARLA Wheeled Vehicle", VisibleAnywhere)
  bool bPhysicsEnabled = true;

  // 小技巧，允许可选的 CarSim 插件使用
  UPROPERTY(Category="CARLA Wheeled Vehicle", VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
  UBaseCarlaMovementComponent * BaseMovementComponent = nullptr;

  UPROPERTY(Category="CARLA Wheeled Vehicle", VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
  TArray<UPhysicsConstraintComponent*> ConstraintsComponents;

  UPROPERTY(Category="CARLA Wheeled Vehicle", VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
  TMap<UPhysicsConstraintComponent*, UPrimitiveComponent*> ConstraintDoor;

  // 车门初始变换的容器，用于重置其位置
  UPROPERTY(Category="CARLA Wheeled Vehicle", VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
  TMap<UPrimitiveComponent*, FTransform> DoorComponentsTransform;

  UPROPERTY(Category="CARLA Wheeled Vehicle", VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
  TMap<UPrimitiveComponent*, UPhysicsConstraintComponent*> CollisionDisableConstraints;

  /// 翻车时通常会有过大的角速度，导致车辆完成360度翻转
  /// 这个函数逐渐减小车辆的角速度，使其最终倒置而不是完全翻转
  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  void ApplyRolloverBehavior();

  void CheckRollover(const float roll, const std::pair<float, float> threshold_roll);

  void AddReferenceToManager();
  void RemoveReferenceToManager();


  FTimerHandle TimerHandler;
public:
  float SpeedAnim { 0.0f };
  float RotationAnim { 0.0f };

  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  float GetSpeedAnim() const { return SpeedAnim; }

  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  void SetSpeedAnim(float Speed) { SpeedAnim = Speed; }

  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  float GetRotationAnim() const { return RotationAnim; }

  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  void SetRotationAnim(float Rotation) { RotationAnim = Rotation; }
};
