// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once// [����ָ��] ȷ��ͷ�ļ�ֻ������һ�� 
// [������Ҫ��ͷ�ļ�] 
#include "Carla/Recorder/CarlaRecorder.h"// ����CarlaRecorder��Ķ���
#include "Carla/Sensor/WorldObserver.h"// ����WorldObserver��Ķ���
#include "Carla/Server/CarlaServer.h"// ����CarlaServer��Ķ���  
#include "Carla/Settings/EpisodeSettings.h"// ����EpisodeSettings��ض���
#include "Carla/Util/NonCopyable.h"// ����NonCopyable��Ķ��壬���ڽ�ֹ����
#include "Carla/Game/FrameData.h"// ����FrameData�ṹ��Ķ���

#include "Misc/CoreDelegates.h" // ��������ί�еĶ���
// [����/����UE4��]
#include <compiler/disable-ue4-macros.h>// ����Unreal Engine 4�ĺ�
#include <carla/multigpu/router.h>// ������GPU��ROS2��ص�Carla��ͷ�ļ� 
#include <carla/multigpu/primaryCommands.h>
#include <carla/multigpu/secondary.h>
#include <carla/multigpu/secondaryCommands.h>
#include <carla/ros2/ROS2.h>
#include <compiler/enable-ue4-macros.h>// ��������Unreal Engine 4�ĺ�

#include <mutex>// ����C++��׼���mutex�࣬�����߳�ͬ��
// [ǰ������]
class UCarlaSettings;// ǰ������UCarlaSettings�� 
struct FEpisodeSettings;// ǰ������FEpisodeSettings�ṹ��
// [FCarlaEngine�ඨ��]
class FCarlaEngine : private NonCopyable// �̳���NonCopyable����ֹ����
{
public:
    // [��̬��Ա����]
  static uint64_t FrameCounter;// ��̬��Ա���������ڼ�¼֡��
  // [��������]
  ~FCarlaEngine();
  // [֪ͨ����]
  void NotifyInitGame(const UCarlaSettings &Settings);// ֪ͨ��ʼ����Ϸ 

  void NotifyBeginEpisode(UCarlaEpisode &Episode);// ֪ͨ��ʼ�µľ��� 

  void NotifyEndEpisode();// ֪ͨ������ǰ����
  // [��ȡ������]
  const FCarlaServer &GetServer() const// ��ȡ����������ʽ�ķ�����
  {
    return Server;
  }

  FCarlaServer &GetServer()// ��ȡ�ǳ���������ʽ�ķ�����
  {
    return Server;
  }
  // [��ȡ��ǰ����]
  UCarlaEpisode *GetCurrentEpisode()
  {
    return CurrentEpisode;
  }
  // [����¼���] 
  void SetRecorder(ACarlaRecorder *InRecorder)
  {
    Recorder = InRecorder;
  }
  // [֡��������غ���] 
  static uint64_t GetFrameCounter()// ��ȡ��ǰ֡��
  {
    return FCarlaEngine::FrameCounter;
  }
  
  static uint64_t UpdateFrameCounter()// ����֡���������ظ��º��֡��
  {
    FCarlaEngine::FrameCounter += 1;
    #if defined(WITH_ROS2)
    auto ROS2 = carla::ros2::ROS2::GetInstance();
    if (ROS2->IsEnabled())
      ROS2->SetFrame(FCarlaEngine::FrameCounter);
    #endif
    return FCarlaEngine::FrameCounter;
  }

  static void ResetFrameCounter(uint64_t Value = 0)// ����֡��  
  {
    FCarlaEngine::FrameCounter = Value;
    #if defined(WITH_ROS2)
    auto ROS2 = carla::ros2::ROS2::GetInstance();
    if (ROS2->IsEnabled())
      ROS2->SetFrame(FCarlaEngine::FrameCounter);
    #endif
  }
  // [��ȡ�μ�������]
  std::shared_ptr<carla::multigpu::Router> GetSecondaryServer()// ��ȡ�μ��������Ĺ���ָ��
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
