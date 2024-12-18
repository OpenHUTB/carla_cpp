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
  // �ṹ��
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
    char Id; // ��ʶ����
    uint32_t Size; // ���ݴ�С
  };
  #pragma pack(pop)

public:

  void SetEpisode(UCarlaEpisode* ThisEpisode) {Episode = ThisEpisode;}

  // ��ȡʵ�ʳ����е�֡����
  void GetFrameData(UCarlaEpisode *ThisEpisode, bool bAdditionalData = false, bool bIncludeActorsAgain = false);

  // �ط�֡����
  void PlayFrameData(UCarlaEpisode *ThisEpisode, std::unordered_map<uint32_t, uint32_t>& MappedId);

  // �����ǰ֡����
  void Clear();

  // д��֡����
  void Write(std::ostream& OutStream);
  // ��ȡ֡����
  void Read(std::istream& InStream);

  // ¼�ƹ���
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
  // �����ײ�¼�
  void AddCollision(AActor *Actor1, AActor *Actor2);
  // ���λ����Ϣ
  void AddPosition(const CarlaRecorderPosition &Position);
  // ��ӽ�ͨ��״̬
  void AddState(const CarlaRecorderStateTrafficLight &State);
  // ��Ӷ�������Ϣ
  void AddAnimVehicle(const CarlaRecorderAnimVehicle &Vehicle);
  // ������Ӷ�����Ϣ
  void AddAnimVehicleWheels(const CarlaRecorderAnimWheels &VehicleWheels);
  // ��Ӳ����߶���
  void AddAnimWalker(const CarlaRecorderAnimWalker &Walker);
  // ������г�����
  void AddAnimBiker(const CarlaRecorderAnimBiker &Biker);
  // ��ӳ�����״̬
  void AddLightVehicle(const CarlaRecorderLightVehicle &LightVehicle);
  // ��ӳ�����״̬�仯�¼�
  void AddEventLightSceneChanged(const UCarlaLight* Light);
  // ��Ӷ���ѧ����
  void AddKinematics(const CarlaRecorderKinematics &ActorKinematics);
  // ��Ӽ��ӿ���Ϣ
  void AddBoundingBox(const CarlaRecorderActorBoundingBox &ActorBoundingBox);
  // ��Ӵ�������
  void AddTriggerVolume(const ATrafficSignBase &TrafficSign);
  // ������������Ϣ
  void AddPhysicsControl(const ACarlaWheeledVehicle& Vehicle);
  // ��ӽ�ͨ��ʱ��
  void AddTrafficLightTime(const ATrafficLightBase& TrafficLight);

  // �������ʵ���λ����Ϣ
  void AddActorPosition(FCarlaActor *CarlaActor);
  // ��Ӳ����߶���
  void AddWalkerAnimation(FCarlaActor *CarlaActor);
  // ������г�����
  void AddBikerAnimation(FCarlaActor *CarlaActor);
  // ��ӳ�������
  void AddVehicleAnimation(FCarlaActor *CarlaActor);
  // ��ӳ������Ӷ���
  void AddVehicleWheelsAnimation(FCarlaActor *CarlaActor);
  // ��ӽ�ͨ��״̬
  void AddTrafficLightState(FCarlaActor *CarlaActor);
  // ��ӳ�����״̬
  void AddVehicleLight(FCarlaActor *CarlaActor);
  // ��Ӷ���ѧ����
  void AddActorKinematics(FCarlaActor *CarlaActor);
  // ��Ӽ��ӿ���Ϣ
  void AddActorBoundingBox(FCarlaActor *CarlaActor);

  // ��ȡ֡������
  void GetFrameCounter();

  // ����������ʵ��
  std::pair<int, FCarlaActor*> CreateOrReuseActor(
      FVector &Location,
      FVector &Rotation,
      FActorDescription &ActorDesc,
      uint32_t DesiredId,
      bool SpawnSensors,
      std::unordered_map<uint32_t, uint32_t>& MappedId);

    // �ط�ʵ��Ĵ����¼�
  std::pair<int, uint32_t> ProcessReplayerEventAdd(
      FVector Location,
      FVector Rotation,
      CarlaRecorderActorDescription Description,
      uint32_t DesiredId,
      bool bIgnoreHero,
      bool ReplaySensors,
      std::unordered_map<uint32_t, uint32_t>& MappedId);

  // �ط�ʵ���ɾ���¼�
  bool ProcessReplayerEventDel(uint32_t DatabaseId);
  // �ط�ʵ��ĸ��ӹ�ϵ�¼�
  bool ProcessReplayerEventParent(uint32_t ChildId, uint32_t ParentId);
  // ����ʵ��λ��
  bool ProcessReplayerPosition(CarlaRecorderPosition Pos1, 
                               CarlaRecorderPosition Pos2, double Per, double DeltaTime);
  // �طŽ�ͨ��״̬
  bool ProcessReplayerStateTrafficLight(CarlaRecorderStateTrafficLight State);
  // ���ó�������
  void ProcessReplayerAnimVehicle(CarlaRecorderAnimVehicle Vehicle);
  // ���ò����߶���
  void ProcessReplayerAnimWalker(CarlaRecorderAnimWalker Walker);
  // ���ó������Ӷ���
  void ProcessReplayerAnimVehicleWheels(CarlaRecorderAnimWheels Vehicle);
  // �������г�����
  void ProcessReplayerAnimBiker(CarlaRecorderAnimBiker Biker);
  // ���ó�����״̬
  void ProcessReplayerLightVehicle(CarlaRecorderLightVehicle LightVehicle);
  // ���ó�����
  void ProcessReplayerLightScene(CarlaRecorderLightScene LightScene);
  // �طŽ���
  bool ProcessReplayerFinish(bool bApplyAutopilot, bool bIgnoreHero, 
                             std::unordered_map<uint32_t, bool> &IsHero);
  // ���������λ���Ը�����Ա
  bool SetCameraPosition(uint32_t Id, FVector Offset, FQuat Rotation);
  // ����ʵ����ٶ�
  void SetActorVelocity(FCarlaActor *CarlaActor, FVector Velocity);
  // ���ò����ߵĶ����ٶ�
  void SetWalkerSpeed(uint32_t ActorId, float Speed);
  // ����/����ʵ�������ģ��
  bool SetActorSimulatePhysics(FCarlaActor *CarlaActor, bool bEnabled);

  void SetFrameCounter();

  // �ҵ��ض�λ�õĽ�ͨ��
  FCarlaActor* FindTrafficLightAt(FVector Location);

  // ����Ѵ�ʵ��
  void AddExistingActors(void);

  UCarlaEpisode *Episode;
};
