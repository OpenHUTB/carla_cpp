// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once// [编译指令] 确保头文件只被编译一次 
// [包含必要的头文件] 
#include "Carla/Recorder/CarlaRecorder.h"// 包含CarlaRecorder类的定义
#include "Carla/Sensor/WorldObserver.h"// 包含WorldObserver类的定义
#include "Carla/Server/CarlaServer.h"// 包含CarlaServer类的定义  
#include "Carla/Settings/EpisodeSettings.h"// 包含EpisodeSettings相关定义
#include "Carla/Util/NonCopyable.h"// 包含NonCopyable类的定义，用于禁止复制
#include "Carla/Game/FrameData.h"// 包含FrameData结构体的定义

#include "Misc/CoreDelegates.h" // 包含核心委托的定义
// [禁用/启用UE4宏]
#include <compiler/disable-ue4-macros.h>// 禁用Unreal Engine 4的宏
#include <carla/multigpu/router.h>// 包含多GPU和ROS2相关的Carla库头文件 
#include <carla/multigpu/primaryCommands.h>
#include <carla/multigpu/secondary.h>
#include <carla/multigpu/secondaryCommands.h>
#include <carla/ros2/ROS2.h>
#include <compiler/enable-ue4-macros.h>// 重新启用Unreal Engine 4的宏

#include <mutex>// 包含C++标准库的mutex类，用于线程同步
// [前向声明]
class UCarlaSettings;// 前向声明UCarlaSettings类 
struct FEpisodeSettings;// 前向声明FEpisodeSettings结构体
// [FCarlaEngine类定义]
class FCarlaEngine : private NonCopyable// 继承自NonCopyable，禁止复制
{
public:
    // [静态成员变量]
  static uint64_t FrameCounter;// 静态成员变量，用于记录帧数
  // [析构函数]
  ~FCarlaEngine();
  // [通知函数]
  void NotifyInitGame(const UCarlaSettings &Settings);// 通知初始化游戏 

  void NotifyBeginEpisode(UCarlaEpisode &Episode);// 通知开始新的剧情 

  void NotifyEndEpisode();// 通知结束当前剧情
  // [获取服务器]
  const FCarlaServer &GetServer() const// 获取常量引用形式的服务器
  {
    return Server;
  }

  FCarlaServer &GetServer()// 获取非常量引用形式的服务器
  {
    return Server;
  }
  // [获取当前剧情]
  UCarlaEpisode *GetCurrentEpisode()
  {
    return CurrentEpisode;
  }
  // [设置录像机] 
  void SetRecorder(ACarlaRecorder *InRecorder)
  {
    Recorder = InRecorder;
  }
  // [帧计数器相关函数] 
  static uint64_t GetFrameCounter()// 获取当前帧数
  {
    return FCarlaEngine::FrameCounter;
  }
  
  static uint64_t UpdateFrameCounter()// 更新帧数，并返回更新后的帧数
  {
    FCarlaEngine::FrameCounter += 1;
    #if defined(WITH_ROS2)
    auto ROS2 = carla::ros2::ROS2::GetInstance();
    if (ROS2->IsEnabled())
      ROS2->SetFrame(FCarlaEngine::FrameCounter);
    #endif
    return FCarlaEngine::FrameCounter;
  }

  static void ResetFrameCounter(uint64_t Value = 0)// 重置帧数  
  {
    FCarlaEngine::FrameCounter = Value;
    #if defined(WITH_ROS2)
    auto ROS2 = carla::ros2::ROS2::GetInstance();
    if (ROS2->IsEnabled())
      ROS2->SetFrame(FCarlaEngine::FrameCounter);
    #endif
  }
  // [获取次级服务器]
  std::shared_ptr<carla::multigpu::Router> GetSecondaryServer()// 获取次级服务器的共享指针
  {
    return SecondaryServer;
  }

private:

  void OnPreTick(UWorld *World, ELevelTick TickType, float DeltaSeconds);

  void OnPostTick(UWorld *World, ELevelTick TickType, float DeltaSeconds);

  void OnEpisodeSettingsChanged(const FEpisodeSettings &Settings);

  void ResetSimulationState();

  bool bIsRunning = false;

  bool bSynchronousMode = false;

  bool bMapChanged = false;

  FCarlaServer Server;

  FWorldObserver WorldObserver;

  UCarlaEpisode *CurrentEpisode = nullptr;

  FEpisodeSettings CurrentSettings;

  ACarlaRecorder *Recorder = nullptr;

  FDelegateHandle OnPreTickHandle;

  FDelegateHandle OnPostTickHandle;

  FDelegateHandle OnEpisodeSettingsChangeHandle;

  bool bIsPrimaryServer = true;
  bool bNewConnection = false;

  std::unordered_map<uint32_t, uint32_t> MappedId;

  std::shared_ptr<carla::multigpu::Router>    SecondaryServer;
  std::shared_ptr<carla::multigpu::Secondary> Secondary;

  std::vector<FFrameData> FramesToProcess;
  std::mutex FrameToProcessMutex;
};

// Note: this has a circular dependency with FCarlaEngine; it must be included late.
#include "Sensor/AsyncDataStreamImpl.h"
