// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "CarlaRecorder.h"
#include "CarlaRecorderAnimWalker.h"
#include "CarlaRecorderHelpers.h"

void CarlaRecorderAnimWalker::Write(std::ostream &OutFile)
{
  // 数据库 ID
  WriteValue<uint32_t>(OutFile, this->DatabaseId);
  WriteValue<float>(OutFile, this->Speed);
}
void CarlaRecorderAnimWalker::Read(std::istream &InFile)
{
  // 数据库 ID
  ReadValue<uint32_t>(InFile, this->DatabaseId);
  ReadValue<float>(InFile, this->Speed);
}

// ---------------------------------------------

void CarlaRecorderAnimWalkers::Clear(void)
{
  Walkers.clear();
}

void CarlaRecorderAnimWalkers::Add(const CarlaRecorderAnimWalker &Walker)
{
  Walkers.push_back(Walker);
}

void CarlaRecorderAnimWalkers::Write(std::ostream &OutFile)
{
  // 写入数据包 ID
  WriteValue<char>(OutFile, static_cast<char>(CarlaRecorderPacketId::AnimWalker));

  // 写入数据包大小
  uint32_t Total = 2 + Walkers.size() * sizeof(CarlaRecorderAnimWalker);
  WriteValue<uint32_t>(OutFile, Total);

  // 写入总记录数
  Total = Walkers.size();
  WriteValue<uint16_t>(OutFile, Total);

  // 写入记录
  if (Total > 0)
  {
    OutFile.write(reinterpret_cast<const char *>(Walkers.data()),
        Walkers.size() * sizeof(CarlaRecorderAnimWalker));
  }
}

void CarlaRecorderAnimWalkers::Read(std::istream &InFile)
{
  uint16_t i, Total;
  CarlaRecorderAnimWalker Walker;

  // 读取 Total walkers
  ReadValue<uint16_t>(InFile, Total);
  for (i = 0; i < Total; ++i)
  {
    Walker.Read(InFile);
    Add(Walker);
  }
}

const std::vector<CarlaRecorderAnimWalker>& CarlaRecorderAnimWalkers::GetWalkers()
{
  return Walkers;
}
