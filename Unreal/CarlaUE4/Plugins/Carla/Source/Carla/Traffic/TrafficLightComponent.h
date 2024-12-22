// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "CoreMinimal.h"
#include "SignComponent.h"
#include "TrafficLightState.h"
#include "Carla/Vehicle/WheeledVehicleAIController.h"
#include "TrafficLightComponent.generated.h"

class ATrafficLightManager;
class ATrafficLightGroup;
class UTrafficLightController;

// 委托定义调度程序：declare_dynamic_multicast_delegate
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FLightChangeDispatcher);

/// 表示 OpenDRIVE 交通信号灯的类
UCLASS(Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CARLA_API UTrafficLightComponent : public USignComponent
{
  GENERATED_BODY()

public:
  // 设置此组件属性的默认值
  UTrafficLightComponent();

  // 通过 UFUNCTION宏 来注册函数到蓝图中；
  // BlueprintCallable 表示这个函数可以被蓝图调用；
  // 这个成员函数由其蓝图的子类实现，你不应该尝试在C++中给出函数的实现，这会导致链接错误。
  // 设置交通灯的状态
  UFUNCTION(Category = "Traffic Light", BlueprintCallable)
  void SetLightState(ETrafficLightState NewState);

  // 获得交通灯的状态
  UFUNCTION(Category = "Traffic Light", BlueprintCallable)
  ETrafficLightState GetLightState() const;

  // 修改冻结的交通信号灯
  UFUNCTION(Category = "Traffic Light", BlueprintCallable)
  void SetFrozenGroup(bool InFreeze);

  // 获得交通灯组（列表）
  UFUNCTION(Category = "Traffic Light", BlueprintPure)
  ATrafficLightGroup* GetGroup();

  const ATrafficLightGroup* GetGroup() const;

  void SetController(UTrafficLightController* Controller);

  UFUNCTION(Category = "Traffic Light", BlueprintPure)
  UTrafficLightController* GetController();

  const UTrafficLightController* GetController() const;

  virtual void InitializeSign(const carla::road::Map &Map) override;

protected:

  UFUNCTION(BlueprintCallable)
  void OnBeginOverlapTriggerBox(UPrimitiveComponent *OverlappedComp,
      AActor *OtherActor,
      UPrimitiveComponent *OtherComp,
      int32 OtherBodyIndex,
      bool bFromSweep,
      const FHitResult &SweepResult);

  UFUNCTION(BlueprintCallable)
  void OnEndOverlapTriggerBox(UPrimitiveComponent *OverlappedComp,
      AActor *OtherActor,
      UPrimitiveComponent *OtherComp,
      int32 OtherBodyIndex);



private:

  friend ATrafficLightManager;

  void GenerateTrafficLightBox(
      const FTransform BoxTransform,
      const FVector BoxSize);

  // 借助 UPROPERTY 宏 将一个 UObject 类的子类的成员变量注册到蓝图中（让蓝图能够调用这个C++类中的函数）
  // 可以传递更多参数来控制 UPROPERTY 宏的行为
  UPROPERTY(Category = "Traffic Light", EditAnywhere)
  ETrafficLightState LightState;

  UPROPERTY(Category = "Traffic Light", BlueprintAssignable)
  FLightChangeDispatcher LightChangeDispatcher;

  // UPROPERTY(Category = "Traffic Light", VisibleAnywhere)
  // ATrafficLightGroup *TrafficLightGroup = nullptr;

  UPROPERTY(Category = "Traffic Light", VisibleAnywhere)
  UTrafficLightController *TrafficLightController = nullptr;

  // 进入交通信号灯触发框的车辆
  UPROPERTY()
  TArray<AWheeledVehicleAIController*> Vehicles;

};
