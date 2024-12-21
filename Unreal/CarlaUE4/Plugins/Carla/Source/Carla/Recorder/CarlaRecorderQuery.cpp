// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "CarlaRecorderQuery.h"

#include "CarlaRecorderHelpers.h"

#include "CarlaRecorder.h"

#include <ctime>
#include <sstream>
#include <string>

#include <compiler/disable-ue4-macros.h>
#include <carla/rpc/VehicleLightState.h>
#include <carla/rpc/VehiclePhysicsControl.h>
#include <compiler/enable-ue4-macros.h>

#include <Carla/Vehicle/CarlaWheeledVehicle.h>

inline bool CarlaRecorderQuery::ReadHeader(void)
{
  if (File.eof())
  {
    return false;
  }

  ReadValue<char>(File, Header.Id);
  ReadValue<uint32_t>(File, Header.Size);

  return true;
}

inline void CarlaRecorderQuery::SkipPacket(void)
{
  File.seekg(Header.Size, std::ios::cur);
}

inline bool CarlaRecorderQuery::CheckFileInfo(std::stringstream &Info)
{
  // 阅读信息
  RecInfo.Read(File);

  // 检查魔术字符串
  if (RecInfo.Magic != "CARLA_RECORDER")
  {
    Info << "File is not a CARLA recorder" << std::endl;
    return false;
  }

  // 显示常规信息
  Info << "Version: " << RecInfo.Version << std::endl;
  Info << "Map: " << TCHAR_TO_UTF8(*RecInfo.Mapfile) << std::endl;
  tm *TimeInfo = localtime(&RecInfo.Date);
  char DateStr[100];
  strftime(DateStr, sizeof(DateStr), "%x %X", TimeInfo);
  Info << "Date: " << DateStr << std::endl << std::endl;

  return true;
}

