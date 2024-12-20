// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma
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
  // 结构
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
    char Id;
    uint32_t Size;
  };
  #pragma pack(pop)

public:

  void SetEpisode(UCarlaEpisode* ThisEpisode) {Episode = ThisEpisode;}

  void GetFrameData(UCarlaEpisode *ThisEpisode, bool bAdditionalData = false, bool bIncludeActorsAgain = false);

  void PlayFrameData(UCarlaEpisode *ThisEpisode, std::unordered_map<uint32_t, uint32_t>& MappedId);

  void Clear();

  void Write(std::ostream& OutStream);
  void Read(std::istream& InStream);

  // 记录函数
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
  void AddCollision(AActor *Actor1, AActor *Actor2);
  void AddPosition(const CarlaRecorderPosition &Position);
  void AddState(const CarlaRecorderStateTrafficLight &State);
  void AddAnimVehicle(const CarlaRecorderAnimVehicle &Vehicle);
  void AddAnimVehicleWheels(const CarlaRecorderAnimWheels &VehicleWheels);
  void AddAnimWalker(const CarlaRecorderAnimWalker &Walker);
  void AddAnimBiker(const CarlaRecorderAnimBiker &Biker);
  void AddLightVehicle(const CarlaRecorderLightVehicle &LightVehicle);
  void AddEventLightSceneChanged(const UCarlaLight* Light);
  void AddKinematics(const CarlaRecorderKinematics &ActorKinematics);
  void AddBoundingBox(const CarlaRecorderActorBoundingBox &ActorBoundingBox);
  void AddTriggerVolume(const ATrafficSignBase &TrafficSign);
  void AddPhysicsControl(const ACarlaWheeledVehicle& Vehicle);
  void AddTrafficLightTime(const ATrafficLightBase& TrafficLight);

  void AddActorPosition(FCarlaActor *CarlaActor);
  void AddWalkerAnimation(FCarlaActor *CarlaActor);
  void AddBikerAnimation(FCarlaActor *CarlaActor);
  void AddVehicleAnimation(FCarlaActor *CarlaActor);
  void AddVehicleWheelsAnimation(FCarlaActor *CarlaActor);
  void AddTrafficLightState(FCarlaActor *CarlaActor);
  void AddVehicleLight(FCarlaActor *CarlaActor);
  void AddActorKinematics(FCarlaActor *CarlaActor);
  void AddActorBoundingBox(FCarlaActor *CarlaActor);

  void GetFrameCounter();

  std::pair<int, FCarlaActor*> CreateOrReuseActor(
      FVector &Location,
      FVector &Rotation,
      FActorDescription &ActorDesc,
      uint32_t DesiredId,
      bool SpawnSensors,
      std::unordered_map<uint32_t, uint32_t>& MappedId);

    // 用于创建 Actor 的 Replay 事件
  std::pair<int, uint32_t> ProcessReplayerEventAdd(
      FVector Location,
      FVector Rotation,
      CarlaRecorderActorDescription Description,
      uint32_t DesiredId,
      bool bIgnoreHero,
      bool ReplaySensors,
      std::unordered_map<uint32_t, uint32_t>& MappedId);

  // 用于删除 Actor 的 replay 事件
  bool ProcessReplayerEventDel(uint32_t DatabaseId);
  // Replay 事件
  bool ProcessReplayerEventParent(uint32_t ChildId, uint32_t ParentId);
  // 重新定位角色
  bool ProcessReplayerPosition(CarlaRecorderPosition Pos1, 
                               CarlaRecorderPosition Pos2, double Per, double DeltaTime);
  //红绿灯状态的重放事件
  bool ProcessReplayerStateTrafficLight(CarlaRecorderStateTrafficLight State);
  // 为 Vehicles 设置动画
  void ProcessReplayerAnimVehicle(CarlaRecorderAnimVehicle Vehicle);
  // 设置行走者的动画
  void ProcessReplayerAnimWalker(CarlaRecorderAnimWalker Walker);
  // 设置 Vehicles Wheels 的动画
  void ProcessReplayerAnimVehicleWheels(CarlaRecorderAnimWheels Vehicle);
  //为骑自行车的人设置动画
  void ProcessReplayerAnimBiker(CarlaRecorderAnimBiker Biker);
  //设置车灯
  void ProcessReplayerLightVehicle(CarlaRecorderLightVehicle LightVehicle);
  // 设置场景灯光
  void ProcessReplayerLightScene(CarlaRecorderLightScene LightScene);
  // 重播结束
  bool ProcessReplayerFinish(bool bApplyAutopilot, bool bIgnoreHero, 
                             std::unordered_map<uint32_t, bool> &IsHero);
  // 将摄像机位置设置为跟随角色
  bool SetCameraPosition(uint32_t Id, FVector Offset, FQuat Rotation);
  // 设置角色的速度
  void SetActorVelocity(FCarlaActor *CarlaActor, FVector Velocity);
  // 设置行走者的动画速度
  void SetWalkerSpeed(uint32_t ActorId, float Speed);
  // 启用/禁用 Actor 的物理特性）
  bool SetActorSimulatePhysics(FCarlaActor *CarlaActor, bool bEnabled);

  void SetFrameCounter();

  FCarlaActor* FindTrafficLightAt(FVector Location);

  void AddExistingActors(void);

  UCarlaEpisode *Episode;
};
