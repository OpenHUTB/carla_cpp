// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/client/Junction.h" // 引入Junction头文件
#include "carla/client/Map.h" // 引入Map头文件
#include "carla/road/element/Waypoint.h" // 引入Waypoint头文件

namespace carla { // 定义carla命名空间
namespace client { // 定义client子命名空间

  Junction::Junction(SharedPtr<const Map> parent, const road::Junction *junction) : _parent(parent) { // Junction构造函数
    _bounding_box = junction->GetBoundingBox(); // 获取并存储交叉口的边界框
    _id = junction->GetId(); // 获取并存储交叉口的ID
  }

  std::vector<std::pair<SharedPtr<Waypoint>, SharedPtr<Waypoint>>> Junction::GetWaypoints( // 获取交叉口的路点
      road::Lane::LaneType type) const { // 输入参数：车道类型
    return _parent->GetJunctionWaypoints(GetId(), type); // 从父Map获取与交叉口相关的路点
  }

  geom::BoundingBox Junction::GetBoundingBox() const { // 获取交叉口的边界框
    return _bounding_box; // 返回存储的边界框
  }

} // namespace client
} // namespace carla

