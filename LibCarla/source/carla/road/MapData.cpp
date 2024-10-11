// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/road/MapData.h" // 引入地图数据的头文件
#include "carla/road/Lane.h"    // 引入车道的头文件

namespace carla {
namespace road {

  std::unordered_map<RoadId, Road> &MapData::GetRoads() { // 获取道路的引用
    return _roads; // 返回道路集合
  }

  std::unordered_map<JuncId, Junction> &MapData::GetJunctions() { // 获取交叉口的引用
    return _junctions; // 返回交叉口集合
  }

  Road &MapData::GetRoad(const RoadId id) { // 根据ID获取特定道路的引用
    return _roads.at(id); // 查找并返回对应的道路
  }

  const Road &MapData::GetRoad(const RoadId id) const { // 常量版本，根据ID获取特定道路的引用
    return const_cast<MapData *>(this)->GetRoad(id); // 将当前对象转换为非const并调用GetRoad
  }

  Junction *MapData::GetJunction(JuncId id) { // 根据ID获取特定交叉口的指针
    const auto search = _junctions.find(id); // 在交叉口集合中查找ID
    if (search != _junctions.end()) { // 如果找到了
      return &search->second; // 返回找到的交叉口的指针
    }
    return nullptr; // 如果没找到，返回空指针
  }

  const Junction *MapData::GetJunction(JuncId id) const { // 常量版本，根据ID获取特定交叉口的指针
    const auto search = _junctions.find(id); // 在交叉口集合中查找ID
    if (search != _junctions.end()) { // 如果找到了
      return &search->second; // 返回找到的交叉口的指针
    }
    return nullptr; // 如果没找到，返回空指针
  }
} // namespace road
} // namespace carla