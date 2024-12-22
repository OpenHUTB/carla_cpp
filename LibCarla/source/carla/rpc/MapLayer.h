// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/MsgPack.h"

#include <cstdint>

namespace carla {
namespace rpc {

// 定义一个类型别名 MapLayerType，它是一个 16 位无符号整数
using MapLayerType = uint16_t;

// 定义一个枚举类 MapLayer，它继承自 MapLayerType
enum class MapLayer : MapLayerType
{
  // 没有任何层
  None            =  0,
  // 表示建筑物层
  Buildings       =  0x1,
  // 表示贴花层
  Decals          =  0x1 << 1,
  // 表示植被层
  Foliage         =  0x1 << 2,
  // 表示地面层
  Ground          =  0x1 << 3,
  // 表示停放的车辆层
  ParkedVehicles  =  0x1 << 4,
  // 表示粒子层
  Particles       =  0x1 << 5,
  // 表示道具层
  Props           =  0x1 << 6,
  // 表示路灯层
  StreetLights    =  0x1 << 7,
  // 表示墙壁层
  Walls           =  0x1 << 8,
  // 表示所有层
  All             =  0xFFFF,
};

// 函数声明：将 MapLayer 枚举值转换为字符串
std::string MapLayerToString(MapLayer MapLayerValue);

} // namespace rpc
} // namespace carla

// 定义一个用于序列化和反序列化 MapLayer 枚举的宏
MSGPACK_ADD_ENUM(carla::rpc::MapLayer);
