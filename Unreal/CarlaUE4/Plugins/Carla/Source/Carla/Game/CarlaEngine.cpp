// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h" // 包含CARLA的主头文件，提供CARLA基本功能和接口
#include "Carla/Game/CarlaEngine.h" // 包含CARLA游戏引擎的头文件，管理游戏世界和仿真

#include "Carla/Game/CarlaEpisode.h" // 包含CARLA游戏环节的头文件，代表一个仿真会话
#include "Carla/Game/CarlaStaticDelegates.h" // 包含CARLA静态委托的头文件，用于注册和管理事件
#include "Carla/Game/CarlaStatics.h" // 包含CARLA静态变量和函数的头文件，提供全局访问点
#include "Carla/Lights/CarlaLightSubsystem.h" // 包含CARLA灯光子系统的头文件，管理游戏世界的灯光
#include "Carla/Recorder/CarlaRecorder.h" // 包含CARLA录制器的头文件，用于记录和回放仿真
#include "Carla/Settings/CarlaSettings.h" // 包含CARLA设置的头文件，定义仿真的配置参数
#include "Carla/Settings/EpisodeSettings.h" // 包含CARLA游戏环节设置的头文件，定义环节特定的配置

#include "Runtime/Core/Public/Misc/App.h" // 包含Unreal Engine应用框架的头文件，提供应用程序接口
#include "PhysicsEngine/PhysicsSettings.h" // 包含物理引擎设置的头文件，定义物理仿真参数
#include "Carla/MapGen/LargeMapManager.h" // 包含CARLA大地图管理器的头文件，管理大型开放世界地图

#include <compiler/disable-ue4-macros.h> // 禁用Unreal Engine的宏，防止与CARLA代码冲突
#include <carla/Logging.h> // 包含CARLA日志系统的头文件，提供日志记录功能
#include <carla/multigpu/primaryCommands.h> // 包含CARLA多GPU主命令的头文件，用于跨GPU通信
#include <carla/multigpu/commands.h> // 包含CARLA多GPU命令的头文件，用于跨GPU通信
#include <carla/multigpu/secondary.h> // 包含CARLA多GPU次要功能的头文件，用于跨GPU通信
#include <carla/multigpu/secondaryCommands.h> // 包含CARLA多GPU次要命令的头文件，用于跨GPU通信
#include <carla/ros2/ROS2.h> // 包含CARLA ROS 2接口的头文件，提供ROS 2集成功能
#include <carla/streaming/EndPoint.h> // 包含CARLA流媒体端点的头文件，提供流媒体传输功能
#include <carla/streaming/Server.h> // 包含CARLA流媒体服务器的头文件，提供流媒体传输服务
#include <compiler/enable-ue4-macros.h> // 启用Unreal Engine的宏

#include <thread> // 包含C++标准库线程的头文件，提供线程管理功能

// =============================================================================
// -- Static local methods -----------------------------------------------------
// =============================================================================

// 初始化静态变量，用于计数帧数
uint64_t FCarlaEngine::FrameCounter = 0;

// 静态函数，用于获取RPC服务器的线程数
static uint32 FCarlaEngine_GetNumberOfThreadsForRPCServer()
{
  // 返回硬件支持的线程数与4的最大值，然后减去2，确保有足够的线程处理其他任务
  return std::max(std::thread::hardware_concurrency(), 4u) - 2u;
}

// 静态函数，用于获取固定的仿真时间间隔
static TOptional<double> FCarlaEngine_GetFixedDeltaSeconds()
{
  // 如果正在基准测试，则返回固定的仿真时间间隔，否则返回空值
  return FApp::IsBenchmarking() ? FApp::GetFixedDeltaTime() : TOptional<double>{};
}

// 静态函数，用于设置固定的仿真时间间隔
static void FCarlaEngine_SetFixedDeltaSeconds(TOptional<double> FixedDeltaSeconds)
{
  // 如果提供了固定的时间间隔，则设置基准测试模式和固定的时间间隔，否则不进行设置
  FApp::SetBenchmarking(FixedDeltaSeconds.IsSet());
  FApp::SetFixedDeltaTime(FixedDeltaSeconds.Get(0.0));
}

// =============================================================================
// -- FCarlaEngine -------------------------------------------------------------
// =============================================================================

