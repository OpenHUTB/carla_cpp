// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/road/element/LaneCrossingCalculator.h" // 导入车道交叉计算器头文件
#include "carla/road/element/LaneMarking.h" // 导入车道标记头文件

#include "carla/geom/Location.h" // 导入位置相关头文件
#include "carla/road/Map.h" // 导入地图相关头文件

namespace carla {
namespace road {
namespace element {

  /// @todo 临时标志，用于搜索可以找到道路标记的车道。
  /// 这需要扩展以搜索肩道，但由于肩道宽度较小，
  /// 在给定位置的最近车道中心查找时可能会导致问题。
  static constexpr uint32_t FLAGS =
      static_cast<uint32_t>(Lane::LaneType::Driving) | // 驾驶车道类型
      static_cast<uint32_t>(Lane::LaneType::Bidirectional) | // 双向车道类型
      static_cast<uint32_t>(Lane::LaneType::Biking) | // 自行车车道类型
      static_cast<uint32_t>(Lane::LaneType::Parking); // 停车车道类型

  /// 从 @a lane_id_origin 到 @a lane_id_destination 计算需要跨越的车道标记。
  static std::vector<LaneMarking> CrossingAtSameSection(
      const Map &map, // 地图引用
      const Waypoint *w0, // 起始点航路点
      const Waypoint *w1, // 目标点航路点
      const bool w0_is_offroad, // 起始点是否在路外
      const bool dest_is_at_right) { // 目标点是否在右侧
    auto w0_marks = map.GetMarkRecord(*w0); // 获取起始点的标记记录
    auto w1_marks = map.GetMarkRecord(*w1); // 获取目标点的标记记录

    if (dest_is_at_right) { // 如果目标点在右侧
      if (w0_is_offroad) { // 如果起始点在路外
        return { LaneMarking(*w1_marks.second) }; // 返回目标点的第二个标记
      } else {
        return { LaneMarking(*w0_marks.first) }; // 返回起始点的第一个标记
      }
    } else { // 如果目标点在左侧
      if (w0_is_offroad) { // 如果起始点在路外
        return { LaneMarking(*w1_marks.first) }; // 返回目标点的第一个标记
      } else {
        return { LaneMarking(*w0_marks.second) }; // 返回起始点的第二个标记
      }
    }

    return {}; // 默认返回空向量
  }

  static bool IsOffRoad(const Map &map, const geom::Location &location) {
    return !map.GetWaypoint(location, FLAGS).has_value(); // 判断位置是否在路外
  }

  std::vector<LaneMarking> LaneCrossingCalculator::Calculate(
      const Map &map, // 地图引用
      const geom::Location &origin, // 起始位置
      const geom::Location &destination) { // 目标位置
    auto w0 = map.GetClosestWaypointOnRoad(origin, FLAGS); // 获取起始位置的最近航路点
    auto w1 = map.GetClosestWaypointOnRoad(destination, FLAGS); // 获取目标位置的最近航路点

    if (!w0.has_value() || !w1.has_value()) { // 如果任一航路点无效
      return {}; // 返回空向量
    }

    if (w0->road_id != w1->road_id || w0->section_id != w1->section_id) { // 如果不在同一条道路或段落
      /// @todo 这个情况也应该被处理。
      return {}; // 返回空向量
    }

    if (map.IsJunction(w0->road_id) || map.IsJunction(w1->road_id)) { // 如果任一航路点在交叉口
      return {}; // 返回空向量
    }

    const auto w0_is_offroad = IsOffRoad(map, origin); // 检查起始位置是否在路外
    const auto w1_is_offroad = IsOffRoad(map, destination); // 检查目标位置是否在路外

    if (w0_is_offroad && w1_is_offroad) { // 如果两者都在路外
      // outside the road
      return {}; // 返回空向量
    }

    if ((w0->lane_id == w1->lane_id) && !w0_is_offroad && !w1_is_offroad) { // 如果在同一车道且都在路内
      // both at the same lane and inside the road
      return {}; // 返回空向量
    }

    const auto transform = map.ComputeTransform(*w0); // 计算起始航路点的转换
    geom::Vector3D orig_vec = transform.GetForwardVector(); // 获取起始点的前向向量
    geom::Vector3D dest_vec = (destination - origin).MakeSafeUnitVector(2 * std::numeric_limits<float>::epsilon()); // 计算目标向量并标准化

    // cross product
    const auto dest_is_at_right =
        (-orig_vec.x * dest_vec.y + orig_vec.y * dest_vec.x) < 0; // 判断目标点是否在右侧

    return CrossingAtSameSection( // 返回同一段的交叉车道标记
        map,
        &*w0,
        &*w1,
        w0_is_offroad,
        dest_is_at_right);
  }

} // namespace element
} // namespace road
} // namespace carla
