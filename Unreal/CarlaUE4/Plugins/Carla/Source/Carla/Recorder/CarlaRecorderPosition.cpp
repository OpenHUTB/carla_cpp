// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "CarlaRecorder.h"
#include "CarlaRecorderPosition.h"
#include "CarlaRecorderHelpers.h"

void CarlaRecorderPosition::Write(std::ostream &OutFile)
{
  // 数据库 ID
  WriteValue<uint32_t>(OutFile, this->DatabaseId);
  // 变换
  WriteFVector(OutFile, this->Location);
  WriteFVector(OutFile, this->Rotation);
}
void CarlaRecorderPosition::Read(std::istream &InFile)
{
  // 数据库 ID
  ReadValue<uint32_t>(InFile, this->DatabaseId);
  // 变换
  ReadFVector(InFile, this->Location);
  ReadFVector(InFile, this->Rotation);
}

// ---------------------------------------------

void CarlaRecorderPositions::Clear(void)
{
  Positions.clear();
}

void CarlaRecorderPositions::Add(const CarlaRecorderPosition &Position)
{
  Positions.push_back(Position);
}

void CarlaRecorderPositions::Write(std::ostream &OutFile)
{
  // 写入数据包 ID
  WriteValue<char>(OutFile, static_cast<char>(CarlaRecorderPacketId::Position));

  // 写入数据包大小
  uint32_t Total = 2 + Positions.size() * sizeof(CarlaRecorderPosition);
  WriteValue<uint32_t>(OutFile, Total);

  // 写入总记录数
  Total = Positions.size();
  WriteValue<uint16_t>(OutFile, Total);

  // 写入记录
  if (Total > 0)
  {
    OutFile.write(reinterpret_cast<const char *>(Positions.data()),
        Positions.size() * sizeof(CarlaRecorderPosition));
  }
}

void CarlaRecorderPositions::Read(std::istream &InFile)
{
  uint16_t i, Total;

  // 阅读所有位置
  ReadValue<uint16_t>(InFile, Total);
  for (i = 0; i < Total; ++i)
  {
    CarlaRecorderPosition Pos;
    Pos.Read(InFile);
    Add(Pos);
  }
}

const std::vector<CarlaRecorderPosition>& CarlaRecorderPositions::GetPositions()
{
  return Positions;
}
