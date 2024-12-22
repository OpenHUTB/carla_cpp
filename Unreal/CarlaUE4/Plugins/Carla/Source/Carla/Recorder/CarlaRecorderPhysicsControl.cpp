// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "CarlaRecorderPhysicsControl.h"
#include "CarlaRecorder.h"
#include "CarlaRecorderHelpers.h"

#include <compiler/disable-ue4-macros.h>
#include "carla/rpc/VehiclePhysicsControl.h"
#include <compiler/enable-ue4-macros.h>


void CarlaRecorderPhysicsControl::Write(std::ostream &OutFile)
{
  carla::rpc::VehiclePhysicsControl RPCPhysicsControl(VehiclePhysicsControl);
  WriteValue<uint32_t>(OutFile, this->DatabaseId);
  WriteValue(OutFile, RPCPhysicsControl.max_rpm);
  WriteValue(OutFile, RPCPhysicsControl.moi);
  WriteValue(OutFile, RPCPhysicsControl.damping_rate_full_throttle);
  WriteValue(OutFile, RPCPhysicsControl.damping_rate_zero_throttle_clutch_engaged);
  WriteValue(OutFile, RPCPhysicsControl.damping_rate_zero_throttle_clutch_disengaged);
  WriteValue(OutFile, RPCPhysicsControl.use_gear_autobox);
  WriteValue(OutFile, RPCPhysicsControl.clutch_strength);
  WriteValue(OutFile, RPCPhysicsControl.final_ratio);
  WriteValue(OutFile, RPCPhysicsControl.mass);
  WriteValue(OutFile, RPCPhysicsControl.drag_coefficient);
  WriteValue(OutFile, RPCPhysicsControl.center_of_mass);

  // 扭矩曲线
  WriteStdVector(OutFile, RPCPhysicsControl.torque_curve);

  // 前进档
  WriteStdVector(OutFile, RPCPhysicsControl.forward_gears);

  // 转向曲线
  WriteStdVector(OutFile, RPCPhysicsControl.steering_curve);

  // 车轮
  WriteStdVector(OutFile, RPCPhysicsControl.wheels);
}

void CarlaRecorderPhysicsControl::Read(std::istream &InFile)
{
  carla::rpc::VehiclePhysicsControl RPCPhysicsControl;
  ReadValue<uint32_t>(InFile, this->DatabaseId);
  ReadValue(InFile, RPCPhysicsControl.max_rpm);
  ReadValue(InFile, RPCPhysicsControl.moi);
  ReadValue(InFile, RPCPhysicsControl.damping_rate_full_throttle);
  ReadValue(InFile, RPCPhysicsControl.damping_rate_zero_throttle_clutch_engaged);
  ReadValue(InFile, RPCPhysicsControl.damping_rate_zero_throttle_clutch_disengaged);
  ReadValue(InFile, RPCPhysicsControl.use_gear_autobox);
  ReadValue(InFile, RPCPhysicsControl.clutch_strength);
  ReadValue(InFile, RPCPhysicsControl.final_ratio);
  ReadValue(InFile, RPCPhysicsControl.mass);
  ReadValue(InFile, RPCPhysicsControl.drag_coefficient);
  ReadValue(InFile, RPCPhysicsControl.center_of_mass);

  // 扭矩曲线
  ReadStdVector(InFile, RPCPhysicsControl.torque_curve);

  // 前进档
  ReadStdVector(InFile, RPCPhysicsControl.forward_gears);

  // 转向曲线
  ReadStdVector(InFile, RPCPhysicsControl.steering_curve);

  // 车轮
  ReadStdVector(InFile, RPCPhysicsControl.wheels);

  VehiclePhysicsControl = FVehiclePhysicsControl(RPCPhysicsControl);
}

// ---------------------------------------------

void CarlaRecorderPhysicsControls::Clear(void)
{
  PhysicsControls.clear();
}

void CarlaRecorderPhysicsControls::Add(const CarlaRecorderPhysicsControl &InObj)
{
  PhysicsControls.push_back(InObj);
}

void CarlaRecorderPhysicsControls::Write(std::ostream &OutFile)
{
  if (PhysicsControls.size() == 0)
  {
    return;
  }
  // 写入数据包 ID
  WriteValue<char>(OutFile, static_cast<char>(CarlaRecorderPacketId::PhysicsControl));

  std::streampos PosStart = OutFile.tellp();
  // 写入 Dummy 数据包大小
  uint32_t Total = 0;
  WriteValue<uint32_t>(OutFile, Total);

  // 写入总记录数
  Total = PhysicsControls.size();
  WriteValue<uint16_t>(OutFile, Total);

  // 写入记录
  for (auto& PhysicsControl : PhysicsControls)
  {
    PhysicsControl.Write(OutFile);
  }

  // 写入实际数据包大小
  std::streampos PosEnd = OutFile.tellp();
  Total = PosEnd - PosStart - sizeof(uint32_t);
  OutFile.seekp(PosStart, std::ios::beg);
  WriteValue<uint32_t>(OutFile, Total);
  OutFile.seekp(PosEnd, std::ios::beg);
}