// FCarlaEngine类的析构函数，负责清理资源和结束仿真
FCarlaEngine::~FCarlaEngine()
{
   // 检查成员变量bIsRunning是否为true，表示引擎是否正在运行
   // 如果正在运行，则需要执行清理操作，例如结束仿真、释放资源等
}

  if (bIsRunning)
  {
    // 如果定义了WITH_ROS2宏，表示项目配置了ROS2（Robot Operating System 2）支持
    #if defined(WITH_ROS2)
    auto ROS2 = carla::ros2::ROS2::GetInstance();
    // 检查ROS2是否已启用
    if (ROS2->IsEnabled())
      ROS2->Shutdown();
    #endif
    FWorldDelegates::OnWorldTickStart.Remove(OnPreTickHandle);
    FWorldDelegates::OnWorldPostActorTick.Remove(OnPostTickHandle);
    FCarlaStaticDelegates::OnEpisodeSettingsChange.Remove(OnEpisodeSettingsChangeHandle);
  }
}

void FCarlaEngine::NotifyInitGame(const UCarlaSettings &Settings)
{
  TRACE_CPUPROFILER_EVENT_SCOPE_STR(__FUNCTION__);
  if (!bIsRunning)
  {
    const auto StreamingPort = Settings.StreamingPort;
    const auto SecondaryPort = Settings.SecondaryPort;
    const auto PrimaryIP     = Settings.PrimaryIP;
    const auto PrimaryPort   = Settings.PrimaryPort;

    auto BroadcastStream     = Server.Start(Settings.RPCPort, StreamingPort, SecondaryPort);
    Server.AsyncRun(FCarlaEngine_GetNumberOfThreadsForRPCServer());

    WorldObserver.SetStream(BroadcastStream);

    OnPreTickHandle = FWorldDelegates::OnWorldTickStart.AddRaw(
        this,
        &FCarlaEngine::OnPreTick);
    OnPostTickHandle = FWorldDelegates::OnWorldPostActorTick.AddRaw(
        this,
        &FCarlaEngine::OnPostTick);
    OnEpisodeSettingsChangeHandle = FCarlaStaticDelegates::OnEpisodeSettingsChange.AddRaw(
        this,
        &FCarlaEngine::OnEpisodeSettingsChanged);

    bIsRunning = true;

    // 将此作为备用服务器转换进行检查
    if (!PrimaryIP.empty())
    {
      // 我们是备用服务器，正在连接到主服务器
      bIsPrimaryServer = false;

      // 当命令来自主服务器时，命令执行器是指负责处理和执行这些命令的组件或模块
      auto CommandExecutor = [=](carla::multigpu::MultiGPUCommand Id, carla::Buffer Data) {
        struct CarlaStreamBuffer : public std::streambuf
        {
            CarlaStreamBuffer(char *buf, std::size_t size) { setg(buf, buf, buf + size); }
        };
        switch (Id) {
          case carla::multigpu::MultiGPUCommand::SEND_FRAME:
          {
            if(GetCurrentEpisode())
            {
              TRACE_CPUPROFILER_EVENT_SCOPE_STR("MultiGPUCommand::SEND_FRAME");
              // 将缓冲区中的帧数据转换为输入流
              CarlaStreamBuffer TempStream((char *) Data.data(), Data.size());
              std::istream InStream(&TempStream);
              GetCurrentEpisode()->GetFrameData().Read(InStream);
              {
                TRACE_CPUPROFILER_EVENT_SCOPE_STR("FramesToProcess.emplace_back");
                std::lock_guard<std::mutex> Lock(FrameToProcessMutex);
                FramesToProcess.emplace_back(GetCurrentEpisode()->GetFrameData());
              }
            }
            // 强制进行一次进行单位时间操作
            Server.Tick();
            break;
          }
          case carla::multigpu::MultiGPUCommand::LOAD_MAP:
          {
            FString FinalPath((char *) Data.data());
            if(GetCurrentEpisode())
            {
              UGameplayStatics::OpenLevel(GetCurrentEpisode()->GetWorld(), *FinalPath, true);
            }
            
            break;
          }
          case carla::multigpu::MultiGPUCommand::GET_TOKEN:
          {
            // 获取传感器 ID
            auto sensor_id = *(reinterpret_cast<carla::streaming::detail::stream_id_type *>(Data.data()));
            // 查询调度器
            carla::streaming::detail::token_type token(Server.GetStreamingServer().GetToken(sensor_id));
            carla::Buffer buf(reinterpret_cast<unsigned char *>(&token), (size_t) sizeof(token));
            carla::log_info("responding with a token for port ", token.get_port());
            Secondary->Write(std::move(buf));
            break;
          }
          case carla::multigpu::MultiGPUCommand::YOU_ALIVE:
          {
            std::string msg("Yes, I'm alive");
            carla::Buffer buf((unsigned char *) msg.c_str(), (size_t) msg.size());
            carla::log_info("responding is alive command");
            Secondary->Write(std::move(buf));
            break;
          }
          case carla::multigpu::MultiGPUCommand::ENABLE_ROS:
          {
            // 获取传感器 ID
            auto sensor_id = *(reinterpret_cast<carla::streaming::detail::stream_id_type *>(Data.data()));
            // 查询调度器
            Server.GetStreamingServer().EnableForROS(sensor_id);
            // 返回一个 'true'
            bool res = true;
            carla::Buffer buf(reinterpret_cast<unsigned char *>(&res), (size_t) sizeof(bool));
            carla::log_info("responding ENABLE_ROS with a true");
            Secondary->Write(std::move(buf));
            break;
          }
          case carla::multigpu::MultiGPUCommand::DISABLE_ROS:
          {
            // 获取传感器 ID
            auto sensor_id = *(reinterpret_cast<carla::streaming::detail::stream_id_type *>(Data.data()));
            // 查询调度器
            Server.GetStreamingServer().DisableForROS(sensor_id);
            // 返回一个 'true'
            bool res = true;
            carla::Buffer buf(reinterpret_cast<unsigned char *>(&res), (size_t) sizeof(bool));
            carla::log_info("responding DISABLE_ROS with a true");
            Secondary->Write(std::move(buf));
            break;
          }
          case carla::multigpu::MultiGPUCommand::IS_ENABLED_ROS:
          {
            // 获取传感器 ID
            auto sensor_id = *(reinterpret_cast<carla::streaming::detail::stream_id_type *>(Data.data()));
            // 查询调度器
            bool res = Server.GetStreamingServer().IsEnabledForROS(sensor_id);
            carla::Buffer buf(reinterpret_cast<unsigned char *>(&res), (size_t) sizeof(bool));
            carla::log_info("responding IS_ENABLED_ROS with: ", res);
            Secondary->Write(std::move(buf));
            break;
          }
        }
      };

      Secondary = std::make_shared<carla::multigpu::Secondary>(PrimaryIP, PrimaryPort, CommandExecutor);
      Secondary->Connect();
      // 将此服务器设置为同步模式
      bSynchronousMode = true;
    }
    else
    {
      // 我们是主服务器，正在启动服务器
      bIsPrimaryServer = true;
      SecondaryServer = Server.GetSecondaryServer();
      SecondaryServer->SetNewConnectionCallback([this]()
      {
        this->bNewConnection = true;
        UE_LOG(LogCarla, Log, TEXT("New secondary connection detected"));
      });
    }
  }

  //创建 ROS2 管理器
  #if defined(WITH_ROS2)
  if (Settings.ROS2)
  {
    auto ROS2 = carla::ros2::ROS2::GetInstance();
    ROS2->Enable(true);
  }
  #endif

  bMapChanged = true;
}

