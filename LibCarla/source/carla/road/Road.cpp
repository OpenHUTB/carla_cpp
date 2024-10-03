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
      Lane *ptr = upper->second.GetLane(lane_id);
      if (ptr != nullptr) {
        return ptr;
      }
      ++upper;
    }

    return nullptr;
  }

  // get the lane on a section previous to 's'
  Lane *Road::GetPrevLane(const double s, const LaneId lane_id) {

    auto lower = _lane_sections.lower_bound(s);
    auto rlower = std::make_reverse_iterator(lower);

    while (rlower != _lane_sections.rend()) {
      // check id
      Lane *ptr = rlower->second.GetLane(lane_id);
      if (ptr != nullptr) {
        return ptr;
      }
      ++rlower;
    }

    return nullptr;
  }

  // get the start and end section with a lan id
  LaneSection *Road::GetStartSection(LaneId id) {
    auto it = _lane_sections.begin();
    while (it != _lane_sections.end()) {
      // check id
      Lane *ptr = it->second.GetLane(id);
      if (ptr != nullptr) {
        return &(it->second);
      }
      ++it;
    }
    return nullptr;
  }

  LaneSection *Road::GetEndSection(LaneId id) {
    auto it = _lane_sections.rbegin();
    while (it != _lane_sections.rend()) {
      // check id
      Lane *ptr = it->second.GetLane(id);
      if (ptr != nullptr) {
        return &(it->second);
      }
      ++it;
    }
    return nullptr;
  }

  element::DirectedPoint Road::GetDirectedPointIn(const double s) const {
    const auto clamped_s = geom::Math::Clamp(s, 0.0, _length);
    const auto geometry = _info.GetInfo<element::RoadInfoGeometry>(clamped_s);

    const auto lane_offset = _info.GetInfo<element::RoadInfoLaneOffset>(clamped_s);
    float offset = 0;
    if(lane_offset){
      offset = static_cast<float>(lane_offset->GetPolynomial().Evaluate(clamped_s));
    }
    // Apply road's lane offset record
    element::DirectedPoint p = geometry->GetGeometry().PosFromDist(clamped_s - geometry->GetDistance());
    // Unreal's Y axis hack (the minus on the offset)
    p.ApplyLateralOffset(-offset);

    // Apply road's elevation record
    const auto elevation_info = GetElevationOn(s);
    p.location.z = static_cast<float>(elevation_info.Evaluate(s));
    p.pitch = elevation_info.Tangent(s);

    return p;
  }

  element::DirectedPoint Road::GetDirectedPointInNoLaneOffset(const double s) const {
    const auto clamped_s = geom::Math::Clamp(s, 0.0, _length);
    const auto geometry = _info.GetInfo<element::RoadInfoGeometry>(clamped_s);

    element::DirectedPoint p = geometry->GetGeometry().PosFromDist(clamped_s - geometry->GetDistance());

    // Apply road's elevation record
    const auto elevation_info = GetElevationOn(s);
    p.location.z = static_cast<float>(elevation_info.Evaluate(s));
    p.pitch = elevation_info.Tangent(s);

    return p;
  }

  const std::pair<double, double> Road::GetNearestPoint(const geom::Location &loc) const {
    std::pair<double, double> last = { 0.0, std::numeric_limits<double>::max() };

    auto geom_info_list = _info.GetInfos<element::RoadInfoGeometry>();
    decltype(geom_info_list)::iterator nearest_geom = geom_info_list.end();

    for (auto g = geom_info_list.begin(); g != geom_info_list.end(); ++g) {
      DEBUG_ASSERT(*g != nullptr);
      auto dist = (*g)->GetGeometry().DistanceTo(loc);
      if (dist.second < last.second) {
        last = dist;
        nearest_geom = g;
      }
    }

    for (auto g = geom_info_list.begin();
        g != geom_info_list.end() && g != nearest_geom;
        ++g) {
      DEBUG_ASSERT(*g != nullptr);
      last.first += (*g)->GetGeometry().GetLength();
    }

    return last;
  }

  const std::pair<const Lane *, double> Road::GetNearestLane(
      const double s,
      const geom::Location &loc,
      uint32_t lane_type) const {
    using namespace carla::road::element;
    std::map<LaneId, const Lane *> lanes(GetLanesAt(s));
    // negative right lanes
    auto right_lanes = MakeListView(
        std::make_reverse_iterator(lanes.lower_bound(0)), lanes.rend());
    // positive left lanes
    auto left_lanes = MakeListView(
        lanes.lower_bound(1), lanes.end());

    const DirectedPoint dp_lane_zero = GetDirectedPointIn(s);
    std::pair<const Lane *, double> result =
        std::make_pair(nullptr, std::numeric_limits<double>::max());

    DirectedPoint current_dp = dp_lane_zero;
    for (const auto &lane : right_lanes) {
      const auto lane_width_info = lane.second->GetInfo<RoadInfoLaneWidth>(s);
      const auto half_width = static_cast<float>(lane_width_info->GetPolynomial().Evaluate(s)) * 0.5f;

      current_dp.ApplyLateralOffset(half_width);
      const auto current_dist = geom::Math::Distance(current_dp.location, loc);

      // if the current_dp is near to loc, we are in the right way
      if (current_dist <= result.second) {
        // only consider the lanes that match the type flag for result
        // candidates
        if ((static_cast<uint32_t>(lane.second->GetType()) & lane_type) > 0) {
          result.first = &(*lane.second);
          result.second = current_dist;
        }
      } else {
        // elsewhere, we are be moving away
        break;
      }
      current_dp.ApplyLateralOffset(half_width);
    }

    current_dp = dp_lane_zero;
    for (const auto &lane : left_lanes) {
      const auto lane_width_info = lane.second->GetInfo<RoadInfoLaneWidth>(s);
      const auto half_width = -static_cast<float>(lane_width_info->GetPolynomial().Evaluate(s)) * 0.5f;

      current_dp.ApplyLateralOffset(half_width);
      const auto current_dist = geom::Math::Distance(current_dp.location, loc);

      // if the current_dp is near to loc, we are in the right way
      if (current_dist <= result.second) {
        // only consider the lanes that match the type flag for result
        // candidates
        if ((static_cast<uint32_t>(lane.second->GetType()) & lane_type) > 0) {
          result.first = &(*lane.second);
          result.second = current_dist;
        }
      } else {
        // elsewhere, we are be moving away
        break;
      }
      current_dp.ApplyLateralOffset(half_width);
    }

    return result;
  }

  std::map<LaneId, const Lane *> Road::GetLanesAt(const double s) const {
    std::map<LaneId, const Lane *> map;
    for (auto &&lane_section : GetLaneSectionsAt(s)) {
      for (auto &&lane : lane_section.GetLanes()) {
        map[lane.first] = &(lane.second);
      }
    }
    return map;
  }

} // road
} // carla
