// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <queue>

#include "GameFramework/Controller.h"

#include "Traffic/TrafficLightState.h"
#include "Vehicle/VehicleControl.h"

#include "WheeledVehicleAIController.generated.h"

class ACarlaWheeledVehicle;
class URandomEngine;
class URoadMap;

/// 带有可选AI的轮式车辆控制器
UCLASS()
class CARLA_API AWheeledVehicleAIController final : public AController
{
  GENERATED_BODY()

  // ===========================================================================
  /// @name 构造函数和析构函数
  // ===========================================================================
  /// @{

public:

  AWheeledVehicleAIController(const FObjectInitializer &ObjectInitializer);

  ~AWheeledVehicleAIController();

  /// @}
  // ===========================================================================
  /// @name 控制器重写
  // ===========================================================================
  /// @{

public:

  void OnPossess(APawn *aPawn) override;

  void OnUnPossess() override;

  void Tick(float DeltaTime) override;

  /// @}
  // ===========================================================================
  /// @name 被控制的车辆
  // ===========================================================================
  /// @{

public:

  // 检查是否正在控制一辆车辆
  UFUNCTION(Category = "Wheeled Vehicle Controller", BlueprintCallable)
  bool IsPossessingAVehicle() const
  {
    return Vehicle != nullptr;
  }

  // 获取被控制的车辆
  UFUNCTION(Category = "Wheeled Vehicle Controller", BlueprintCallable)
  ACarlaWheeledVehicle *GetPossessedVehicle()
  {
    return Vehicle;
  }

  const ACarlaWheeledVehicle *GetPossessedVehicle() const
  {
    return Vehicle;
  }

  /// @}
  // ===========================================================================
  /// @name 控制选项
  // ===========================================================================
  /// @{

  // 设置粘性控制
  UFUNCTION(Category = "Wheeled Vehicle Controller", BlueprintCallable)
  void SetStickyControl(bool bEnabled)
  {
    bControlIsSticky = bEnabled;
  }

  /// @}
  // ===========================================================================
  /// @name 道路地图
  // ===========================================================================
  /// @{

public:

  // 设置道路地图
  void SetRoadMap(URoadMap *InRoadMap)
  {
    RoadMap = InRoadMap;
  }

  // 获取道路地图
  UFUNCTION(Category = "Road Map", BlueprintCallable)
  URoadMap *GetRoadMap()
  {
    return RoadMap;
  }

  /// @}
  // ===========================================================================
  /// @name 随机引擎
  // ===========================================================================
  /// @{

public:

  // 获取随机引擎
  UFUNCTION(Category = "Random Engine", BlueprintCallable)
  URandomEngine *GetRandomEngine()
  {
    check(RandomEngine != nullptr);
    return RandomEngine;
  }

  /// @}
  // ===========================================================================
  /// @name 自动驾驶
  // ===========================================================================
  /// @{

public:

  // 检查自动驾驶是否启用
  UFUNCTION(Category = "Wheeled Vehicle Controller", BlueprintCallable)
  bool IsAutopilotEnabled() const
  {
    return bAutopilotEnabled;
  }

  // 设置自动驾驶
  UFUNCTION(Category = "Wheeled Vehicle Controller", BlueprintCallable)
  void SetAutopilot(bool Enable, bool KeepState = false)
  {
    if (IsAutopilotEnabled() != Enable)
    {
      ConfigureAutopilot(Enable, KeepState);
    }
  }

  // 切换自动驾驶状态
  UFUNCTION(Category = "Wheeled Vehicle Controller", BlueprintCallable)
  void ToggleAutopilot()
  {
    ConfigureAutopilot(!bAutopilotEnabled);
  }

private:

  void ConfigureAutopilot(const bool Enable, const bool KeepState = false);

  /// @}
  // ===========================================================================
  /// @name 交通
  // ===========================================================================
  /// @{

public:

  // 获取当前速度限制（km/h）
  UFUNCTION(Category = "Wheeled Vehicle Controller", BlueprintCallable)
  float GetSpeedLimit() const
  {
    return SpeedLimit;
  }

  // 设置车辆速度限制（km/h）
  UFUNCTION(Category = "Wheeled Vehicle Controller", BlueprintCallable)
  void SetSpeedLimit(float InSpeedLimit)
  {
    SpeedLimit = InSpeedLimit;
  }

  // 获取当前影响该车辆的交通灯状态
  UFUNCTION(Category = "Wheeled Vehicle Controller", BlueprintCallable)
  ETrafficLightState GetTrafficLightState() const
  {
    return TrafficLightState;
  }

  // 设置当前影响该车辆的交通灯状态
  UFUNCTION(Category = "Wheeled Vehicle Controller", BlueprintCallable)
  void SetTrafficLightState(ETrafficLightState InTrafficLightState)
  {
    TrafficLightState = InTrafficLightState;
  }

  // 获取当前影响该车辆的交通灯
  UFUNCTION(Category = "Wheeled Vehicle Controller", BlueprintCallable)
  ATrafficLightBase *GetTrafficLight() const
  {
    return TrafficLight;
  }

  // 设置当前影响该车辆的交通灯
  UFUNCTION(Category = "Wheeled Vehicle Controller", BlueprintCallable)
  void SetTrafficLight(ATrafficLightBase *InTrafficLight)
  {
    TrafficLight = InTrafficLight;
  }

  // 设置自动驾驶时要遵循的固定路线
  UFUNCTION(Category = "Wheeled Vehicle Controller", BlueprintCallable)
  void SetFixedRoute(const TArray<FVector> &Locations, bool bOverwriteCurrent = true);

  /// @}

private:

  // 被控制的车辆
  UPROPERTY()
  ACarlaWheeledVehicle *Vehicle = nullptr;

  // 道路地图
  UPROPERTY()
  URoadMap *RoadMap = nullptr;

  // 随机引擎
  UPROPERTY()
  URandomEngine *RandomEngine = nullptr;

  // 自动驾驶是否启用
  UPROPERTY(VisibleAnywhere)
  bool bAutopilotEnabled = false;

  // 控制是否具有粘性
  UPROPERTY(VisibleAnywhere)
  bool bControlIsSticky = true;

  // 速度限制
  UPROPERTY(VisibleAnywhere)
  float SpeedLimit = 30.0f;

  // 交通灯状态
  UPROPERTY(VisibleAnywhere)
  ETrafficLightState TrafficLightState = ETrafficLightState::Green;

  // 最大转向角度
  UPROPERTY(VisibleAnywhere)
  float MaximumSteerAngle = -1.0f;

  // 当前影响车辆的交通灯
  UPROPERTY()
  ATrafficLightBase *TrafficLight;

  // 目标位置队列
  std::queue<FVector> TargetLocations;
};