std::string CarlaRecorderQuery::QueryInfo(std::string Filename, bool bShowAll)
{
  std::stringstream Info;

  // 获取最终路径 + 文件名
  std::string Filename2 = GetRecorderFilename(Filename);

  // 尝试打开
  File.open(Filename2, std::ios::binary);
  if (!File.is_open())
  {
    Info << "File " << Filename2 << " not found on server\n";
    return Info.str();
  }

  uint16_t i, Total;
  bool bFramePrinted = false;

  // 用于重复任务的 Lambda
  auto PrintFrame = [this](std::stringstream &Info)
  {
    Info << "Frame " << Frame.Id << " at " << Frame.Elapsed << " seconds\n";
  };

  if (!CheckFileInfo(Info))
    return Info.str();

  // 仅解析帧
  while (File)
  {
    // 获取标头
    if (!ReadHeader())
    {
      break;
    }

    // 检查帧数据包
    switch (Header.Id)
    {
      // 框架
      case static_cast<char>(CarlaRecorderPacketId::FrameStart):
        Frame.Read(File);
        if (bShowAll)
        {
          PrintFrame(Info);
          bFramePrinted = true;
        }
        else
          bFramePrinted = false;
        break;

      // events add
      case static_cast<char>(CarlaRecorderPacketId::EventAdd):
        ReadValue<uint16_t>(File, Total);
        if (Total > 0 && !bFramePrinted)
        {
          PrintFrame(Info);
          bFramePrinted = true;
        }
        for (i = 0; i < Total; ++i)
        {
          // 添加
          EventAdd.Read(File);
          Info << " Create " << EventAdd.DatabaseId << ": " << TCHAR_TO_UTF8(*EventAdd.Description.Id) <<
            " (" <<
            static_cast<int>(EventAdd.Type) << ") at (" << EventAdd.Location.X << ", " <<
            EventAdd.Location.Y << ", " << EventAdd.Location.Z << ")" << std::endl;
          for (auto &Att : EventAdd.Description.Attributes)
          {
            Info << "  " << TCHAR_TO_UTF8(*Att.Id) << " = " << TCHAR_TO_UTF8(*Att.Value) << std::endl;
          }
        }
        break;

      // 事件删除
      case static_cast<char>(CarlaRecorderPacketId::EventDel):
        ReadValue<uint16_t>(File, Total);
        if (Total > 0 && !bFramePrinted)
        {
          PrintFrame(Info);
          bFramePrinted = true;
        }
        for (i = 0; i < Total; ++i)
        {
          EventDel.Read(File);
          Info << " Destroy " << EventDel.DatabaseId << "\n";
        }
        break;

      // 活动育儿
      case static_cast<char>(CarlaRecorderPacketId::EventParent):
        ReadValue<uint16_t>(File, Total);
        if (Total > 0 && !bFramePrinted)
        {
          PrintFrame(Info);
          bFramePrinted = true;
        }
        for (i = 0; i < Total; ++i)
        {
          EventParent.Read(File);
          Info << " Parenting " << EventParent.DatabaseId << " with " << EventParent.DatabaseIdParent <<
            " (parent)\n";
        }
        break;

      // 碰撞
      case static_cast<char>(CarlaRecorderPacketId::Collision):
        ReadValue<uint16_t>(File, Total);
        if (Total > 0 && !bFramePrinted)
        {
          PrintFrame(Info);
          bFramePrinted = true;
        }
        for (i = 0; i < Total; ++i)
        {
          Collision.Read(File);
          Info << " Collision id " << Collision.Id << " between " << Collision.DatabaseId1;
          if (Collision.IsActor1Hero)
            Info << " (hero) ";
          Info << " with " << Collision.DatabaseId2;
          if (Collision.IsActor2Hero)
            Info << " (hero) ";
          Info << std::endl;
        }
        break;

      // 碰撞
      case static_cast<char>(CarlaRecorderPacketId::Position):
        if (bShowAll)
        {
          ReadValue<uint16_t>(File, Total);
          if (Total > 0 && !bFramePrinted)
          {
            PrintFrame(Info);
            bFramePrinted = true;
          }
          Info << " Positions: " << Total << std::endl;
          for (i = 0; i < Total; ++i)
          {
            Position.Read(File);
            Info << "  Id: " << Position.DatabaseId << " Location: (" << Position.Location.X << ", " << Position.Location.Y << ", " << Position.Location.Z << ") Rotation: (" <<  Position.Rotation.X << ", " << Position.Rotation.Y << ", " << Position.Rotation.Z << ")" << std::endl;
          }
        }
        else
          SkipPacket();
        break;

      // 交通灯
      case static_cast<char>(CarlaRecorderPacketId::State):
        if (bShowAll)
        {
          ReadValue<uint16_t>(File, Total);
          if (Total > 0 && !bFramePrinted)
          {
            PrintFrame(Info);
            bFramePrinted = true;
          }
          Info << " State traffic lights: " << Total << std::endl;
          for (i = 0; i < Total; ++i)
          {
            StateTraffic.Read(File);
            Info << "  Id: " << StateTraffic.DatabaseId << " state: " << static_cast<char>(0x30 + StateTraffic.State) << " frozen: " <<
              StateTraffic.IsFrozen << " elapsedTime: " << StateTraffic.ElapsedTime << std::endl;
          }
        }
        else
          SkipPacket();
        break;

      // 车辆动画
      case static_cast<char>(CarlaRecorderPacketId::AnimVehicle):
        if (bShowAll)
        {
          ReadValue<uint16_t>(File, Total);
          if (Total > 0 && !bFramePrinted)
          {
            PrintFrame(Info);
            bFramePrinted = true;
          }
          Info << " Vehicle animations: " << Total << std::endl;
          for (i = 0; i < Total; ++i)
          {
            Vehicle.Read(File);
            Info << "  Id: " << Vehicle.DatabaseId << " Steering: " << Vehicle.Steering << " Throttle: " << Vehicle.Throttle << " Brake: " << Vehicle.Brake << " Handbrake: " << Vehicle.bHandbrake << " Gear: " << Vehicle.Gear << std::endl;
          }
        }
        else
          SkipPacket();
        break;

      // 行走者动画
      case static_cast<char>(CarlaRecorderPacketId::AnimWalker):
        if (bShowAll)
        {
          ReadValue<uint16_t>(File, Total);
          if (Total > 0 && !bFramePrinted)
          {
            PrintFrame(Info);
            bFramePrinted = true;
          }
          Info << " Walker animations: " << Total << std::endl;
          for (i = 0; i < Total; ++i)
          {
            Walker.Read(File);
            Info << "  Id: " << Walker.DatabaseId << " speed: " << Walker.Speed << std::endl;
          }
        }
        else
          SkipPacket();
        break;

      // 车辆门动画
      case static_cast<char>(CarlaRecorderPacketId::VehicleDoor):
        if (bShowAll)
        {
          ReadValue<uint16_t>(File, Total);
          if (Total > 0 && !bFramePrinted)
          {
            PrintFrame(Info);
            bFramePrinted = true;
          }
          Info << " Vehicle door animations: " << Total << std::endl;
          for (i = 0; i < Total; ++i)
          {
            DoorVehicle.Read(File);

            CarlaRecorderDoorVehicle::VehicleDoorType doorVehicle;
            doorVehicle = DoorVehicle.Doors;
            EVehicleDoor eDoorVehicle = static_cast<EVehicleDoor>(doorVehicle);
            std::string opened_doors_list;

            Info << "  Id: " << DoorVehicle.DatabaseId << std::endl; 
            Info << "  Doors opened: "; 
            if (eDoorVehicle == EVehicleDoor::FL)
              Info << " Front Left " << std::endl;
            if (eDoorVehicle == EVehicleDoor::FR)
              Info << " Front Right " << std::endl;
            if (eDoorVehicle == EVehicleDoor::RL)
              Info << " Rear Left " << std::endl;
            if (eDoorVehicle == EVehicleDoor::RR)
              Info << " Rear Right " << std::endl;
            if (eDoorVehicle == EVehicleDoor::Hood)
              Info << " Hood " << std::endl;
            if (eDoorVehicle == EVehicleDoor::Trunk)
              Info << " Trunk " << std::endl;
            if (eDoorVehicle == EVehicleDoor::All)
              Info << " All " << std::endl;
          }
        }
        else
          SkipPacket();
        break;


      // 车辆灯光动画
      case static_cast<char>(CarlaRecorderPacketId::VehicleLight):
        if (bShowAll)
        {
          ReadValue<uint16_t>(File, Total);
          if (Total > 0 && !bFramePrinted)
          {
            PrintFrame(Info);
            bFramePrinted = true;
          }
          Info << " Vehicle light animations: " << Total << std::endl;
          for (i = 0; i < Total; ++i)
          {
            LightVehicle.Read(File);

            carla::rpc::VehicleLightState LightState(LightVehicle.State);
            FVehicleLightState State(LightState);
            std::string enabled_lights_list;
            if (State.Position)
              enabled_lights_list += "Position ";
            if (State.LowBeam)
              enabled_lights_list += "LowBeam ";
            if (State.HighBeam)
              enabled_lights_list += "HighBeam ";
            if (State.Brake)
              enabled_lights_list += "Brake ";
            if (State.RightBlinker)
              enabled_lights_list += "RightBlinker ";
            if (State.LeftBlinker)
              enabled_lights_list += "LeftBlinker ";
            if (State.Reverse)
              enabled_lights_list += "Reverse ";
            if (State.Interior)
              enabled_lights_list += "Interior ";
            if (State.Fog)
              enabled_lights_list += "Fog ";
            if (State.Special1)
              enabled_lights_list += "Special1 ";
            if (State.Special2)
              enabled_lights_list += "Special2 ";

            if (enabled_lights_list.size())
            {
              Info << "  Id: " << LightVehicle.DatabaseId << " " <<
                  enabled_lights_list.substr(0, enabled_lights_list.size() - 1) << std::endl;
            }
            else
            {
              Info << "  Id: " << LightVehicle.DatabaseId << " None" << std::endl;
            }
          }
        }
        else
          SkipPacket();
        break;

      // 场景光照动画
      case static_cast<char>(CarlaRecorderPacketId::SceneLight):
        if (bShowAll)
        {
          ReadValue<uint16_t>(File, Total);
          if (Total > 0 && !bFramePrinted)
          {
            PrintFrame(Info);
            bFramePrinted = true;
          }
          Info << " Scene light changes: " << Total << std::endl;
          for (i = 0; i < Total; ++i)
          {
            LightScene.Read(File);
            Info << "  Id: " << LightScene.LightId << " enabled: " << (LightScene.bOn ? "True" : "False")
                << " intensity: " << LightScene.Intensity
                << " RGB_color: (" << LightScene.Color.R << ", " << LightScene.Color.G << ", " << LightScene.Color.B << ")"
                << std::endl;
          }
        }
        else
          SkipPacket();
        break;

      // 动态 Actor 运动学
      case static_cast<char>(CarlaRecorderPacketId::Kinematics):
        if (bShowAll)
        {
          ReadValue<uint16_t>(File, Total);
          if (Total > 0 && !bFramePrinted)
          {
            PrintFrame(Info);
            bFramePrinted = true;
          }
          Info << " Dynamic actors: " << Total << std::endl;
          for (i = 0; i < Total; ++i)
          {
            Kinematics.Read(File);
            Info << "  Id: " << Kinematics.DatabaseId << " linear_velocity: ("
                << Kinematics.LinearVelocity.X << ", " << Kinematics.LinearVelocity.Y << ", " << Kinematics.LinearVelocity.Z << ")"
                << " angular_velocity: ("
                << Kinematics.AngularVelocity.X << ", " << Kinematics.AngularVelocity.Y << ", " << Kinematics.AngularVelocity.Z << ")"
                << std::endl;
          }
        }
        else
          SkipPacket();
        break;

      // Actor 边界框
      case static_cast<char>(CarlaRecorderPacketId::BoundingBox):
        if (bShowAll)
        {
          ReadValue<uint16_t>(File, Total);
          if (Total > 0 && !bFramePrinted)
          {
            PrintFrame(Info);
            bFramePrinted = true;
          }
          Info << " Actor bounding boxes: " << Total << std::endl;
          for (i = 0; i < Total; ++i)
          {
            ActorBoundingBox.Read(File);
            Info << "  Id: " << ActorBoundingBox.DatabaseId << " origin: ("
                << ActorBoundingBox.BoundingBox.Origin.X << ", "
                << ActorBoundingBox.BoundingBox.Origin.Y << ", "
                << ActorBoundingBox.BoundingBox.Origin.Z << ")"
                << " extension: ("
                << ActorBoundingBox.BoundingBox.Extension.X << ", "
                << ActorBoundingBox.BoundingBox.Extension.Y << ", "
                << ActorBoundingBox.BoundingBox.Extension.Z << ")"
                << std::endl;
          }
        }
        else
          SkipPacket();
        break;

      // Actor 触发器卷
      case static_cast<char>(CarlaRecorderPacketId::TriggerVolume):
        if (bShowAll)
        {
          ReadValue<uint16_t>(File, Total);
          if (Total > 0 && !bFramePrinted)
          {
            PrintFrame(Info);
            bFramePrinted = true;
          }
          Info << " Actor trigger volumes: " << Total << std::endl;
          for (i = 0; i < Total; ++i)
          {
            ActorBoundingBox.Read(File);
            Info << "  Id: " << ActorBoundingBox.DatabaseId << " origin: ("
                << ActorBoundingBox.BoundingBox.Origin.X << ", "
                << ActorBoundingBox.BoundingBox.Origin.Y << ", "
                << ActorBoundingBox.BoundingBox.Origin.Z << ")"
                << " extension: ("
                << ActorBoundingBox.BoundingBox.Extension.X << ", "
                << ActorBoundingBox.BoundingBox.Extension.Y << ", "
                << ActorBoundingBox.BoundingBox.Extension.Z << ")"
                << std::endl;
          }
        }
        else
          SkipPacket();
        break;

      // 平台时间
      case static_cast<char>(CarlaRecorderPacketId::PlatformTime):
        if (bShowAll)
        {
          if (!bFramePrinted)
          {
            PrintFrame(Info);
            bFramePrinted = true;
          }

          PlatformTime.Read(File);
          Info << " Current platform time: " << PlatformTime.Time << std::endl;
        }
        else
          SkipPacket();
        break;

      case static_cast<char>(CarlaRecorderPacketId::PhysicsControl):
        if (bShowAll)
        {
          ReadValue<uint16_t>(File, Total);
          if (Total > 0 && !bFramePrinted)
          {
            PrintFrame(Info);
            bFramePrinted = true;
          }

          Info << " Physics Control events: " << Total << std::endl;
          for (i = 0; i < Total; ++i)
          {
            PhysicsControl.Read(File);
            carla::rpc::VehiclePhysicsControl Control(PhysicsControl.VehiclePhysicsControl);
            Info << "  Id: " << PhysicsControl.DatabaseId << std::endl
                << "   max_rpm = " << Control.max_rpm << std::endl
                << "   MOI = " << Control.moi << std::endl
                << "   damping_rate_full_throttle = " << Control.damping_rate_full_throttle << std::endl
                << "   damping_rate_zero_throttle_clutch_engaged = " << Control.damping_rate_zero_throttle_clutch_engaged << std::endl
                << "   damping_rate_zero_throttle_clutch_disengaged = " << Control.damping_rate_zero_throttle_clutch_disengaged << std::endl
                << "   use_gear_auto_box = " << (Control.use_gear_autobox ? "true" : "false") << std::endl
                << "   gear_switch_time = " << Control.gear_switch_time << std::endl
                << "   clutch_strength = " << Control.clutch_strength << std::endl
                << "   final_ratio = " << Control.final_ratio << std::endl
                << "   mass = " << Control.mass << std::endl
                << "   drag_coefficient = " << Control.drag_coefficient << std::endl
                << "   center_of_mass = " << "(" << Control.center_of_mass.x << ", " << Control.center_of_mass.y << ", " << Control.center_of_mass.z << ")" << std::endl;
            Info << "   torque_curve =";
            for (auto& vec : Control.torque_curve)
            {
              Info << " (" << vec.x << ", " << vec.y << ")";
            }
            Info << std::endl;
            Info << "   steering_curve =";
            for (auto& vec : Control.steering_curve)
            {
              Info << " (" << vec.x << ", " << vec.y << ")";
            }
            Info << std::endl;
            Info << "   forward_gears:" << std::endl;
            uint32_t count = 0;
            for (auto& Gear : Control.forward_gears)
            {
              Info << "    gear " << count << ": ratio " << Gear.ratio
                  << " down_ratio " << Gear.down_ratio
                  << " up_ratio " << Gear.up_ratio << std::endl;
              ++count;
            }
            Info << "   wheels:" << std::endl;
            count = 0;
            for (auto& Wheel : Control.wheels)
            {
              Info << "    wheel " << count << ": tire_friction " << Wheel.tire_friction
                  << " damping_rate " << Wheel.damping_rate
                  << " max_steer_angle " << Wheel.max_steer_angle
                  << " radius " << Wheel.radius
                  << " max_brake_torque " << Wheel.max_brake_torque
                  << " max_handbrake_torque " << Wheel.max_handbrake_torque
                  << " position " << "(" << Wheel.position.x << ", " << Wheel.position.y << ", " << Wheel.position.z << ")"
                  << std::endl;
              ++count;
            }
          }
        }
        else
          SkipPacket();
        break;

      case static_cast<char>(CarlaRecorderPacketId::TrafficLightTime):
        if (bShowAll)
        {
          ReadValue<uint16_t>(File, Total);
          if (Total > 0 && !bFramePrinted)
          {
            PrintFrame(Info);
            bFramePrinted = true;
          }

          Info << " Traffic Light time events: " << Total << std::endl;
          for (i = 0; i < Total; ++i)
          {
            TrafficLightTime.Read(File);
            Info << "  Id: " << TrafficLightTime.DatabaseId
                << " green_time: " << TrafficLightTime.GreenTime
                << " yellow_time: " << TrafficLightTime.YellowTime
                << " red_time: " << TrafficLightTime.RedTime
                << std::endl;
          }
        }
        else
          SkipPacket();
        break;

      case static_cast<char>(CarlaRecorderPacketId::WalkerBones):
        if (bShowAll)
        {
          ReadValue<uint16_t>(File, Total);
          if (Total > 0 && !bFramePrinted)
          {
            PrintFrame(Info);
            bFramePrinted = true;
          }

          Info << " Walkers Bones: " << Total << std::endl;
          for (i = 0; i < Total; ++i)
          {
            WalkerBones.Clear();
            WalkerBones.Read(File);
            Info << "  Id: " << WalkerBones.DatabaseId << "\n";
            for (const auto &Bone : WalkerBones.Bones)
            {
              Info << "     Bone: \"" << TCHAR_TO_UTF8(*Bone.Name) << "\" relative: " << "Loc("
                   << Bone.Location.X << ", " << Bone.Location.Y << ", " << Bone.Location.Z << ") Rot(" 
                   << Bone.Rotation.X << ", " << Bone.Rotation.Y << ", " << Bone.Rotation.Z << ")\n";
            }
          }
          Info << std::endl;
        }
        else
          SkipPacket();
        break;

        // 帧终点
      case static_cast<char>(CarlaRecorderPacketId::FrameEnd):
        // 什么都不做，它是空的
        break;

      default:
        SkipPacket();
        break;
    }
  }

  Info << "\nFrames: " << Frame.Id << "\n";
  Info << "Duration: " << Frame.Elapsed << " seconds\n";

  File.close();

  return Info.str();
}