void FCarlaEngine::NotifyBeginEpisode(UCarlaEpisode &Episode)
{
  TRACE_CPUPROFILER_EVENT_SCOPE_STR(__FUNCTION__);
  Episode.EpisodeSettings.FixedDeltaSeconds = FCarlaEngine_GetFixedDeltaSeconds();
  CurrentEpisode = &Episode;

  // 重置地图设置
  UWorld* World = CurrentEpisode->GetWorld();
  ALargeMapManager* LargeMapManager = UCarlaStatics::GetLargeMapManager(World);
  if (LargeMapManager)
  {
    CurrentSettings.TileStreamingDistance = LargeMapManager->GetLayerStreamingDistance();
    CurrentSettings.ActorActiveDistance = LargeMapManager->GetActorStreamingDistance();
  }

  if (!bIsPrimaryServer)
  {
    // 将此次要服务器设置为无渲染模式
    CurrentSettings.bNoRenderingMode = true;
  }

  CurrentEpisode->ApplySettings(CurrentSettings);

  ResetFrameCounter(GFrameNumber);

  // 建立Episode和Recorder之间的连接
  if (Recorder)
  {
    Recorder->SetEpisode(&Episode);
    Episode.SetRecorder(Recorder);
    Recorder->GetReplayer()->CheckPlayAfterMapLoaded();
  }

  Server.NotifyBeginEpisode(Episode);

  Episode.bIsPrimaryServer = bIsPrimaryServer;
}

void FCarlaEngine::NotifyEndEpisode()
{
  Server.NotifyEndEpisode();
  CurrentEpisode = nullptr;
}

