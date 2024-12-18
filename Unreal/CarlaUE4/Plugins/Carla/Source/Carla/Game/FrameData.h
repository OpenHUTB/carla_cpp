/ Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Recorder/CarlaRecorderTraficLightTime.h"
#include "Carla/Recorder/CarlaRecorderPhysicsControl.h"
#include "Carla/Recorder/CarlaRecorderPlatformTime.h"
#include "Carla/Recorder/CarlaRecorderBoundingBox.h"
#include "Carla/Recorder/CarlaRecorderKinematics.h"
#include "Carla/Recorder/CarlaRecorderLightScene.h"
#include "Carla/Recorder/CarlaRecorderLightVehicle.h"
#include "Carla/Recorder/CarlaRecorderAnimVehicle.h"
#include "Carla/Recorder/CarlaRecorderAnimVehicleWheels.h"
#include "Carla/Recorder/CarlaRecorderAnimWalker.h"
#include "Carla/Recorder/CarlaRecorderAnimBiker.h"
#include "Carla/Recorder/CarlaRecorderCollision.h"
#include "Carla/Recorder/CarlaRecorderEventAdd.h"
#include "Carla/Recorder/CarlaRecorderEventDel.h"
#include "Carla/Recorder/CarlaRecorderEventParent.h"
#include "Carla/Recorder/CarlaRecorderFrames.h"
#include "Carla/Recorder/CarlaRecorderInfo.h"
#include "Carla/Recorder/CarlaRecorderPosition.h"
#include "Carla/Recorder/CarlaRecorderFrameCounter.h"
#include "Carla/Recorder/CarlaRecorderState.h"
#include "Carla/Actor/ActorDescription.h"
#include "Carla/Lights/CarlaLight.h"
#include "Carla/Traffic/TrafficLightBase.h"
#include "Carla/Traffic/TrafficSignBase.h"


#include <sstream>
#include <unordered_map>

class UCarlaEpisode;
class FCarlaActor;

class FFrameData
{
  // 结构体
  CarlaRecorderInfo Info;
  CarlaRecorderFrames Frames;
  CarlaRecorderEventsAdd EventsAdd;
  CarlaRecorderEventsDel EventsDel;
  CarlaRecorderEventsParent EventsParent;
  CarlaRecorderCollisions Collisions;
  CarlaRecorderPositions Positions;
  CarlaRecorderStates States;
  CarlaRecorderAnimVehicles Vehicles;
  CarlaRecorderAnimVehicleWheels Wheels;
  CarlaRecorderAnimWalkers Walkers;
  CarlaRecorderAnimBikers Bikers;
  CarlaRecorderLightVehicles LightVehicles;
  CarlaRecorderLightScenes LightScenes;
  CarlaRecorderActorsKinematics Kinematics;
  CarlaRecorderActorBoundingBoxes BoundingBoxes;
  CarlaRecorderActorTriggerVolumes TriggerVolumes;
  CarlaRecorderPlatformTime PlatformTime;
  CarlaRecorderPhysicsControls PhysicsControls;
  CarlaRecorderTrafficLightTimes TrafficLightTimes;
  CarlaRecorderFrameCounter FrameCounter;

  #pragma pack(push, 1)
  struct Header
  {
    char Id; // 标识类型
    uint32_t Size; // 数据大小
  };
  #pragma pack(pop)

public:

  void SetEpisode(UCarlaEpisode* ThisEpisode) {Episode = ThisEpisode;}

  // 获取实际场景中的帧数据
  void GetFrameData(UCarlaEpisode *ThisEpisode, bool bAdditionalData = false, bool bIncludeActorsAgain = false);

  // 回放帧数据
  void PlayFrameData(UCarlaEpisode *ThisEpisode, std::unordered_map<uint32_t, uint32_t>& MappedId);

  // 清除当前帧数据
  void Clear();

  // 写入帧数据
  void Write(std::ostream& OutStream);
  // 读取帧数据
  void Read(std::istream& InStream);