std::string CarlaRecorderQuery::QueryCollisions(std::string Filename, char Category1, char Category2)
{
  std::stringstream Info;

  // 获取最终路径 + 文件名
  std::string Filename2 = GetRecorderFilename(Filename);

  // 尝试打开
  File.open(Filename2, std::ios::binary);
  if (!File.is_open())
  {
    Info << "File " << Filename2 << " not found on server\n";
    return Info.str();
  }

  if (!CheckFileInfo(Info))
    return Info.str();

  // 其他， 车辆， 步行者， 交通灯， 英雄， 任何
  char Categories[] = { 'o', 'v', 'w', 't', 'h', 'a' };
  uint16_t i, Total;
  struct ReplayerActorInfo
  {
    uint8_t Type;
    FString Id;
  };
  std::unordered_map<uint32_t, ReplayerActorInfo> Actors;
  struct PairHash
  {
    std::size_t operator()(const std::pair<uint32_t, uint32_t>& P) const
    {
        std::size_t hash = P.first;
        hash <<= 32;
        hash += P.second;
        return hash;
    }
  };
  std::unordered_set<std::pair<uint32_t, uint32_t>, PairHash > oldCollisions, newCollisions;

  // 页眉
  Info << std::setw(8) << "Time";
  Info << " " << std::setw(6) << "Types";
  Info << " " << std::setw(6) << std::right << "Id";
  Info << " " << std::setw(35) << std::left << "Actor 1";
  Info << " " << std::setw(6) << std::right << "Id";
  Info << " " << std::setw(35) << std::left << "Actor 2";
  Info << std::endl;

  // 仅解析帧
  while (File)
  {

    // 获取标头
    if (!ReadHeader())
    {
      break;
    }

    // 检查帧数据包
    switch (Header.Id)
    {
      // 框架
      case static_cast<char>(CarlaRecorderPacketId::FrameStart):
        Frame.Read(File);
        // 交换碰撞集（以了解碰撞是新的还是从上一帧继续的）
        oldCollisions = std::move(newCollisions);
        newCollisions.clear();
        break;

      // 事件添加
      case static_cast<char>(CarlaRecorderPacketId::EventAdd):
        ReadValue<uint16_t>(File, Total);
        for (i = 0; i < Total; ++i)
        {
          // 添加
          EventAdd.Read(File);
          Actors[EventAdd.DatabaseId] = ReplayerActorInfo { EventAdd.Type, EventAdd.Description.Id };
        }
        break;

      // 事件删除
      case static_cast<char>(CarlaRecorderPacketId::EventDel):
        ReadValue<uint16_t>(File, Total);
        for (i = 0; i < Total; ++i)
        {
          EventDel.Read(File);
          Actors.erase(EventAdd.DatabaseId);
        }
        break;

      // 活动育儿
      case static_cast<char>(CarlaRecorderPacketId::EventParent):
        SkipPacket();
        break;

      // 碰撞
      case static_cast<char>(CarlaRecorderPacketId::Collision):
        ReadValue<uint16_t>(File, Total);
        for (i = 0; i < Total; ++i)
        {
          Collision.Read(File);

          int Valid = 0;

          // 获取两个 Actor 的类别
          uint8_t Type1, Type2;
          if (Collision.DatabaseId1 != uint32_t(-1))
            Type1 = Categories[Actors[Collision.DatabaseId1].Type];
          else
            Type1 = 'o'; // 其他非执行组件对象
          
          if (Collision.DatabaseId2 != uint32_t(-1))
            Type2 = Categories[Actors[Collision.DatabaseId2].Type];
          else
            Type2 = 'o'; // 其他非执行组件对象

          // 过滤器角色 1
          if (Category1 == 'a')
            ++Valid;
          else if (Category1 == Type1)
            ++Valid;
          else if (Category1 == 'h' && Collision.IsActor1Hero)
            ++Valid;

          // 过滤器角色  2
          if (Category2 == 'a')
            ++Valid;
          else if (Category2 == Type2)
            ++Valid;
          else if (Category2 == 'h' && Collision.IsActor2Hero)
            ++Valid;

          // 仅当两个 Actor 都通过过滤器时显示
          if (Valid == 2)
          {
            // 检查我们是否需要显示为 starting collision 还是 continuation 冲突
            auto collisionPair = std::make_pair(Collision.DatabaseId1, Collision.DatabaseId2);
            if (oldCollisions.count(collisionPair) == 0)
            {
              Info << std::setw(8) << std::setprecision(0) << std::right << std::fixed << Frame.Elapsed;
              Info << " " << "  " << Type1 << " " << Type2 << " ";
              Info << " " << std::setw(6) << std::right << Collision.DatabaseId1;
              Info << " " << std::setw(35) << std::left << TCHAR_TO_UTF8(*Actors[Collision.DatabaseId1].Id);
              Info << " " << std::setw(6) << std::right << Collision.DatabaseId2;
              Info << " " << std::setw(35) << std::left << TCHAR_TO_UTF8(*Actors[Collision.DatabaseId2].Id);
              Info << std::endl;
            }
            // 保存当前碰撞
            newCollisions.insert(collisionPair);
          }
        }
        break;

      case static_cast<char>(CarlaRecorderPacketId::Position):
        SkipPacket();
        break;

      case static_cast<char>(CarlaRecorderPacketId::State):
        SkipPacket();
        break;

      // 帧终点
      case static_cast<char>(CarlaRecorderPacketId::FrameEnd):
        // 什么都不做，它是空的
        break;

      default:
        SkipPacket();
        break;
    }
  }

  Info << "\nFrames: " << Frame.Id << "\n";
  Info << "Duration: " << Frame.Elapsed << " seconds\n";

  File.close();

  return Info.str();
}

