// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/Exception.h" // 引入异常处理头文件
#include "carla/geom/CubicPolynomial.h" // 引入三次多项式几何头文件
#include "carla/geom/Location.h" // 引入位置几何头文件
#include "carla/geom/Math.h" // 引入数学运算头文件
#include "carla/ListView.h" // 引入列表视图头文件
#include "carla/Logging.h" // 引入日志记录头文件
#include "carla/road/element/RoadInfoElevation.h" // 引入道路信息的高度头文件
#include "carla/road/element/RoadInfoGeometry.h" // 引入道路几何信息头文件
#include "carla/road/element/RoadInfoLaneOffset.h" // 引入车道偏移信息头文件
#include "carla/road/element/RoadInfoLaneWidth.h" // 引入车道宽度信息头文件
#include "carla/road/Lane.h" // 引入车道头文件
#include "carla/road/MapData.h" // 引入地图数据头文件
#include "carla/road/Road.h" // 引入道路头文件

#include <stdexcept> // 引入标准异常处理头文件

namespace carla {
namespace road {

  // 获取与该道路相关联的地图数据
  const MapData *Road::GetMap() const {
    return _map_data; // 返回地图数据指针
  }

  // 获取道路的ID
  RoadId Road::GetId() const {
    return _id; // 返回道路ID
  }

  // 获取道路名称
  std::string Road::GetName() const {
    return _name; // 返回道路名称
  }

  // 获取道路长度
  double Road::GetLength() const {
    return _length; // 返回道路长度
  }

  // 判断该道路是否为交叉口
  bool Road::IsJunction() const {
    return _is_junction; // 返回是否为交叉口
  }

  // 获取交叉口ID
  JuncId Road::GetJunctionId() const {
    return _junction_id; // 返回交叉口ID
  }

  // 获取下一条道路的ID
  RoadId Road::GetSuccessor() const {
    return _successor; // 返回下一条道路ID
  }

  // 获取前一条道路的ID
  RoadId Road::GetPredecessor() const {
    return _predecessor; // 返回前一条道路ID
  }

  // 获取下一条道路的指针列表
  std::vector<Road *> Road::GetNexts() const {
    return _nexts; // 返回下一条道路的指针列表
  }

  // 获取前一条道路的指针列表
  std::vector<Road *> Road::GetPrevs() const {
    return _prevs; // 返回前一条道路的指针列表
  }

  // 获取给定距离s处的道路高度多项式
  const geom::CubicPolynomial &Road::GetElevationOn(const double s) const {
    auto info = GetInfo<element::RoadInfoElevation>(s); // 获取道路高度信息
    if (info == nullptr) { // 如果没有找到高度信息
      throw_exception(std::runtime_error("failed to find road elevation.")); // 抛出异常
    }
    return info->GetPolynomial(); // 返回高度多项式
  }

  // 根据距离s和车道ID获取车道的引用
  Lane &Road::GetLaneByDistance(double s, LaneId lane_id) {
    for (auto &section : GetLaneSectionsAt(s)) { // 遍历在距离s处的车道段
      auto *lane = section.GetLane(lane_id); // 获取指定ID的车道
      if (lane != nullptr) { // 如果找到了车道
        return *lane; // 返回车道引用
      }
    }
    throw_exception(std::runtime_error("lane not found")); // 抛出异常，表示未找到车道
  }

  // 获取给定距离s和车道ID的常量车道引用
  const Lane &Road::GetLaneByDistance(double s, LaneId lane_id) const {
    return const_cast<Road *>(this)->GetLaneByDistance(s, lane_id); // 调用非常量版本
  }

  // 根据距离s获取所有车道的指针列表
  std::vector<Lane*> Road::GetLanesByDistance(double s) {
    std::vector<Lane*> result; // 创建结果列表
    auto lane_sections = GetLaneSectionsAt(s); // 获取在距离s处的车道段
    for (auto &lane_section : lane_sections) { // 遍历每个车道段
      for (auto & lane_pair : lane_section.GetLanes()) { // 遍历车道段中的每个车道
        result.emplace_back(&lane_pair.second); // 将车道指针添加到结果列表
      }
    }
    return result; // 返回车道指针列表
  }

  // 根据距离s获取所有车道的常量指针列表
  std::vector<const Lane*> Road::GetLanesByDistance(double s) const {
    std::vector<const Lane*> result; // 创建结果列表
    const auto lane_sections = GetLaneSectionsAt(s); // 获取在距离s处的车道段
    for (const auto &lane_section : lane_sections) { // 遍历每个车道段
      for (const auto & lane_pair : lane_section.GetLanes()) { // 遍历车道段中的每个车道
        result.emplace_back(&lane_pair.second); // 将车道指针添加到结果列表
      }
    }
    return result; // 返回车道指针列表
  }

  // 根据车道段ID和车道ID获取车道的引用
  Lane &Road::GetLaneById(SectionId section_id, LaneId lane_id) {
    return GetLaneSectionById(section_id).GetLanes().at(lane_id); // 获取指定车道段中的车道引用
  }