  // 录制功能
  void CreateRecorderEventAdd(
      uint32_t DatabaseId,
      uint8_t Type,
      const FTransform &Transform,
      FActorDescription ActorDescription,
      bool bAddOtherRelatedInfo = true);
  void AddEvent(const CarlaRecorderEventAdd &Event);
  void AddEvent(const CarlaRecorderEventDel &Event);
  void AddEvent(const CarlaRecorderEventParent &Event);

private:
  // 添加碰撞事件
  void AddCollision(AActor *Actor1, AActor *Actor2);
  // 添加位置信息
  void AddPosition(const CarlaRecorderPosition &Position);
  // 添加交通灯状态
  void AddState(const CarlaRecorderStateTrafficLight &State);
  // 添加动画车信息
  void AddAnimVehicle(const CarlaRecorderAnimVehicle &Vehicle);
  // 添加轮子动画信息
  void AddAnimVehicleWheels(const CarlaRecorderAnimWheels &VehicleWheels);
  // 添加步行者动画
  void AddAnimWalker(const CarlaRecorderAnimWalker &Walker);
  // 添加自行车动画
  void AddAnimBiker(const CarlaRecorderAnimBiker &Biker);
  // 添加车辆灯状态
  void AddLightVehicle(const CarlaRecorderLightVehicle &LightVehicle);
  // 添加场景灯状态变化事件
  void AddEventLightSceneChanged(const UCarlaLight* Light);
  // 添加动力学数据
  void AddKinematics(const CarlaRecorderKinematics &ActorKinematics);
  // 添加监视框信息
  void AddBoundingBox(const CarlaRecorderActorBoundingBox &ActorBoundingBox);
  // 添加触发区域
  void AddTriggerVolume(const ATrafficSignBase &TrafficSign);
  // 添加物理控制信息
  void AddPhysicsControl(const ACarlaWheeledVehicle& Vehicle);
  // 添加交通灯时间
  void AddTrafficLightTime(const ATrafficLightBase& TrafficLight);

  // 添加现有实体的位置信息
  void AddActorPosition(FCarlaActor *CarlaActor);
  // 添加步行者动画
  void AddWalkerAnimation(FCarlaActor *CarlaActor);
  // 添加自行车动画
  void AddBikerAnimation(FCarlaActor *CarlaActor);
  // 添加车辆动画
  void AddVehicleAnimation(FCarlaActor *CarlaActor);
  // 添加车辆轮子动画
  void AddVehicleWheelsAnimation(FCarlaActor *CarlaActor);
  // 添加交通灯状态
  void AddTrafficLightState(FCarlaActor *CarlaActor);
  // 添加车辆灯状态
  void AddVehicleLight(FCarlaActor *CarlaActor);
  // 添加动力学数据
  void AddActorKinematics(FCarlaActor *CarlaActor);
  // 添加监视框信息
  void AddActorBoundingBox(FCarlaActor *CarlaActor);

  // 获取帧计数器
  void GetFrameCounter();

  // 创建或重用实体
  std::pair<int, FCarlaActor*> CreateOrReuseActor(
      FVector &Location,
      FVector &Rotation,
      FActorDescription &ActorDesc,
      uint32_t DesiredId,
      bool SpawnSensors,
      std::unordered_map<uint32_t, uint32_t>& MappedId);

    // 重放实体的创建事件
  std::pair<int, uint32_t> ProcessReplayerEventAdd(
      FVector Location,
      FVector Rotation,
      CarlaRecorderActorDescription Description,
      uint32_t DesiredId,
      bool bIgnoreHero,
      bool ReplaySensors,
      std::unordered_map<uint32_t, uint32_t>& MappedId);

  // 重放实体的删除事件
  bool ProcessReplayerEventDel(uint32_t DatabaseId);
  // 重放实体的父子关系事件
  bool ProcessReplayerEventParent(uint32_t ChildId, uint32_t ParentId);
  // 重置实体位置
  bool ProcessReplayerPosition(CarlaRecorderPosition Pos1, 
                               CarlaRecorderPosition Pos2, double Per, double DeltaTime);
  // 重放交通灯状态
  bool ProcessReplayerStateTrafficLight(CarlaRecorderStateTrafficLight State);
  // 设置车辆动画
  void ProcessReplayerAnimVehicle(CarlaRecorderAnimVehicle Vehicle);
  // 设置步行者动画
  void ProcessReplayerAnimWalker(CarlaRecorderAnimWalker Walker);
  // 设置车辆轮子动画
  void ProcessReplayerAnimVehicleWheels(CarlaRecorderAnimWheels Vehicle);
  // 设置自行车动画
  void ProcessReplayerAnimBiker(CarlaRecorderAnimBiker Biker);
  // 设置车辆灯状态
  void ProcessReplayerLightVehicle(CarlaRecorderLightVehicle LightVehicle);
  // 设置场景灯
  void ProcessReplayerLightScene(CarlaRecorderLightScene LightScene);
  // 重放结束
  bool ProcessReplayerFinish(bool bApplyAutopilot, bool bIgnoreHero, 
                             std::unordered_map<uint32_t, bool> &IsHero);
  // 设置摄像机位置以跟随演员
  bool SetCameraPosition(uint32_t Id, FVector Offset, FQuat Rotation);
  // 设置实体的速度
  void SetActorVelocity(FCarlaActor *CarlaActor, FVector Velocity);
  // 设置步行者的动画速度
  void SetWalkerSpeed(uint32_t ActorId, float Speed);
  // 启用/禁用实体的物理模拟
  bool SetActorSimulatePhysics(FCarlaActor *CarlaActor, bool bEnabled);

  void SetFrameCounter();

  // 找到特定位置的交通灯
  FCarlaActor* FindTrafficLightAt(FVector Location);

  // 添加已存实体
  void AddExistingActors(void);

  UCarlaEpisode *Episode;
};
