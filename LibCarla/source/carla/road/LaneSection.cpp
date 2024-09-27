// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/road/LaneSection.h"  // 引入车道段的头文件
#include "carla/road/Road.h"         // 引入道路的头文件

namespace carla {  // 定义carla命名空间
namespace road {   // 定义road命名空间

  double LaneSection::GetDistance() const {  // 获取车道段距离的函数
    return _s;  // 返回车道段的起始距离
  }

  double LaneSection::GetLength() const {  // 获取车道段长度的函数
    const auto *road = GetRoad();  // 获取所属道路
    DEBUG_ASSERT(road != nullptr);  // 确保道路不为空
    return road->UpperBound(_s) - _s;  // 返回车道段的长度
  }

  Road *LaneSection::GetRoad() const {  // 获取所属道路的函数
    return _road;  // 返回指向道路的指针
  }

  SectionId LaneSection::GetId() const {  // 获取车道段ID的函数
    return _id;  // 返回车道段的ID
  }

  Lane *LaneSection::GetLane(const LaneId id) {  // 根据ID获取车道的函数
    auto search = _lanes.find(id);  // 在车道映射中查找指定ID
    if (search != _lanes.end()) {  // 如果找到
      return &search->second;  // 返回对应的车道指针
    }
    return nullptr;  // 未找到，返回空指针
  }

  const Lane *LaneSection::GetLane(const LaneId id) const {  // 获取车道的常量版本
    auto search = _lanes.find(id);  // 在车道映射中查找指定ID
    if (search != _lanes.end()) {  // 如果找到
      return &search->second;  // 返回对应的车道指针
    }
    return nullptr;  // 未找到，返回空指针
  }

  std::map<LaneId, Lane> &LaneSection::GetLanes() {  // 获取所有车道的函数
    return _lanes;  // 返回车道的无序映射
  }

  const std::map<LaneId, Lane> &LaneSection::GetLanes() const {  // 获取所有车道的常量版本
    return _lanes;  // 返回车道的无序映射
  }

  std::vector<Lane *> LaneSection::GetLanesOfType(Lane::LaneType lane_type) {  // 获取指定类型车道的函数
    std::vector<Lane *> drivable_lanes;  // 用于存储可行驶车道的向量
    for (auto &&lane : _lanes) {  // 遍历所有车道
      if ((static_cast<uint32_t>(lane.second.GetType()) & static_cast<uint32_t>(lane_type)) > 0) {  // 检查车道类型是否匹配
        drivable_lanes.emplace_back(&lane.second);  // 如果匹配，则将车道添加到可行驶车道列表中
      }
    }
    return drivable_lanes;  // 返回可行驶车道的向量
  }

} // namespace road
} // namespace carla