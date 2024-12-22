// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "MapLayer.h"

namespace carla {
namespace rpc {

std::string MapLayerToString(MapLayer MapLayerValue) //将MapLayer枚举值转换为其对应的字符串表示形式的函数 
{
  switch(MapLayerValue) //检查MapLayerValue的值  
  {
    case MapLayer::None:            return "None";
    case MapLayer::Buildings:       return "Buildings"; //表示地图上的建筑物  
    case MapLayer::Decals:          return "Decals"; //表示路面标记
    case MapLayer::Foliage:         return "Foliage"; //表示植被
    case MapLayer::Ground:          return "Ground"; //表示地图的地面层  
    case MapLayer::ParkedVehicles:  return "Parked_Vehicles"; //停放车辆层 
    case MapLayer::Particles:       return "Particles"; //表示烟雾或火焰等粒子效果
    case MapLayer::Props:           return "Props"; //环境中的杂项道具  
    case MapLayer::StreetLights:    return "StreetLights"; //表示街道照明 
    case MapLayer::Walls:           return "Walls"; //表示墙壁或障碍物 
    case MapLayer::All:             return "All"; //表示所有可用层  
    default:                        return "Invalid"; //对于无效的MapLayer值的后备处理
  }
}


} // namespace rpc
} // namespace carla
