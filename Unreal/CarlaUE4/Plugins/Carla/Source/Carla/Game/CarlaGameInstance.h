// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once // 指示此头文件只应被包含一次，防止重复包含

#include "Engine/GameInstance.h" // 包含Unreal Engine游戏实例类的头文件，提供游戏实例的基础功能

#include "Carla/Game/CarlaEngine.h" // 包含Carla游戏引擎的头文件，管理Carla仿真的核心组件
#include "Carla/Recorder/CarlaRecorder.h" // 包含Carla录像机的头文件，用于记录和回放仿真
#include "Carla/Server/CarlaServer.h" // 包含Carla服务器的头文件，处理网络通信和多客户端管理

#include <compiler/disable-ue4-macros.h> // 禁用Unreal Engine的宏，防止与Carla代码冲突
#include <carla/rpc/MapLayer.h> // 包含Carla RPC地图层的头文件，定义地图层相关的RPC结构
#include <carla/rpc/OpendriveGenerationParameters.h> // 包含Carla RPC Opendrive生成参数的头文件，定义Opendrive生成相关的RPC结构
#include <compiler/enable-ue4-macros.h> // 启用Unreal Engine的宏

#include "CarlaGameInstance.generated.h" // 包含由Unreal Engine自动生成的代码，用于支持UStruct的反射和蓝图系统

class UCarlaSettings; // 前向声明Carla设置类，用于配置和存储Carla仿真的设置

// UCarlaGameInstance类，继承自UGameInstance，用于在Carla仿真中保持跨关卡存活的元素
UCLASS() // 标记为可在蓝图中使用的类
class CARLA_API UCarlaGameInstance : public UGameInstance // 声明类为游戏实例类
{
  GENERATED_BODY() // 由Unreal Engine生成的代码，用于创建类的实例

public:

  // 构造函数，创建UCarlaGameInstance实例时调用
  UCarlaGameInstance();

  // 析构函数，销毁UCarlaGameInstance实例时调用
  ~UCarlaGameInstance();

  // 获取Carla设置的引用，如果CarlaSettings为空则断言失败
  UCarlaSettings &GetCarlaSettings()
  {
    check(CarlaSettings != nullptr); // 断言CarlaSettings不为空
    return *CarlaSettings; // 返回Carla设置的引用
  }

  // 获取Carla设置的const引用，如果CarlaSettings为空则断言失败
  const UCarlaSettings &GetCarlaSettings() const
  {
    check(CarlaSettings != nullptr); // 断言CarlaSettings不为空
    return *CarlaSettings; // 返回Carla设置的const引用
  }

  // 为蓝图提供额外的函数重载，返回Carla设置的指针
  UFUNCTION(BlueprintCallable) // 标记为可在蓝图中调用的函数
  UCarlaSettings *GetCARLASettings()
  {
    return CarlaSettings; // 返回Carla设置的指针
  }

  // 获取CarlaEpisode的指针，通过调用CarlaEngine的GetCurrentEpisode函数
  UFUNCTION(BlueprintCallable) // 标记为可在蓝图中调用的函数
  UCarlaEpisode *GetCarlaEpisode()
  {
    return CarlaEngine.GetCurrentEpisode(); // 返回当前CarlaEpisode的指针
  }

  // 通知游戏初始化，调用CarlaEngine的NotifyInitGame函数并传递CarlaSettings
  void NotifyInitGame()
  {
    CarlaEngine.NotifyInitGame(GetCarlaSettings()); // 通知CarlaEngine游戏初始化，并传递Carla设置
  }

  // 通知开始新的游戏章节，调用CarlaEngine的NotifyBeginEpisode函数
  void NotifyBeginEpisode(UCarlaEpisode &Episode)
  {
    CarlaEngine.NotifyBeginEpisode(Episode); // 通知CarlaEngine开始新的游戏章节
  }

  // 通知结束游戏章节，调用CarlaEngine的NotifyEndEpisode函数
  void NotifyEndEpisode()
  {
    CarlaEngine.NotifyEndEpisode(); // 通知CarlaEngine结束游戏章节
  }

  // 获取CarlaServer的const引用，通过调用CarlaEngine的GetServer函数
  // 具体实现未提供，以下为注释
  /*
  UFUNCTION(BlueprintCallable)
  const CarlaServer &GetCarlaServer() const
  {
    return CarlaEngine.GetServer();
  }
  */

private:
  // 成员变量，存储Carla设置的指针
  UCarlaSettings *CarlaSettings;

  // 成员变量，存储Carla游戏引擎的引用
  CarlaEngine CarlaEngine;
};

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