  // 根据车道段ID和车道ID获取车道的常量引用
  const Lane &Road::GetLaneById(SectionId section_id, LaneId lane_id) const {
    return const_cast<Road *>(this)->GetLaneById(section_id, lane_id); // 调用非常量版本
  }

  // 获取给定距离s处的下一个车道
  Lane *Road::GetNextLane(const double s, const LaneId lane_id) {
    auto upper = _lane_sections.upper_bound(s); // 获取距离s的上界车道段

    while (upper != _lane_sections.end()) { // 当上界车道段不为空时
      // 检查ID
   Lane *ptr = upper->second.GetLane(lane_id); // 获取与 lane_id 相关联的车道指针
if (ptr != nullptr) { // 如果车道指针不为空
    return ptr; // 返回该车道指针
}
++upper; // 移动到下一个元素

return nullptr; // 如果没有找到，返回空指针
}

// 获取在 's' 之前的车道
Lane *Road::GetPrevLane(const double s, const LaneId lane_id) {

    auto lower = _lane_sections.lower_bound(s); // 找到第一个大于或等于 s 的元素
    auto rlower = std::make_reverse_iterator(lower); // 创建反向迭代器

    while (rlower != _lane_sections.rend()) { // 遍历直到结束
        Lane *ptr = rlower->second.GetLane(lane_id); // 获取与 lane_id 相关联的车道指针
        if (ptr != nullptr) { // 如果车道指针不为空
            return ptr; // 返回该车道指针
        }
        ++rlower; // 移动到前一个元素
    }

    return nullptr; // 如果没有找到，返回空指针
}

// 获取具有指定 lane id 的开始段
LaneSection *Road::GetStartSection(LaneId id) {
    auto it = _lane_sections.begin(); // 从车道段的开始位置迭代
    while (it != _lane_sections.end()) { // 遍历直到结束
        Lane *ptr = it->second.GetLane(id); // 获取与 id 相关联的车道指针
        if (ptr != nullptr) { // 如果车道指针不为空
            return &(it->second); // 返回该车道段的地址
        }
        ++it; // 移动到下一个元素
    }
    return nullptr; // 如果没有找到，返回空指针
}

// 获取具有指定 lane id 的结束段
LaneSection *Road::GetEndSection(LaneId id) {
    auto it = _lane_sections.rbegin(); // 从车道段的结束位置反向迭代
    while (it != _lane_sections.rend()) { // 遍历直到结束
        Lane *ptr = it->second.GetLane(id); // 获取与 id 相关联的车道指针
        if (ptr != nullptr) { // 如果车道指针不为空
            return &(it->second); // 返回该车道段的地址
        }
        ++it; // 移动到前一个元素
    }
    return nullptr; // 如果没有找到，返回空指针
}

// 根据给定的 s 获取方向点
element::DirectedPoint Road::GetDirectedPointIn(const double s) const {
    const auto clamped_s = geom::Math::Clamp(s, 0.0, _length); // 将 s 限制在有效范围内
    const auto geometry = _info.GetInfo<element::RoadInfoGeometry>(clamped_s); // 获取几何信息

    const auto lane_offset = _info.GetInfo<element::RoadInfoLaneOffset>(clamped_s); // 获取车道偏移信息
    float offset = 0; // 初始化偏移量
    if(lane_offset){ // 如果存在车道偏移信息
        offset = static_cast<float>(lane_offset->GetPolynomial().Evaluate(clamped_s)); // 计算偏移量
    }
    // 应用道路的车道偏移记录
    element::DirectedPoint p = geometry->GetGeometry().PosFromDist(clamped_s - geometry->GetDistance()); // 计算位置
    // Unreal 的 Y 轴偏移（偏移量取负）
    p.ApplyLateralOffset(-offset); // 应用横向偏移

    // 应用道路的高程记录
    const auto elevation_info = GetElevationOn(s); // 获取高程信息
    p.location.z = static_cast<float>(elevation_info.Evaluate(s)); // 设置 z 坐标
    p.pitch = elevation_info.Tangent(s); // 设置俯仰角

    return p; // 返回方向点
}

// 根据给定的 s 获取方向点，不考虑车道偏移
element::DirectedPoint Road::GetDirectedPointInNoLaneOffset(const double s) const {
    const auto clamped_s = geom::Math::Clamp(s, 0.0, _length); // 将 s 限制在有效范围内
    const auto geometry = _info.GetInfo<element::RoadInfoGeometry>(clamped_s); // 获取几何信息

    element::DirectedPoint p = geometry->GetGeometry().PosFromDist(clamped_s - geometry->GetDistance()); // 计算位置

    // 应用道路的高程记录
    const auto elevation_info = GetElevationOn(s); // 获取高程信息
    p.location.z = static_cast<float>(elevation_info.Evaluate(s)); // 设置 z 坐标
    p.pitch = elevation_info.Tangent(s); // 设置俯仰角

    return p; // 返回方向点
}

// 获取距离给定位置最近的点
const std::pair<double, double> Road::GetNearestPoint(const geom::Location &loc) const {
    std::pair<double, double> last = { 0.0, std::numeric_limits<double>::max() }; // 初始化最近点和距离

    auto geom_info_list = _info.GetInfos<element::RoadInfoGeometry>(); // 获取几何信息列表
    decltype(geom_info_list)::iterator nearest_geom = geom_info_list.end(); // 初始化最近的几何信息指针

    for (auto g = geom_info_list.begin(); g != geom_info_list.end(); ++g) { // 遍历几何信息列表
        DEBUG_ASSERT(*g != nullptr); // 确保指针不为空
        auto dist = (*g)->GetGeometry().DistanceTo(loc); // 计算与位置的距离
        if (dist.second < last.second) { // 如果当前距离小于最近距离
            last = dist; // 更新最近距离
            nearest_geom = g; // 更新最近的几何信息指针
        }
    }

    for (auto g = geom_info_list.begin();
         g != geom_info_list.end() && g != nearest_geom;
         ++g) { // 遍历直到最近的几何信息
        DEBUG_ASSERT(*g != nullptr); // 确保指针不为空
        last.first += (*g)->GetGeometry().GetLength(); // 累加长度
    }

    return last; // 返回最近点和距离
}
  const std::pair<const Lane *, double> Road::GetNearestLane(
      const double s,
      const geom::Location &loc,
      uint32_t lane_type) const {
    using namespace carla::road::element; // 使用 carla::road::element 命名空间
    std::map<LaneId, const Lane *> lanes(GetLanesAt(s)); // 获取在 s 位置的所有车道

    // 获取右侧车道（负值车道）
    auto right_lanes = MakeListView(
        std::make_reverse_iterator(lanes.lower_bound(0)), lanes.rend()); // 创建右侧车道的视图
    // 获取左侧车道（正值车道）
    auto left_lanes = MakeListView(
        lanes.lower_bound(1), lanes.end()); // 创建左侧车道的视图

    const DirectedPoint dp_lane_zero = GetDirectedPointIn(s); // 获取在 s 处的方向点
    std::pair<const Lane *, double> result =
        std::make_pair(nullptr, std::numeric_limits<double>::max()); // 初始化结果为 null 和最大距离

    DirectedPoint current_dp = dp_lane_zero; // 当前方向点初始化为 dp_lane_zero
    for (const auto &lane : right_lanes) { // 遍历右侧车道
      const auto lane_width_info = lane.second->GetInfo<RoadInfoLaneWidth>(s); // 获取车道宽度信息
      const auto half_width = static_cast<float>(lane_width_info->GetPolynomial().Evaluate(s)) * 0.5f; // 计算半宽度

      current_dp.ApplyLateralOffset(half_width); // 应用半宽度偏移到当前方向点
      const auto current_dist = geom::Math::Distance(current_dp.location, loc); // 计算当前方向点到目标位置的距离

      // 如果当前方向点接近目标位置，说明我们在正确的方向上
      if (current_dist <= result.second) {
        // 仅考虑与类型标志匹配的车道作为候选结果
        if ((static_cast<uint32_t>(lane.second->GetType()) & lane_type) > 0) {
          result.first = &(*lane.second); // 更新结果中的车道指针
          result.second = current_dist; // 更新结果中的距离
        }
      } else {
        // 如果距离变远，说明我们可能在偏离方向
        break; // 退出循环
      }
      current_dp.ApplyLateralOffset(half_width); // 再次应用半宽度偏移
    }

    current_dp = dp_lane_zero; // 重置当前方向点为初始值
    for (const auto &lane : left_lanes) { // 遍历左侧车道
      const auto lane_width_info = lane.second->GetInfo<RoadInfoLaneWidth>(s); // 获取车道宽度信息
      const auto half_width = -static_cast<float>(lane_width_info->GetPolynomial().Evaluate(s)) * 0.5f; // 计算负半宽度

      current_dp.ApplyLateralOffset(half_width); // 应用负半宽度偏移到当前方向点
      const auto current_dist = geom::Math::Distance(current_dp.location, loc); // 计算当前方向点到目标位置的距离

      // 如果当前方向点接近目标位置，说明我们在正确的方向上
      if (current_dist <= result.second) {
        // 仅考虑与类型标志匹配的车道作为候选结果
        if ((static_cast<uint32_t>(lane.second->GetType()) & lane_type) > 0) {
          result.first = &(*lane.second); // 更新结果中的车道指针
          result.second = current_dist; // 更新结果中的距离
        }
      } else {
        // 如果距离变远，说明我们可能在偏离方向
        break; // 退出循环
      }
      current_dp.ApplyLateralOffset(half_width); // 再次应用负半宽度偏移
    }

    return result; // 返回最近的车道和距离
}

std::map<LaneId, const Lane *> Road::GetLanesAt(const double s) const {
    std::map<LaneId, const Lane *> map; // 初始化一个车道映射
    for (auto &&lane_section : GetLaneSectionsAt(s)) { // 遍历在 s 位置的车道段
      for (auto &&lane : lane_section.GetLanes()) { // 遍历车道段中的所有车道
        map[lane.first] = &(lane.second); // 将车道 ID 和车道指针存入映射
      }
    }
    return map; // 返回车道映射
}

} // road
} // carla