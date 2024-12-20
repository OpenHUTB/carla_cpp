// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <fstream>

#include "CarlaRecorderTraficLightTime.h"
#include "CarlaRecorderPhysicsControl.h"
#include "CarlaRecorderPlatformTime.h"
#include "CarlaRecorderBoundingBox.h"
#include "CarlaRecorderKinematics.h"
#include "CarlaRecorderLightScene.h"
#include "CarlaRecorderLightVehicle.h"
#include "CarlaRecorderAnimWalker.h"
#include "CarlaRecorderAnimVehicle.h"
#include "CarlaRecorderCollision.h"
#include "CarlaRecorderEventAdd.h"
#include "CarlaRecorderEventDel.h"
#include "CarlaRecorderEventParent.h"
#include "CarlaRecorderFrames.h"
#include "CarlaRecorderInfo.h"
#include "CarlaRecorderPosition.h"
#include "CarlaRecorderState.h"
#include "CarlaRecorderWalkerBones.h"
#include "CarlaRecorderDoorVehicle.h"

class CarlaRecorderQuery
{

  #pragma pack(push, 1)
  struct Header
  {
    char Id;
    uint32_t Size;
  };
  #pragma pack(pop)

public:

  // 获取通用信息
  std::string QueryInfo(std::string Filename, bool bShowAll = false);
  //获取碰撞信息
  std::string QueryCollisions(std::string Filename, char Category1 = 'a', char Category2 = 'a');
  //  获取被阻塞的演员信息
  std::string QueryBlocked(std::string Filename, double MinTime = 30, double MinDistance = 10);

private:

  std::ifstream File;
  Header Header;
  CarlaRecorderInfo RecInfo;
  CarlaRecorderFrame Frame;
  CarlaRecorderEventAdd EventAdd;
  CarlaRecorderEventDel EventDel;
  CarlaRecorderEventParent EventParent;
  CarlaRecorderPosition Position;
  CarlaRecorderCollision Collision;
  CarlaRecorderStateTrafficLight StateTraffic;
  CarlaRecorderAnimVehicle Vehicle;
  CarlaRecorderAnimWalker Walker;
  CarlaRecorderLightVehicle LightVehicle;
  CarlaRecorderLightScene LightScene;
  CarlaRecorderKinematics Kinematics;
  CarlaRecorderActorBoundingBox ActorBoundingBox;
  CarlaRecorderPlatformTime PlatformTime;
  CarlaRecorderPhysicsControl PhysicsControl;
  CarlaRecorderTrafficLightTime TrafficLightTime;
  CarlaRecorderWalkerBones WalkerBones;
  CarlaRecorderDoorVehicle DoorVehicle;

  // 读取下一个数据包头部
  bool ReadHeader(void);

  //跳过当前数据包
  void SkipPacket(void);

  //  读取开始信息结构并检查魔法字符串
  bool CheckFileInfo(std::stringstream &Info);
};
