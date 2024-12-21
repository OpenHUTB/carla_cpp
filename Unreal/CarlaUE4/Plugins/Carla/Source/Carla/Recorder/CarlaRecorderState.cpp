// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "CarlaRecorder.h"
#include "CarlaRecorderState.h"
#include "CarlaRecorderHelpers.h"

void CarlaRecorderStateTrafficLight::Write(std::ostream &OutFile)
{
  WriteValue<uint32_t>(OutFile, this->DatabaseId);
  WriteValue<bool>(OutFile, this->IsFrozen);
  WriteValue<float>(OutFile, this->ElapsedTime);
  WriteValue<char>(OutFile, this->State);
}

void CarlaRecorderStateTrafficLight::Read(std::istream &InFile)
{
  ReadValue<uint32_t>(InFile, this->DatabaseId);
  ReadValue<bool>(InFile, this->IsFrozen);
  ReadValue<float>(InFile, this->ElapsedTime);
  ReadValue<char>(InFile, this->State);
}

// ---------------------------------------------

void CarlaRecorderStates::Clear(void)
{
  StatesTrafficLights.clear();
}

void CarlaRecorderStates::Add(const CarlaRecorderStateTrafficLight &State)
{
  StatesTrafficLights.push_back(std::move(State));
}

void CarlaRecorderStates::Write(std::ostream &OutFile)
{
  // 写入数据包 ID
  WriteValue<char>(OutFile, static_cast<char>(CarlaRecorderPacketId::State));

  // 写入数据包大小
  uint32_t Total = 2 + StatesTrafficLights.size() * sizeof(CarlaRecorderStateTrafficLight);
  WriteValue<uint32_t>(OutFile, Total);

  // 写入总记录数
  Total = StatesTrafficLights.size();
  WriteValue<uint16_t>(OutFile, Total);

  for (uint16_t i = 0; i < Total; ++i)
  {
    StatesTrafficLights[i].Write(OutFile);
  }
}

void CarlaRecorderStates::Read(std::istream &InFile)
{
  uint16_t i, Total;
  CarlaRecorderStateTrafficLight StateTrafficLight;

  // 读取红绿灯状态总数
  ReadValue<uint16_t>(InFile, Total);
  for (i = 0; i < Total; ++i)
  {
    StateTrafficLight.Read(InFile);
    Add(StateTrafficLight);
  }
}

const std::vector<CarlaRecorderStateTrafficLight>& CarlaRecorderStates::GetStates()
{
  return StatesTrafficLights;
}
