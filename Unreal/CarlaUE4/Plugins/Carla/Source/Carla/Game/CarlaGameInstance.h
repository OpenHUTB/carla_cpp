// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Engine/GameInstance.h"

#include "Carla/Game/CarlaEngine.h"
#include "Carla/Recorder/CarlaRecorder.h"
#include "Carla/Server/CarlaServer.h"

#include <compiler/disable-ue4-macros.h>
#include <carla/rpc/MapLayer.h>
#include <carla/rpc/OpendriveGenerationParameters.h>
#include <compiler/enable-ue4-macros.h>

#include "CarlaGameInstance.generated.h"

class UCarlaSettings;

// The game instance contains elements that must be kept alive in between
// levels. It is instantiate once per game.
// 定义一个名为 UCarlaGameInstance 的类，继承自 UGameInstance，并且是一个可以在蓝图中使用的类（UCLASS）
UCLASS()
class CARLA_API UCarlaGameInstance : public UGameInstance
{
  GENERATED_BODY()

public:

  // 构造函数
  UCarlaGameInstance();

  // 析构函数
  ~UCarlaGameInstance();

  // 获取 CarlaSettings 的引用，如果 CarlaSettings 为 nullptr 会触发断言检查
  UCarlaSettings &GetCarlaSettings()
  {
    check(CarlaSettings!= nullptr);
    return *CarlaSettings;
  }

  // 获取 CarlaSettings 的 const 引用，如果 CarlaSettings 为 nullptr 会触发断言检查
  const UCarlaSettings &GetCarlaSettings() const
  {
    check(CarlaSettings!= nullptr);
    return *CarlaSettings;
  }

  // 额外的函数重载，用于在蓝图中调用，返回一个指向 CarlaSettings 的指针
  UFUNCTION(BlueprintCallable)
  UCarlaSettings *GetCARLASettings()
  {
    return CarlaSettings;
  }

  // 获取 CarlaEpisode 的指针，调用 CarlaEngine 的 GetCurrentEpisode 函数
  UFUNCTION(BlueprintCallable)
  UCarlaEpisode *GetCarlaEpisode()
  {
    return CarlaEngine.GetCurrentEpisode();
  }

  // 通知游戏初始化，调用 CarlaEngine 的 NotifyInitGame 函数并传递 CarlaSettings
  void NotifyInitGame()
  {
    CarlaEngine.NotifyInitGame(GetCarlaSettings());
  }

  // 通知开始新的游戏章节，调用 CarlaEngine 的 NotifyBeginEpisode 函数
  void NotifyBeginEpisode(UCarlaEpisode &Episode)
  {
    CarlaEngine.NotifyBeginEpisode(Episode);
  }

  // 通知结束游戏章节，调用 CarlaEngine 的 NotifyEndEpisode 函数
  void NotifyEndEpisode()
  {
    CarlaEngine.NotifyEndEpisode();
  }

  // 获取 CarlaServer 的 const 引用，调用 CarlaEngine 的 GetServer 函数
  const FCarlaServer &GetServer() const
  {
    return CarlaEngine.GetServer();
  }

  // 获取 CarlaServer 的引用，调用 CarlaEngine 的 GetServer 函数
  FCarlaServer &GetServer()
  {
    return CarlaEngine.GetServer();
  }

  // 设置 OpendriveGenerationParameters
  void SetOpendriveGenerationParameters(
      const carla::rpc::OpendriveGenerationParameters & Parameters)
  {
    GenerationParameters = Parameters;
  }

  // 获取 OpendriveGenerationParameters 的 const 引用
  const carla::rpc::OpendriveGenerationParameters&
      GetOpendriveGenerationParameters() const
  {
    return GenerationParameters;
  }

  // 蓝图可调用的函数，用于设置 MapLayer
  UFUNCTION(Category = "Carla Game Instance", BlueprintCallable)
  void SetMapLayer(int32 MapLayer)
  {
    CurrentMapLayer = MapLayer;
  }

  // 蓝图可调用的函数，用于获取当前的 MapLayer
  UFUNCTION(Category = "Carla Game Instance", BlueprintCallable)
  int32 GetCurrentMapLayer() const
  {
    return CurrentMapLayer;
  }

  // 获取 CarlaEngine 的指针
  FCarlaEngine* GetCarlaEngine()
  {
    return &CarlaEngine;
  }

private:

  // 一个可编辑的属性，存储 CarlaSettings 的指针，默认为 nullptr
  UPROPERTY(Category = "CARLA Settings", EditAnywhere)
  UCarlaSettings *CarlaSettings = nullptr;

  // CarlaEngine 对象
  FCarlaEngine CarlaEngine;

  // 一个属性，存储 CarlaRecorder 的指针，默认为 nullptr
  UPROPERTY()
  ACarlaRecorder *Recorder = nullptr;

  // 存储 OpendriveGenerationParameters 的对象
  carla::rpc::OpendriveGenerationParameters GenerationParameters;

  // 一个可编辑的属性，存储当前的 MapLayer，默认为 carla::rpc::MapLayer::All
  UPROPERTY(Category = "CARLA Game Instance", EditAnywhere)
  int32 CurrentMapLayer = static_cast<int32>(carla::rpc::MapLayer::All);

  // 存储 MapPath 的字符串
  UPROPERTY()
  FString _MapPath;
};