void FCarlaEngine::OnPreTick(UWorld *, ELevelTick TickType, float DeltaSeconds)
{
  TRACE_CPUPROFILER_EVENT_SCOPE_STR(__FUNCTION__);
  if (TickType == ELevelTick::LEVELTICK_All)
  {

    if (bIsPrimaryServer)
    {
      if (CurrentEpisode && !bSynchronousMode && SecondaryServer->HasClientsConnected())
      {
        // 设置为同步模式。
        CurrentSettings.bSynchronousMode = true;
        CurrentSettings.FixedDeltaSeconds = 1 / 20.0f;
        OnEpisodeSettingsChanged(CurrentSettings);
        CurrentEpisode->ApplySettings(CurrentSettings);
      }

      // 处理RPC命令
      do
      {
        Server.RunSome(1u);
      }
      while (bSynchronousMode && !Server.TickCueReceived());
    }
    else
    {
      // 处理帧数据
      do
      {
        Server.RunSome(1u);
      }
      while (!FramesToProcess.size());
    }

    // 更新帧计数器
    UpdateFrameCounter();

    if (CurrentEpisode)
    {
      CurrentEpisode->TickTimers(DeltaSeconds);

      if (!bIsPrimaryServer)
      {
        if (FramesToProcess.size())
        {
          TRACE_CPUPROFILER_EVENT_SCOPE_STR("FramesToProcess.PlayFrameData");
          std::lock_guard<std::mutex> Lock(FrameToProcessMutex);
          FramesToProcess.front().PlayFrameData(CurrentEpisode, MappedId);
          FramesToProcess.erase(FramesToProcess.begin()); // 移除第一个元素
        }
      }
    }
  }
}


void FCarlaEngine::OnPostTick(UWorld *World, ELevelTick TickType, float DeltaSeconds)
{
  TRACE_CPUPROFILER_EVENT_SCOPE_STR(__FUNCTION__);
  // 标记录制/回放系统
  if (GetCurrentEpisode())
  {
    if (bIsPrimaryServer)
    {
      if (SecondaryServer->HasClientsConnected()) {
        GetCurrentEpisode()->GetFrameData().GetFrameData(GetCurrentEpisode(), true, bNewConnection);
        bNewConnection = false;
        std::ostringstream OutStream;
        GetCurrentEpisode()->GetFrameData().Write(OutStream);

        // 将帧数据发送到次级服务器
        std::string Tmp(OutStream.str());
        SecondaryServer->GetCommander().SendFrameData(carla::Buffer(std::move((unsigned char *) Tmp.c_str()), (size_t) Tmp.size()));

        GetCurrentEpisode()->GetFrameData().Clear();
      }
    }

    auto* EpisodeRecorder = GetCurrentEpisode()->GetRecorder();
    if (EpisodeRecorder)
    {
      EpisodeRecorder->Ticking(DeltaSeconds);
    }
  }

  if ((TickType == ELevelTick::LEVELTICK_All) && (CurrentEpisode != nullptr))
  {
    // 查找光子系统
    bool LightUpdatePending = false;
    if (World)
    {
      UCarlaLightSubsystem* CarlaLightSubsystem = World->GetSubsystem<UCarlaLightSubsystem>();
      if (CarlaLightSubsystem)
      {
        LightUpdatePending = CarlaLightSubsystem->IsUpdatePending();
      }
    }

    // 发送 worldsnapshot
    //worldsnapshot:
    //1·游戏开发：在游戏中，"world snapshot" 可以用来记录游戏的状态，保存玩家的位置、状态、物品等信息，以便后续恢复。
    //2·虚拟现实和增强现实：在这些环境中，世界快照可以帮助记录用户的位置和交互，便于分析和重现体验。
    WorldObserver.BroadcastTick(*CurrentEpisode, DeltaSeconds, bMapChanged, LightUpdatePending);
    CurrentEpisode->GetSensorManager().PostPhysTick(World, TickType, DeltaSeconds);
    ResetSimulationState();
  }
}

void FCarlaEngine::OnEpisodeSettingsChanged(const FEpisodeSettings &Settings)
{
  CurrentSettings = FEpisodeSettings(Settings);

  bSynchronousMode = Settings.bSynchronousMode;

  if (GEngine && GEngine->GameViewport)
  {
    GEngine->GameViewport->bDisableWorldRendering = Settings.bNoRenderingMode;
  }

  FCarlaEngine_SetFixedDeltaSeconds(Settings.FixedDeltaSeconds);

  // 为物理子步设置参数
  UPhysicsSettings* PhysSett = UPhysicsSettings::Get();
  PhysSett->bSubstepping = Settings.bSubstepping;
  PhysSett->MaxSubstepDeltaTime = Settings.MaxSubstepDeltaTime;
  PhysSett->MaxSubsteps = Settings.MaxSubsteps;

  UWorld* World = CurrentEpisode->GetWorld();
  ALargeMapManager* LargeMapManager = UCarlaStatics::GetLargeMapManager(World);
  if (LargeMapManager)
  {
    LargeMapManager->SetLayerStreamingDistance(Settings.TileStreamingDistance);
    LargeMapManager->SetActorStreamingDistance(Settings.ActorActiveDistance);
  }
}

void FCarlaEngine::ResetSimulationState()
{
  bMapChanged = false;
}
