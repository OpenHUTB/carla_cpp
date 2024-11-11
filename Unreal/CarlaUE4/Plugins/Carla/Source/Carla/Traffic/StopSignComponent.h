// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "CoreMinimal.h"
#include "SignComponent.h"
#include "Carla/Vehicle/WheeledVehicleAIController.h"
#include "StopSignComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CARLA_API UStopSignComponent : public USignComponent
{
  GENERATED_BODY()

public:

  virtual void InitializeSign(const carla::road::Map &Map) override;

private:

  void GenerateStopBox(const FTransform BoxTransform,
      const FVector BoxSize);

  void GenerateCheckBox(const FTransform BoxTransform,
      float BoxSize);

  /// 尽量给停车标志处的车辆让路，检查复选框中的车辆数量
  UFUNCTION(BlueprintCallable)
  void GiveWayIfPossible();

  void DelayedGiveWay(float Delay);

  UFUNCTION(BlueprintCallable)
  void OnOverlapBeginStopEffectBox(UPrimitiveComponent *OverlappedComp,
      AActor *OtherActor,
      UPrimitiveComponent *OtherComp,
      int32 OtherBodyIndex,
      bool bFromSweep,
      const FHitResult &SweepResult);

  UFUNCTION(BlueprintCallable)
  void OnOverlapEndStopEffectBox(UPrimitiveComponent *OverlappedComp,
      AActor *OtherActor,
      UPrimitiveComponent *OtherComp,
      int32 OtherBodyIndex);

  UFUNCTION(BlueprintCallable)
  void OnOverlapBeginStopCheckBox(UPrimitiveComponent *OverlappedComp,
      AActor *OtherActor,
      UPrimitiveComponent *OtherComp,
      int32 OtherBodyIndex,
      bool bFromSweep,
      const FHitResult &SweepResult);

  UFUNCTION(BlueprintCallable)
  void OnOverlapEndStopCheckBox(UPrimitiveComponent *OverlappedComp,
      AActor *OtherActor,
      UPrimitiveComponent *OtherComp,
      int32 OtherBodyIndex);

  void RemoveSameVehicleInBothLists();

  UPROPERTY()
  TSet<ACarlaWheeledVehicle*> VehiclesInStop;

  UPROPERTY()
  TMap<ACarlaWheeledVehicle*, int> VehiclesToCheck;
};
