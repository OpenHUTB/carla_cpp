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
 // 获取当前的UCarlaEpisode实例
UCarlaEpisode *GetCurrentEpisode()
{
  return CurrentEpisode; // 返回当前Episode的指针
}

// 设置录像机实例
void SetRecorder(ACarlaRecorder *InRecorder)
{
  Recorder = InRecorder; // 将传入的录像机实例赋值给成员变量Recorder
}

// 获取当前帧数的静态函数
static uint64_t GetFrameCounter() // 获取当前帧数
{
  return FCarlaEngine::FrameCounter; // 返回由FCarlaEngine类维护的帧计数器
}

// 更新帧数，并返回更新后的帧数的静态函数
static uint64_t UpdateFrameCounter() // 更新帧数，并返回更新后的帧数
{
  FCarlaEngine::FrameCounter += 1; // 帧计数器加1
  #if defined(WITH_ROS2) // 如果定义了WITH_ROS2宏，表示支持ROS2集成
  auto ROS2 = carla::ros2::ROS2::GetInstance(); // 获取ROS2实例
  if (ROS2->IsEnabled()) // 如果ROS2实例已启用
    ROS2->SetFrame(FCarlaEngine::FrameCounter); // 设置ROS2的当前帧数
  #endif
  return FCarlaEngine::FrameCounter; // 返回更新后的帧计数器
}

// 重置帧数的静态函数
static void ResetFrameCounter(uint64_t Value = 0) // 重置帧数，默认值为0
{
  FCarlaEngine::FrameCounter = Value; // 将帧计数器设置为传入的值
  #if defined(WITH_ROS2) // 如果定义了WITH_ROS2宏，表示支持ROS2集成
  auto ROS2 = carla::ros2::ROS2::GetInstance(); // 获取ROS2实例
  if (ROS2->IsEnabled()) // 如果ROS2实例已启用
    ROS2->SetFrame(FCarlaEngine::FrameCounter); // 设置ROS2的当前帧数
  #endif
}

// 获取次级服务器的共享指针
std::shared_ptr<carla::multigpu::Router> GetSecondaryServer() // 获取次级服务器的共享指针
{
  return SecondaryServer; // 返回次级服务器的共享指针
}

private:

// 在每个Tick之前调用的函数
void OnPreTick(UWorld *World, ELevelTick TickType, float DeltaSeconds);

// 在每个Tick之后调用的函数
void OnPostTick(UWorld *World, ELevelTick TickType, float DeltaSeconds);

// 当Episode设置发生变化时调用的函数
void OnEpisodeSettingsChanged(const FEpisodeSettings &Settings);

// 重置仿真状态的函数
void ResetSimulationState();

bool bIsRunning = false; // 标识仿真是否正在运行

bool bSynchronousMode = false; // 标识是否处于同步模式

bool bMapChanged = false; // 标识地图是否已更改

FCarlaServer Server; // CARLA服务器实例

FWorldObserver WorldObserver; // 世界观察者实例

UCarlaEpisode *CurrentEpisode = nullptr; // 当前Episode的指针

FEpisodeSettings CurrentSettings; // 当前Episode设置

ACarlaRecorder *Recorder = nullptr; // 录像机实例

FDelegateHandle OnPreTickHandle; // OnPreTick事件的委托句柄

FDelegateHandle OnPostTickHandle; // OnPostTick事件的委托句柄

FDelegateHandle OnEpisodeSettingsChangeHandle; // Episode设置更改事件的委托句柄

bool bIsPrimaryServer = true; // 标识是否为主服务器

bool bNewConnection = false; // 标识是否有新的连接

std::unordered_map<uint32_t, uint32_t> MappedId; // 用于映射ID的哈希表

std::shared_ptr<carla::multigpu::Router> SecondaryServer; // 次级服务器的共享指针
std::shared_ptr<carla::multigpu::Secondary> Secondary; // 次级实例的共享指针

std::vector<FFrameData> FramesToProcess; // 待处理帧数据的向量
std::mutex FrameToProcessMutex; // 帧数据处理的互斥锁
};

// Note: this has a circular dependency with FCarlaEngine; it must be included late.
#include "Sensor/AsyncDataStreamImpl.h"
