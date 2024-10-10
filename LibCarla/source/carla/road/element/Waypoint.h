// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/road/RoadTypes.h" // 包含道路类型定义

#include <cstdint> // 包含标准整数类型定义
#include <functional> // 包含函数对象和函数适配器定义

namespace carla {
namespace road {
namespace element {

  struct Waypoint { // 定义Waypoint结构体

    RoadId road_id = 0u; // 道路ID

    SectionId section_id = 0u; // 路段ID

    LaneId lane_id = 0; // 车道ID

    double s = 0.0; // 沿道路的偏移量
  };

} // namespace element
} // namespace road
} // namespace carla

namespace std {

  template <>
  struct hash<carla::road::element::Waypoint> { // 定义Waypoint的哈希函数

    using argument_type = carla::road::element::Waypoint; // 参数类型

    using result_type = uint64_t; // 结果类型

    /// 根据@a waypoint的road_id、lane_id、section_id和"s"偏移量生成一个唯一的ID
    /// "s"偏移量被截断到半厘米精度
    ///
    result_type operator()(const argument_type &waypoint) const; // 哈希函数实现

  };

} // namespace std

namespace carla {
namespace road {
namespace element {

  inline bool operator==(const Waypoint &lhs, const Waypoint &rhs) { // 定义等于运算符
    auto hasher = std::hash<Waypoint>(); // 创建哈希对象
    return hasher(lhs) == hasher(rhs); // 返回哈希值是否相等
  }

  inline bool operator!=(const Waypoint &lhs, const Waypoint &rhs) { // 定义不等于运算符
    return !operator==(lhs, rhs); // 返回不等于运算符的结果
  }

} // namespace element
} // namespace road
} // namespace carla
