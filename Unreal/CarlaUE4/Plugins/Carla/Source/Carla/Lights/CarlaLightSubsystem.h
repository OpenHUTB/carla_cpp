// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// 本作品根据 MIT 许可证的条款进行许可。
// 有关副本，请参阅 <https://opensource.org/licenses/MIT>。

#pragma once

#include <vector>

#include <compiler/disable-ue4-macros.h>
#include <carla/rpc/LightState.h>
#include <compiler/enable-ue4-macros.h>

#include "Carla.h"
#include "CoreMinimal.h"
#include "CarlaLight.h"
#include "Subsystems/WorldSubsystem.h"

#include "CarlaLightSubsystem.generated.h"

/**
 *
 */
UCLASS(Blueprintable, BlueprintType)
class CARLA_API UCarlaLightSubsystem : public UWorldSubsystem
{
  GENERATED_BODY()

  //using cr = carla::rpc;

public:

  // 开始 USubsystem
  void Initialize(FSubsystemCollectionBase& Collection) override;
  // 结束 USubsystem
  void Deinitialize() override;

  void RegisterLight(UCarlaLight* CarlaLight);

  void UnregisterLight(UCarlaLight* CarlaLight);

  UFUNCTION(BlueprintCallable)
  bool IsUpdatePending() const;

  UFUNCTION(BlueprintCallable)
  int32 NumLights() const {
    return Lights.Num();
  }

  std::vector<carla::rpc::LightState> GetLights(FString Client);

  void SetLights(
      FString Client,
      std::vector<carla::rpc::LightState> LightsToSet,
      bool DiscardClient = false);

  UCarlaLight* GetLight(int Id);

  TMap<int, UCarlaLight* >& GetLights()
  {
    return Lights;
  }

  void SetDayNightCycle(const bool active);

private:

  void SetClientStatesdirty(FString ClientThatUpdate);

  TMap<int, UCarlaLight* > Lights;

  // 每个客户端的标志，用于指示是否需要进行更新
  TMap<FString, bool> ClientStates;
  // 由于客户端在模拟中没有正确的 ID，因此
  // 我使用 host ： 端口对。

};
