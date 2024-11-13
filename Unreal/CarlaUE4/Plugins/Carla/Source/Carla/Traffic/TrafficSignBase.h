// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "GameFramework/Actor.h"

#include "TrafficSignBase.generated.h"

class UBoxComponent;

// UENUM 将枚举类型进行标记，方便 虚幻头文件解析工具(UHT, UnrealHeaderTool) 为其生成相应的类型文件，生成反射系统需要的代码。
// BlueprintType说明符令该枚举可以在蓝图中自如使用。
UENUM(BlueprintType)
enum class ETrafficSignState : uint8 {
  Null = 0, // 解决 UE4.24 枚举问题
  UNKNOWN            = 0u   UMETA(DisplayName = "UNKNOWN"),
  TrafficLightRed    = 1u   UMETA(DisplayName = "Traffic Light - Red"),
  TrafficLightYellow = 2u   UMETA(DisplayName = "Traffic Light - Yellow"),
  TrafficLightGreen  = 3u   UMETA(DisplayName = "Traffic Light - Green"),
  SpeedLimit_30             UMETA(DisplayName = "Speed Limit - 30"),
  SpeedLimit_40             UMETA(DisplayName = "Speed Limit - 40"),
  SpeedLimit_50             UMETA(DisplayName = "Speed Limit - 50"),
  SpeedLimit_60             UMETA(DisplayName = "Speed Limit - 60"),
  SpeedLimit_90             UMETA(DisplayName = "Speed Limit - 90"),
  SpeedLimit_100            UMETA(DisplayName = "Speed Limit - 100"),
  SpeedLimit_120            UMETA(DisplayName = "Speed Limit - 120"),
  SpeedLimit_130            UMETA(DisplayName = "Speed Limit - 130"),
  StopSign                  UMETA(DisplayName = "Stop Sign"),
  YieldSign                 UMETA(DisplayName = "Yield Sign")
};

UCLASS()
class CARLA_API ATrafficSignBase : public AActor {

  GENERATED_BODY()

public:

  ATrafficSignBase(const FObjectInitializer &ObjectInitializer);

  UFUNCTION(BlueprintCallable)
  ETrafficSignState GetTrafficSignState() const
  {
    return TrafficSignState;
  }

  UFUNCTION(BlueprintCallable)
  void SetTrafficSignState(ETrafficSignState State)
  {
    TrafficSignState = State;
  }

  UFUNCTION(BlueprintImplementableEvent)
  UBoxComponent *GetTriggerVolume() const;

  TArray<UBoxComponent*> GetTriggerVolumes() const;

private:

  UPROPERTY(Category = "Traffic Sign", EditAnywhere)
  ETrafficSignState TrafficSignState = ETrafficSignState::UNKNOWN;
};
