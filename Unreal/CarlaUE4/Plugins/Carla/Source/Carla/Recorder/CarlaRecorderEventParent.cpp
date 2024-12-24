// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "CarlaRecorder.h"
#include "CarlaRecorderEventParent.h"
#include "CarlaRecorderHelpers.h"

// 从输入流 InFile 中读取数据，并将其存储在类的成员变量中
void CarlaRecorderEventParent::Read(std::istream &InFile)
{
    // database id
    ReadValue<uint32_t>(InFile, this->DatabaseId);
    // database id parent
    ReadValue<uint32_t>(InFile, this->DatabaseIdParent);
}
void CarlaRecorderEventParent::Write(std::ostream &OutFile) const
{
    // database id
    WriteValue<uint32_t>(OutFile, this->DatabaseId);
    // database id parent
    WriteValue<uint32_t>(OutFile, this->DatabaseIdParent);
}

//---------------------------------------------

void CarlaRecorderEventsParent::Clear(void)
{
    Events.clear();
}

void CarlaRecorderEventsParent::Add(const CarlaRecorderEventParent &Event)
{
    Events.push_back(std::move(Event));
}

void CarlaRecorderEventsParent::Write(std::ostream &OutFile)
{
    // write the packet id
    WriteValue<char>(OutFile, static_cast<char>(CarlaRecorderPacketId::EventParent));

    std::streampos PosStart = OutFile.tellp();

    // write a dummy packet size
    uint32_t Total = 0;
    WriteValue<uint32_t>(OutFile, Total);

    // write total records
    Total = Events.size();
    WriteValue<uint16_t>(OutFile, Total);

    for (uint16_t i=0; i<Total; ++i)
    {
        Events[i].Write(OutFile);
    }

    // write the real packet size
    std::streampos PosEnd = OutFile.tellp();
    Total = PosEnd - PosStart - sizeof(uint32_t);
    OutFile.seekp(PosStart, std::ios::beg);
    WriteValue<uint32_t>(OutFile, Total);
    OutFile.seekp(PosEnd, std::ios::beg);
}

void CarlaRecorderEventsParent::Read(std::istream &InFile)
{
    uint16_t i, Total;
    CarlaRecorderEventParent EventParent;
    std::stringstream Info;

    // process parenting events
    ReadValue<uint16_t>(InFile, Total);
    for (i = 0; i < Total; ++i)
    {
        EventParent.Read(InFile);
        Add(EventParent);
    }
}

const std::vector<CarlaRecorderEventParent>& CarlaRecorderEventsParent::GetEvents()
{
    return Events;
}
