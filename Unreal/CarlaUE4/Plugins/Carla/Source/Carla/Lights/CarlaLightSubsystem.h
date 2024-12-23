// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once//预处理指令

#include <vector>//<vector>是C++ 标准库中的头文件，包含这个头文件可以使用向量（vector）数据结构。

#include <compiler/disable-ue4-macros.h>//这是一个自定义的包含路径下的头文件（从路径推测可能是某个编译器相关且与UE4相关宏有关的头文件）。
#include <carla/rpc/LightState.h>//这是一个自定义路径下（carla/rpc）的头文件，与LightState有关。
#include <compiler/enable-ue4-macros.h>//这是与之前disable - ue4 - macros.h相对应的头文件。

#include "Carla.h"//这是自定义的头文件（从双引号而不是尖括号可知），名为Carla.h。
#include "CoreMinimal.h"//这是虚幻引擎中的基础头文件。
#include "CarlaLight.h"//这是自定义的头文件，与CarlaLight相关。
#include "Subsystems/WorldSubsystem.h"//这是一个自定义路径下（Subsystems）的头文件，与WorldSubsystem相关。

#include "CarlaLightSubsystem.generated.h"//这是一个由代码生成工具（可能是虚幻引擎的代码生成工具）自动生成的头文件，与CarlaLightSubsystem相关。

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
  //结束 USubsystem
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
