// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "TrafficLightComponent.h"
#include "TrafficLightGroup.h"
#include "TrafficSignBase.h"
#include "Carla/OpenDrive/OpenDrive.h"

#include "TrafficLightManager.generated.h"

/// 负责创建和分配交通灯组、控制器和组件的类。
UCLASS()
class CARLA_API ATrafficLightManager : public AActor
{
  GENERATED_BODY()

public:

  ATrafficLightManager();

  UFUNCTION(BlueprintCallable, Category = "Traffic Light Manager")
  void RegisterLightComponentFromOpenDRIVE(UTrafficLightComponent * TrafficLight);

  UFUNCTION(BlueprintCallable, Category = "Traffic Light Manager")
  void RegisterLightComponentGenerated(UTrafficLightComponent * TrafficLight);

  const boost::optional<carla::road::Map> &GetMap();

  UFUNCTION(BlueprintCallable, Category = "Traffic Light Manager")
  ATrafficLightGroup* GetTrafficGroup(int JunctionId);

  UFUNCTION(BlueprintCallable, Category = "Traffic Light Manager")
  UTrafficLightController* GetController(FString ControllerId);

  UFUNCTION(BlueprintCallable, Category = "Traffic Light Manager")
  USignComponent* GetTrafficSign(FString SignId);

  UFUNCTION(BlueprintCallable, Category = "Traffic Light Manager")
  void SetFrozen(bool InFrozen);

  UFUNCTION(BlueprintCallable, Category = "Traffic Light Manager")
  bool GetFrozen();

  UFUNCTION(CallInEditor, Category = "Traffic Light Manager")
  void GenerateSignalsAndTrafficLights();

  UFUNCTION(CallInEditor, Category = "Traffic Light Manager")
  void RemoveGeneratedSignalsAndTrafficLights();

  UFUNCTION(CallInEditor, Category = "Traffic Light Manager")
  void MatchTrafficLightActorsWithOpenDriveSignals();

  // Called when the game starts by the gamemode
  void InitializeTrafficLights();

private:

  void SpawnTrafficLights();

  void SpawnSignals();

  void RemoveRoadrunnerProps() const;

  void RemoveAttachedProps(TArray<AActor*> Actors) const;

  // 映射对 ATrafficLightGroup (交叉路口) 的引用
  UPROPERTY()
  TMap<int, ATrafficLightGroup *> TrafficGroups;

  // 映射对 UTrafficLightController（控制器）的引用
  UPROPERTY()
  TMap<FString, UTrafficLightController *> TrafficControllers;

  // 映射到单个 TrafficLightComponents 的引用
  UPROPERTY()
  TMap<FString, USignComponent *> TrafficSignComponents;

  // 交通标志 TrafficSigns 的映射引用
  TArray<ATrafficSignBase*> TrafficSigns;

  UPROPERTY(EditAnywhere, Category= "Traffic Light Manager")
  TSubclassOf<AActor> TrafficLightModel;

  // 将 OpenDRIVE 类型与交通标志蓝图关联起来
  UPROPERTY(EditAnywhere, Category= "Traffic Light Manager")
  TMap<FString, TSubclassOf<AActor>> TrafficSignsModels;

  UPROPERTY(EditAnywhere, Category= "Traffic Light Manager")
  TMap<FString, TSubclassOf<USignComponent>> SignComponentModels;

  UPROPERTY(EditAnywhere, Category= "Traffic Light Manager")
  TMap<FString, TSubclassOf<AActor>> SpeedLimitModels;

  UPROPERTY(Category = "Traffic Light Manager", VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
  USceneComponent *SceneComponent;

  UPROPERTY(EditAnywhere, Category= "Traffic Light Manager")
  bool TrafficLightsGenerated = false;

  // 没有对应 OpenDRIVE 交叉路口的 TrafficLightGroups 的 ID
  UPROPERTY()
  int TrafficLightGroupMissingId = -2;

  // 没有对应 OpenDRIVE 交叉路口的 TrafficLightControllers 的 ID
  UPROPERTY()
  int TrafficLightControllerMissingId = -1;

  // 没有对应 OpenDRIVE 交叉路口的 TrafficLightComponents 的 ID
  UPROPERTY()
  int TrafficLightComponentMissingId = -1;

  UPROPERTY()
  bool bTrafficLightsFrozen = false;

};