std::string CarlaRecorderQuery::QueryBlocked(std::string Filename, double MinTime, double MinDistance)
{
  std::stringstream Info;

  // 获取最终路径 + 文件名
  std::string Filename2 = GetRecorderFilename(Filename);

  // 尝试打开
  File.open(Filename2, std::ios::binary);
  if (!File.is_open())
  {
    Info << "File " << Filename2 << " not found on server\n";
    return Info.str();
  }

  if (!CheckFileInfo(Info))
    return Info.str();

  // 其他， 车辆， 步行者， 交通灯， 英雄， 任何
  uint16_t i, Total;
  struct ReplayerActorInfo
  {
    uint8_t Type;
    FString Id;
    FVector LastPosition;
    double Time;
    double Duration;
  };
  std::unordered_map<uint32_t, ReplayerActorInfo> Actors;
  // 能够按每个角色的持续时间对结果进行排序（降序）
  std::multimap<double, std::string, std::greater<double>> Results;

  // 页眉
  Info << std::setw(8) << "Time";
  Info << " " << std::setw(6) << "Id";
  Info << " " << std::setw(35) << std::left << "Actor";
  Info << " " << std::setw(10) << std::right << "Duration";
  Info << std::endl;

  // 仅解析帧
  while (File)
  {

    // 获取标头
    if (!ReadHeader())
    {
      break;
    }

    // 检查帧数据包
    switch (Header.Id)
    {
      // 页眉
      case static_cast<char>(CarlaRecorderPacketId::FrameStart):
        Frame.Read(File);
        break;

      // 事件添加
      case static_cast<char>(CarlaRecorderPacketId::EventAdd):
        ReadValue<uint16_t>(File, Total);
        for (i = 0; i < Total; ++i)
        {
          //添加
          EventAdd.Read(File);
          Actors[EventAdd.DatabaseId] = ReplayerActorInfo { EventAdd.Type, EventAdd.Description.Id, FVector(0, 0, 0), 0.0, 0.0 };
        }
        break;

      // 事件删除
      case static_cast<char>(CarlaRecorderPacketId::EventDel):
        ReadValue<uint16_t>(File, Total);
        for (i = 0; i < Total; ++i)
        {
          EventDel.Read(File);
          Actors.erase(EventAdd.DatabaseId);
        }
        break;

      // 活动育儿
      case static_cast<char>(CarlaRecorderPacketId::EventParent):
        SkipPacket();
        break;

      // 碰撞
      case static_cast<char>(CarlaRecorderPacketId::Collision):
        SkipPacket();
        break;

      // 位置
      case static_cast<char>(CarlaRecorderPacketId::Position):
        // 阅读所有位置
        ReadValue<uint16_t>(File, Total);
        for (i=0; i<Total; ++i)
        {
          Position.Read(File);
          // 检查 Actor 是否移动的距离小于
          if (FVector::Distance(Actors[Position.DatabaseId].LastPosition, Position.Location) < MinDistance)
          {
            // Actor 已停止
            if (Actors[Position.DatabaseId].Duration == 0)
              Actors[Position.DatabaseId].Time = Frame.Elapsed;
            Actors[Position.DatabaseId].Duration += Frame.DurationThis;
          }
          else
          {
            // Actor 已停止
            if (Actors[Position.DatabaseId].Duration >= MinTime)
            {
              std::stringstream Result;
              Result << std::setw(8) << std::setprecision(0) << std::fixed << Actors[Position.DatabaseId].Time;
              Result << " " << std::setw(6) << Position.DatabaseId;
              Result << " " << std::setw(35) << std::left << TCHAR_TO_UTF8(*Actors[Position.DatabaseId].Id);
              Result << " " << std::setw(10) << std::setprecision(0) << std::fixed << std::right << Actors[Position.DatabaseId].Duration;
              Result << std::endl;
              Results.insert(std::make_pair(Actors[Position.DatabaseId].Duration, Result.str()));
            }
            // Actor 移动
            Actors[Position.DatabaseId].Duration = 0;
            Actors[Position.DatabaseId].LastPosition = Position.Location;
          }
        }
        break;

      // 交通灯
      case static_cast<char>(CarlaRecorderPacketId::State):
        SkipPacket();
        break;

      //  帧终点
      case static_cast<char>(CarlaRecorderPacketId::FrameEnd):
        //什么都不做，它是空的
        break;

      default:
        SkipPacket();
        break;
    }
  }

  // 停止的 Show Actors 不再移动
  for (auto &Actor : Actors)
  {
    // 检查以显示信息
    if (Actor.second.Duration >= MinTime)
    {
      std::stringstream Result;
      Result << std::setw(8) << std::setprecision(0) << std::fixed << Actor.second.Time;
      Result << " " << std::setw(6) << Actor.first;
      Result << " " << std::setw(35) << std::left << TCHAR_TO_UTF8(*Actor.second.Id);
      Result << " " << std::setw(10) << std::setprecision(0) << std::fixed << std::right << Actor.second.Duration;
      Result << std::endl;
      Results.insert(std::make_pair(Actor.second.Duration, Result.str()));
    }
  }

  // 显示结果
  for (auto &Result : Results)
  {
    Info << Result.second;
  }

  Info << "\nFrames: " << Frame.Id << "\n";
  Info << "Duration: " << Frame.Elapsed << " seconds\n";

  File.close();

  return Info.str();
}
