// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "CarlaRecorder.h"
#include "CarlaRecorderAnimVehicle.h"
#include "CarlaRecorderHelpers.h"
// 将动画车辆的数据写入输出文件
void CarlaRecorderAnimVehicle::Write(std::ostream &OutFile)
{
  // database id
  WriteValue<uint32_t>(OutFile, this->DatabaseId);// 写入数据库ID
  WriteValue<float>(OutFile, this->Steering);// 写入转向值
  WriteValue<float>(OutFile, this->Throttle);// 写入油门值
  WriteValue<float>(OutFile, this->Brake);// 写入刹车值
  WriteValue<bool>(OutFile, this->bHandbrake);// 写入手刹状态
  WriteValue<int32_t>(OutFile, this->Gear);// 写入档位
}
// 从输入文件读取动画车辆的数据
void CarlaRecorderAnimVehicle::Read(std::istream &InFile)
{
  // database id
  ReadValue<uint32_t>(InFile, this->DatabaseId);// 读取数据库ID
  ReadValue<float>(InFile, this->Steering);// 读取转向值
  ReadValue<float>(InFile, this->Throttle);// 读取油门值
  ReadValue<float>(InFile, this->Brake);// 读取刹车值
  ReadValue<bool>(InFile, this->bHandbrake);// 读取手刹状态
  ReadValue<int32_t>(InFile, this->Gear);// 读取档位
}

// ---------------------------------------------
// 清除所有动画车辆的数据
void CarlaRecorderAnimVehicles::Clear(void)
{
  Vehicles.clear();// 清空车辆列表
}
// 添加一个新的动画车辆到列表
void CarlaRecorderAnimVehicles::Add(const CarlaRecorderAnimVehicle &Vehicle)
{
  Vehicles.push_back(Vehicle);// 将车辆添加到列表
}
// 将所有动画车辆的数据写入输出文件
void CarlaRecorderAnimVehicles::Write(std::ostream &OutFile)
{
  // write the packet id
  WriteValue<char>(OutFile, static_cast<char>(CarlaRecorderPacketId::AnimVehicle));
// 写入动画车辆的数据包ID
  std::streampos PosStart = OutFile.tellp();// 获取当前写入位置


  // write a dummy packet size
  uint32_t Total = 0;
  WriteValue<uint32_t>(OutFile, Total);

  // write total records
  Total = Vehicles.size();// 获取车辆总数
  WriteValue<uint16_t>(OutFile, Total);// 写入车辆总数

  for (uint16_t i=0; i<Total; ++i)
    Vehicles[i].Write(OutFile);// 写入每辆车的数据

  // write the real packet size
  std::streampos PosEnd = OutFile.tellp();// 获取当前写入位置
  Total = PosEnd - PosStart - sizeof(uint32_t);// 计算实际数据包大小
  OutFile.seekp(PosStart, std::ios::beg);// 移动写入位置到数据包大小的位置
  WriteValue<uint32_t>(OutFile, Total);// 写入实际数据包大小
  OutFile.seekp(PosEnd, std::ios::beg);// 移动写入位置到文件末尾
}
// 从输入文件读取所有动画车辆的数据
void CarlaRecorderAnimVehicles::Read(std::istream &InFile)
{
  uint16_t i, Total;
  CarlaRecorderAnimVehicle Vehicle;

  // read Total Vehicles
  ReadValue<uint16_t>(InFile, Total);
  for (i = 0; i < Total; ++i)
  {
    Vehicle.Read(InFile);
    Add(Vehicle);
  }
}
// 获取所有动画车辆的数据
const std::vector<CarlaRecorderAnimVehicle>& CarlaRecorderAnimVehicles::GetVehicles()
{
  return Vehicles;
}
