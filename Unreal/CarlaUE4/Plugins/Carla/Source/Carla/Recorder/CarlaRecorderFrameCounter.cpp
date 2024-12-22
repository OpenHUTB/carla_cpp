// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "CarlaRecorderFrameCounter.h"
#include "CarlaRecorder.h"
#include "CarlaRecorderHelpers.h"

void CarlaRecorderFrameCounter::Read(std::istream &InFile)
{
  ReadValue<uint64_t>(InFile, this->FrameCounter);
}

void CarlaRecorderFrameCounter::Write(std::ostream &OutFile)
{
    // 写入包 ID，类型为 char。静态转换为帧计数器的包类型
    // `CarlaRecorderPacketId::FrameCounter` 应该是一个枚举值，表示这个数据包的类型是 "FrameCounter"
    WriteValue<char>(OutFile, static_cast<char>(CarlaRecorderPacketId::FrameCounter));

    // 写入包的大小，包的总大小为 uint64_t 类型的大小，即 8 字节
    // `sizeof(uint64_t)` 返回的是 uint64_t 类型的字节大小，这里为 8
    uint32_t Total = sizeof(uint64_t);
    WriteValue<uint32_t>(OutFile, Total);

    // 写入帧计数器的值，类型为 uint64_t（64 位无符号整数）
    WriteValue<uint64_t>(OutFile, this->FrameCounter);
}

