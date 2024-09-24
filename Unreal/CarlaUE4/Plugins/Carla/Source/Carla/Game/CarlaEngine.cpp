// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/Game/CarlaEngine.h"

#include "Carla/Game/CarlaEpisode.h"
#include "Carla/Game/CarlaStaticDelegates.h"
#include "Carla/Game/CarlaStatics.h"
#include "Carla/Lights/CarlaLightSubsystem.h"
#include "Carla/Recorder/CarlaRecorder.h"
#include "Carla/Settings/CarlaSettings.h"
#include "Carla/Settings/EpisodeSettings.h"

#include "Runtime/Core/Public/Misc/App.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Carla/MapGen/LargeMapManager.h"

#include <compiler/disable-ue4-macros.h>
#include <carla/Logging.h>
#include <carla/multigpu/primaryCommands.h>
#include <carla/multigpu/commands.h>
#include <carla/multigpu/secondary.h>
#include <carla/multigpu/secondaryCommands.h>
#include <carla/ros2/ROS2.h>
#include <carla/streaming/EndPoint.h>
#include <carla/streaming/Server.h>
#include <compiler/enable-ue4-macros.h>

#include <thread>

// =============================================================================
// -- Static local methods -----------------------------------------------------
// =============================================================================

// 初始化静态变量
uint64_t FCarlaEngine::FrameCounter = 0;

static uint32 FCarlaEngine_GetNumberOfThreadsForRPCServer()
{
  return std::max(std::thread::hardware_concurrency(), 4u) - 2u;
}

static TOptional<double> FCarlaEngine_GetFixedDeltaSeconds()
{
  return FApp::IsBenchmarking() ? FApp::GetFixedDeltaTime() : TOptional<double>{};
}

static void FCarlaEngine_SetFixedDeltaSeconds(TOptional<double> FixedDeltaSeconds)
{
  FApp::SetBenchmarking(FixedDeltaSeconds.IsSet());
  FApp::SetFixedDeltaTime(FixedDeltaSeconds.Get(0.0));
}

// =============================================================================
// -- FCarlaEngine -------------------------------------------------------------
// =============================================================================

FCarlaEngine::~FCarlaEngine()
{
  if (bIsRunning)
  {
    #if defined(WITH_ROS2)
    auto ROS2 = carla::ros2::ROS2::GetInstance();
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
