// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "CarlaRecorder.h"
#include "CarlaRecorderCollision.h"
#include "CarlaRecorderHelpers.h"
// 从输入流中读取碰撞数据
void CarlaRecorderCollision::Read(std::istream &InFile)
{
    // id
    ReadValue<uint32_t>(InFile, this->Id);
    // actors database id
    ReadValue<uint32_t>(InFile, this->DatabaseId1);
    ReadValue<uint32_t>(InFile, this->DatabaseId2);
    // is hero
    ReadValue<bool>(InFile, this->IsActor1Hero);
    ReadValue<bool>(InFile, this->IsActor2Hero);
}
// 将碰撞数据写入输出流
void CarlaRecorderCollision::Write(std::ostream &OutFile) const
{
    // id
    WriteValue<uint32_t>(OutFile, this->Id);
    // actors database id
    WriteValue<uint32_t>(OutFile, this->DatabaseId1);
    WriteValue<uint32_t>(OutFile, this->DatabaseId2);
    // is hero
    WriteValue<bool>(OutFile, this->IsActor1Hero);
    WriteValue<bool>(OutFile, this->IsActor2Hero);
}
// 判断两个 CarlaRecorderCollision 对象(DatabaseId1、DatabaseId2)是否相等
bool CarlaRecorderCollision::operator==(const CarlaRecorderCollision &Other) const
{
    return (this->DatabaseId1 == Other.DatabaseId1 &&
            this->DatabaseId2 == Other.DatabaseId2);
}
//---------------------------------------------
// 清空所有碰撞记录
// 将 Collisions 容器中的所有元素移除
void CarlaRecorderCollisions::Clear(void)
{
    Collisions.clear();
}

// 将新的碰撞记录添加到 Collisions 集合中，使用 std::move 以避免不必要的拷贝，直接将 Collision 对象转移到集合中
void CarlaRecorderCollisions::Add(const CarlaRecorderCollision &Collision)
{
    Collisions.insert(std::move(Collision));
}
// 将所有碰撞记录写入输出文件，遍历 Collisions 集合，将每一项碰撞记录通过 Write 方法写入文件
void CarlaRecorderCollisions::Write(std::ostream &OutFile)
{
    // write the packet id
    WriteValue<char>(OutFile, static_cast<char>(CarlaRecorderPacketId::Collision));

    // write the packet size
    uint32_t Total = 2 + Collisions.size() * sizeof(CarlaRecorderCollision);
    WriteValue<uint32_t>(OutFile, Total);

    // write total records
    Total = Collisions.size();
    WriteValue<uint16_t>(OutFile, Total);

    // for (uint16_t i=0; i<Total; ++i)
    for (auto &Coll : Collisions)
    {
        Coll.Write(OutFile);
    }
}
