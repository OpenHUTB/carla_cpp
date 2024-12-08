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

/// ���п�ѡAI����ʽ����������
UCLASS()
class CARLA_API AWheeledVehicleAIController final : public AController
{
  GENERATED_BODY()

  // ===========================================================================
  /// @name ���캯������������
  // ===========================================================================
  /// @{

public:

  AWheeledVehicleAIController(const FObjectInitializer &ObjectInitializer);

  ~AWheeledVehicleAIController();

  /// @}
  // ===========================================================================
  /// @name ��������д
  // ===========================================================================
  /// @{

public:

  void OnPossess(APawn *aPawn) override;

  void OnUnPossess() override;

  void Tick(float DeltaTime) override;

  /// @}
  // ===========================================================================
  /// @name �����Ƶĳ���
  // ===========================================================================
  /// @{

public:

  // ����Ƿ����ڿ���һ������
  UFUNCTION(Category = "Wheeled Vehicle Controller", BlueprintCallable)
  bool IsPossessingAVehicle() const
  {
    return Vehicle != nullptr;
  }

  // ��ȡ�����Ƶĳ���
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
  /// @name ����ѡ��
  // ===========================================================================
  /// @{

  // ����ճ�Կ���
  UFUNCTION(Category = "Wheeled Vehicle Controller", BlueprintCallable)
  void SetStickyControl(bool bEnabled)
  {
    bControlIsSticky = bEnabled;
  }

  /// @}
  // ===========================================================================
  /// @name ��·��ͼ
  // ===========================================================================
  /// @{

public:

  // ���õ�·��ͼ
  void SetRoadMap(URoadMap *InRoadMap)
  {
    RoadMap = InRoadMap;
  }

  // ��ȡ��·��ͼ
  UFUNCTION(Category = "Road Map", BlueprintCallable)
  URoadMap *GetRoadMap()
  {
    return RoadMap;
  }

  /// @}
  // ===========================================================================
  /// @name �������
  // ===========================================================================
  /// @{

public:

  // ��ȡ�������
  UFUNCTION(Category = "Random Engine", BlueprintCallable)
  URandomEngine *GetRandomEngine()
  {
    check(RandomEngine != nullptr);
    return RandomEngine;
  }

  /// @}
  // ===========================================================================
  /// @name �Զ���ʻ
  // ===========================================================================
  /// @{

public:

  // ����Զ���ʻ�Ƿ�����
  UFUNCTION(Category = "Wheeled Vehicle Controller", BlueprintCallable)
  bool IsAutopilotEnabled() const
  {
    return bAutopilotEnabled;
  }

  // �����Զ���ʻ
  UFUNCTION(Category = "Wheeled Vehicle Controller", BlueprintCallable)
  void SetAutopilot(bool Enable, bool KeepState = false)
  {
    if (IsAutopilotEnabled() != Enable)
    {
      ConfigureAutopilot(Enable, KeepState);
    }
  }

  // �л��Զ���ʻ״̬
  UFUNCTION(Category = "Wheeled Vehicle Controller", BlueprintCallable)
  void ToggleAutopilot()
  {
    ConfigureAutopilot(!bAutopilotEnabled);
  }

private:

  void ConfigureAutopilot(const bool Enable, const bool KeepState = false);

  /// @}
  // ===========================================================================
  /// @name ��ͨ
  // ===========================================================================
  /// @{

public:

  // ��ȡ��ǰ�ٶ����ƣ�km/h��
  UFUNCTION(Category = "Wheeled Vehicle Controller", BlueprintCallable)
  float GetSpeedLimit() const
  {
    return SpeedLimit;
  }

  // ���ó����ٶ����ƣ�km/h��
  UFUNCTION(Category = "Wheeled Vehicle Controller", BlueprintCallable)
  void SetSpeedLimit(float InSpeedLimit)
  {
    SpeedLimit = InSpeedLimit;
  }

  // ��ȡ��ǰӰ��ó����Ľ�ͨ��״̬
  UFUNCTION(Category = "Wheeled Vehicle Controller", BlueprintCallable)
  ETrafficLightState GetTrafficLightState() const
  {
    return TrafficLightState;
  }

  // ���õ�ǰӰ��ó����Ľ�ͨ��״̬
  UFUNCTION(Category = "Wheeled Vehicle Controller", BlueprintCallable)
  void SetTrafficLightState(ETrafficLightState InTrafficLightState)
  {
    TrafficLightState = InTrafficLightState;
  }

  // ��ȡ��ǰӰ��ó����Ľ�ͨ��
  UFUNCTION(Category = "Wheeled Vehicle Controller", BlueprintCallable)
  ATrafficLightBase *GetTrafficLight() const
  {
    return TrafficLight;
  }

  // ���õ�ǰӰ��ó����Ľ�ͨ��
  UFUNCTION(Category = "Wheeled Vehicle Controller", BlueprintCallable)
  void SetTrafficLight(ATrafficLightBase *InTrafficLight)
  {
    TrafficLight = InTrafficLight;
  }

  // �����Զ���ʻʱҪ��ѭ�Ĺ̶�·��
  UFUNCTION(Category = "Wheeled Vehicle Controller", BlueprintCallable)
  void SetFixedRoute(const TArray<FVector> &Locations, bool bOverwriteCurrent = true);

  /// @}

private:

  // �����Ƶĳ���
  UPROPERTY()
  ACarlaWheeledVehicle *Vehicle = nullptr;

  // ��·��ͼ
  UPROPERTY()
  URoadMap *RoadMap = nullptr;

  // �������
  UPROPERTY()
  URandomEngine *RandomEngine = nullptr;

  // �Զ���ʻ�Ƿ�����
  UPROPERTY(VisibleAnywhere)
  bool bAutopilotEnabled = false;

  // �����Ƿ����ճ��
  UPROPERTY(VisibleAnywhere)
  bool bControlIsSticky = true;

  // �ٶ�����
  UPROPERTY(VisibleAnywhere)
  float SpeedLimit = 30.0f;

  // ��ͨ��״̬
  UPROPERTY(VisibleAnywhere)
  ETrafficLightState TrafficLightState = ETrafficLightState::Green;

  // ���ת��Ƕ�
  UPROPERTY(VisibleAnywhere)
  float MaximumSteerAngle = -1.0f;

  // ��ǰӰ�쳵���Ľ�ͨ��
  UPROPERTY()
  ATrafficLightBase *TrafficLight;

  // Ŀ��λ�ö���
  std::queue<FVector> TargetLocations;
};