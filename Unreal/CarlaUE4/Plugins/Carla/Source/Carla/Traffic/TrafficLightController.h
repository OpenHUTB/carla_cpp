// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "CoreMinimal.h"
#include "Object.h"
#include "TrafficLightState.h"
#include "TrafficLightComponent.h"
#include "Containers/Map.h"
#include "Carla/Actor/CarlaActor.h"
#include "TrafficLightController.generated.h"

class ATrafficLightGroup;

/// 定义信号量的某个阶段，并定义其状态以及该状态的持续时间
USTRUCT(BlueprintType)
struct FTrafficLightStage
{
  // 在类体的第一行添加（如果你的类继承自UObject，你的类名上方需要加入UCLASS()宏）
  GENERATED_BODY()

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  float Time;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  ETrafficLightState State;
};

/// 从 OpenDrive 映射一个控制器。控制相关交通信号灯并包含其循环
UCLASS(BlueprintType)
class CARLA_API UTrafficLightController : public UObject
{
  GENERATED_BODY()

public:

  UTrafficLightController();

  UFUNCTION(Category = "Traffic Controller", BlueprintCallable)
  void SetStates(TArray<FTrafficLightStage> States);

  UFUNCTION(Category = "Traffic Controller", BlueprintPure)
  const FTrafficLightStage &GetCurrentState() const;

  // 将交通信号灯组件更新到下一个状态
  UFUNCTION(Category = "Traffic Controller", BlueprintCallable)
  float NextState();

  // 推进控制器的计数器，如果循环完成则返回 true
  UFUNCTION(Category = "Traffic Controller", BlueprintCallable)
  bool AdvanceTimeAndCycleFinished(float DeltaTime);

  UFUNCTION(Category = "Traffic Controller", BlueprintCallable)
  void StartCycle();

  UFUNCTION(Category = "Traffic Controller", BlueprintPure)
  const TArray<UTrafficLightComponent *> &GetTrafficLights();

  UFUNCTION(Category = "Traffic Controller", BlueprintCallable)
  void EmptyTrafficLights();

  UFUNCTION(Category = "Traffic Controller", BlueprintPure)
  const FString &GetControllerId() const;

  UFUNCTION(Category = "Traffic Controller", BlueprintCallable)
  void SetControllerId(const FString &Id);

  UFUNCTION(Category = "Traffic Controller", BlueprintCallable)
  void AddTrafficLight(UTrafficLightComponent * TrafficLight);

  UFUNCTION(Category = "Traffic Controller", BlueprintCallable)
  void RemoveTrafficLight(UTrafficLightComponent * TrafficLight);

  void AddCarlaActorTrafficLight(FCarlaActor* CarlaActor);

  void RemoveCarlaActorTrafficLight(FCarlaActor* CarlaActor);

  UFUNCTION(Category = "Traffic Controller", BlueprintCallable)
  bool IsCycleFinished() const;

  UFUNCTION(Category = "Traffic Controller", BlueprintCallable)
  void SetTrafficLightsState(ETrafficLightState NewState);

  UFUNCTION(Category = "Traffic Controller", BlueprintCallable)
  int GetSequence() const;

  UFUNCTION(Category = "Traffic Controller", BlueprintCallable)
  void SetSequence(int InSequence);

  UFUNCTION(Category = "Traffic Controller", BlueprintCallable)
  void ResetState();

  UFUNCTION(Category = "Traffic Controller", BlueprintCallable)
  void SetYellowTime(float NewTime);

  UFUNCTION(Category = "Traffic Controller", BlueprintCallable)
  void SetRedTime(float NewTime);

  UFUNCTION(Category = "Traffic Controller", BlueprintCallable)
  void SetGreenTime(float NewTime);

  UFUNCTION(Category = "Traffic Controller", BlueprintCallable)
  float GetGreenTime() const;

  UFUNCTION(Category = "Traffic Controller", BlueprintCallable)
  float GetYellowTime() const;

  UFUNCTION(Category = "Traffic Controller", BlueprintCallable)
  float GetRedTime() const;

  UFUNCTION(Category = "Traffic Controller", BlueprintCallable)
  float GetElapsedTime() const;

  UFUNCTION(Category = "Traffic Controller", BlueprintCallable)
  void SetElapsedTime(float InElapsedTime);

  void SetGroup(ATrafficLightGroup* Group);

  ATrafficLightGroup* GetGroup();

  const ATrafficLightGroup* GetGroup() const;

  ETrafficLightState GetCurrentLightState() const
  {
    return CurrentLightState;
  }
  void SetCurrentLightState(ETrafficLightState NewState)
  {
    CurrentLightState = NewState;
  }

private:

  void SetStateTime(const ETrafficLightState State, float NewTime);

  float GetStateTime(const ETrafficLightState State) const;

  UPROPERTY(Category = "Traffic Controller", EditAnywhere)
  FString ControllerId = "";

  UPROPERTY(Category = "Traffic Controller", EditAnywhere)
  int CurrentState = 0;

  // 与信号灯的状态配对（时间 - 状态），例如绿灯 10 秒
  UPROPERTY(Category = "Traffic Controller", EditAnywhere)
  TArray<FTrafficLightStage> LightStates = {
    {10, ETrafficLightState::Green},
    { 3, ETrafficLightState::Yellow},
    { 2, ETrafficLightState::Red}
  };

  UPROPERTY(Category = "Traffic Controller", EditAnywhere)
  TArray<UTrafficLightComponent *> TrafficLights;

  TArray<FCarlaActor *> TrafficLightCarlaActors;

  UPROPERTY(Category = "Traffic Controller", VisibleAnywhere)
  ATrafficLightGroup* TrafficLightGroup;

  // 交叉路口的序列（暂时未使用）
  UPROPERTY(Category = "Traffic Controller", EditAnywhere)
  int Sequence = 0;

  UPROPERTY()
  float ElapsedTime = 0;

  ETrafficLightState CurrentLightState = ETrafficLightState::Green;
};
