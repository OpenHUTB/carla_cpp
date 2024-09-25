// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/geom/CubicPolynomial.h" // 包含立方多项式几何操作的头文件
#include "carla/NonCopyable.h"          // 包含非拷贝类功能的头文件
#include "carla/road/Lane.h"            // 包含车道类定义的头文件
#include "carla/road/RoadElementSet.h"  // 包含道路元素集合的头文件
#include "carla/road/RoadTypes.h"       // 包含各种道路类型定义的头文件

#include <map>      // 包含 std::map 容器的头文件
#include <vector>   // 包含 std::vector 容器的头文件

namespace carla {
namespace road {

  class Road;            // 前向声明 Road 类
  class MapBuilder;      // 前向声明 MapBuilder 类

  class LaneSection : private MovableNonCopyable { // LaneSection 类继承非拷贝行为
  public:

    explicit LaneSection(SectionId id, double s) : _id(id), _s(s) {} // 构造函数初始化 ID 和位置

    double GetDistance() const; // 方法获取与节段开始的距离

    double GetLength() const; // 方法获取车道节段的长度

    Road *GetRoad() const; // 方法获取关联的 Road 对象

    Lane *GetLane(const LaneId id); // 方法通过 ID 获取 Lane 对象

    const Lane *GetLane(const LaneId id) const; // 常量方法通过 ID 获取 Lane 对象

    bool ContainsLane(LaneId id) const { // 方法检查节段中是否包含指定车道
      return (_lanes.find(id) != _lanes.end()); // 如果找到车道 ID，返回 true
    }

    SectionId GetId() const; // 方法获取节段 ID

    std::map<LaneId, Lane> &GetLanes(); // 方法获取可修改的车道引用

    const std::map<LaneId, Lane> &GetLanes() const; // 常量方法获取车道引用，不能修改

    std::vector<Lane *> GetLanesOfType(Lane::LaneType type); // 方法获取特定类型的车道

  private:

    friend MapBuilder; // 允许 MapBuilder 访问私有成员

    const SectionId _id = 0u; // 车道节段的唯一标识符

    const double _s = 0.0; // 该节段在道路上的起始位置

    Road *_road = nullptr; // 指向关联 Road 对象的指针

    std::map<LaneId, Lane> _lanes; // 该节段中的车道集合，以 ID 索引

    geom::CubicPolynomial _lane_offset; // 用于车道偏移计算的立方多项式
  };

} // namespace road
} // namespace carla