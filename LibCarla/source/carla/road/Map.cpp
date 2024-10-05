// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/road/Map.h" // 导入地图相关的头文件
#include "carla/Exception.h" // 导入异常处理的头文件
#include "carla/geom/Math.h" // 导入数学计算相关的头文件
#include "carla/geom/Vector3D.h" // 导入三维向量相关的头文件
#include "carla/road/MeshFactory.h" // 导入网格工厂的头文件
#include "carla/road/Deformation.h" // 导入变形相关的头文件
#include "carla/road/element/LaneCrossingCalculator.h" // 导入车道交叉计算器的头文件
#include "carla/road/element/RoadInfoCrosswalk.h" // 导入人行横道信息的头文件
#include "carla/road/element/RoadInfoElevation.h" // 导入道路高度信息的头文件
#include "carla/road/element/RoadInfoGeometry.h" // 导入道路几何信息的头文件
#include "carla/road/element/RoadInfoLaneOffset.h" // 导入车道偏移信息的头文件
#include "carla/road/element/RoadInfoLaneWidth.h" // 导入车道宽度信息的头文件
#include "carla/road/element/RoadInfoMarkRecord.h" // 导入道路标记记录信息的头文件
#include "carla/road/element/RoadInfoSpeed.h" // 导入道路速度信息的头文件
#include "carla/road/element/RoadInfoSignal.h" // 导入道路信号信息的头文件

#include "marchingcube/MeshReconstruction.h" // 导入网格重建的头文件

#include <vector> // 导入向量库
#include <unordered_map> // 导入无序映射库
#include <stdexcept> // 导入标准异常库
#include <chrono> // 导入时间相关库
#include <thread> // 导入线程相关库
#include <iomanip> // 导入格式化输入输出库
#include <cmath> // 导入数学库

namespace carla {
namespace road {

  using namespace carla::road::element; // 使用carla::road::element命名空间中的内容

  /// 我们使用这个epsilon值将路径点从车道边缘移动开，以避免浮点精度错误。
  static constexpr double EPSILON = 10.0 * std::numeric_limits<double>::epsilon();

  // ===========================================================================
  // -- 静态本地方法 ----------------------------------------------------------
  // ===========================================================================

  // 合并两个向量
  template <typename T>
  static std::vector<T> ConcatVectors(std::vector<T> dst, std::vector<T> src) {
    if (src.size() > dst.size()) { // 如果源向量大小大于目标向量
      return ConcatVectors(src, dst); // 递归调用合并源和目标
    }
    dst.insert(
        dst.end(), // 在目标向量末尾插入源向量的元素
        std::make_move_iterator(src.begin()), // 移动源向量的开始迭代器
        std::make_move_iterator(src.end())); // 移动源向量的结束迭代器
    return dst; // 返回合并后的向量
  }

  // 获取车道开始位置的距离
  static double GetDistanceAtStartOfLane(const Lane &lane) {
    if (lane.GetId() <= 0) { // 如果车道ID小于等于0
      return lane.GetDistance() + 10.0 * EPSILON; // 返回距离加上一个小的偏移量
    } else {
      return lane.GetDistance() + lane.GetLength() - 10.0 * EPSILON; // 返回距离加上车道长度减去偏移量
    }
  }

  // 获取车道结束位置的距离
  static double GetDistanceAtEndOfLane(const Lane &lane) {
    if (lane.GetId() > 0) { // 如果车道ID大于0
      return lane.GetDistance() + 10.0 * EPSILON; // 返回距离加上一个小的偏移量
    } else {
      return lane.GetDistance() + lane.GetLength() - 10.0 * EPSILON; // 返回距离加上车道长度减去偏移量
    }
  }

  /// 返回每个可行驶车道的路径点 @a lane_section.
  template <typename FuncT>
  static void ForEachDrivableLaneImpl(
      RoadId road_id, // 道路ID
      const LaneSection &lane_section, // 车道段
      double distance, // 距离
      FuncT &&func) { // 函数对象
    for (const auto &pair : lane_section.GetLanes()) { // 遍历车道
      const auto &lane = pair.second; // 获取车道
      if (lane.GetId() == 0) { // 如果车道ID为0，跳过
        continue;
      }
      if ((static_cast<uint32_t>(lane.GetType()) & static_cast<uint32_t>(Lane::LaneType::Driving)) > 0) { // 如果是可驾驶类型的车道
        std::forward<FuncT>(func)(Waypoint{ // 调用传入的函数
            road_id, // 道路ID
            lane_section.GetId(), // 车道段ID
            lane.GetId(), // 车道ID
            distance < 0.0 ? GetDistanceAtStartOfLane(lane) : distance}); // 计算距离
      }
    }
  }

  // 遍历指定类型的车道
  template <typename FuncT>
  static void ForEachLaneImpl(
      RoadId road_id, // 道路ID
      const LaneSection &lane_section, // 车道段
      double distance, // 距离
      Lane::LaneType lane_type, // 车道类型
      FuncT &&func) { // 函数对象
    for (const auto &pair : lane_section.GetLanes()) { // 遍历车道
      const auto &lane = pair.second; // 获取车道
      if (lane.GetId() == 0) { // 如果车道ID为0，跳过
        continue;
      }
      if ((static_cast<int32_t>(lane.GetType()) & static_cast<int32_t>(lane_type)) > 0) { // 如果是指定类型的车道
        std::forward<FuncT>(func)(Waypoint{ // 调用传入的函数
            road_id, // 道路ID
            lane_section.GetId(), // 车道段ID
            lane.GetId(), // 车道ID
            distance < 0.0 ? GetDistanceAtStartOfLane(lane) : distance}); // 计算距离
      }
    }
  }

  /// 返回每个可行驶车道的路径点，遍历所有车道段 @a road.
  template <typename FuncT>
  static void ForEachDrivableLane(const Road &road, FuncT &&func) {
    for (const auto &lane_section : road.GetLaneSections()) { // 遍历道路的所有车道段
      ForEachDrivableLaneImpl( // 调用实现函数
          road.GetId(), // 道路ID
          lane_section, // 车道段
          -1.0, // 在车道起点位置
          std::forward<FuncT>(func)); // 调用传入的函数
    }
  }

 /// 对于每个指定类型的车道，返回该道路的每个车道段的一个航点
template <typename FuncT>
static void ForEachLane(const Road &road, Lane::LaneType lane_type, FuncT &&func) {
    for (const auto &lane_section : road.GetLaneSections()) { // 遍历道路的每个车道段
        ForEachLaneImpl(
            road.GetId(), // 获取道路ID
            lane_section, // 当前车道段
            -1.0, // 在车道起始位置
            lane_type, // 指定的车道类型
            std::forward<FuncT>(func)); // 执行提供的函数
    }
}

/// 返回在指定距离上每个可驾驶车道的一个航点
template <typename FuncT>
static void ForEachDrivableLaneAt(const Road &road, double distance, FuncT &&func) {
    for (const auto &lane_section : road.GetLaneSectionsAt(distance)) { // 遍历指定距离的每个车道段
        ForEachDrivableLaneImpl(
            road.GetId(), // 获取道路ID
            lane_section, // 当前车道段
            distance, // 指定的距离
            std::forward<FuncT>(func)); // 执行提供的函数
    }
}

/// 假定 road_id 和 section_id 是有效的
static bool IsLanePresent(const MapData &data, Waypoint waypoint) {
    const auto &section = data.GetRoad(waypoint.road_id).GetLaneSectionById(waypoint.section_id); // 获取指定的车道段
    return section.ContainsLane(waypoint.lane_id); // 检查车道是否存在
}

// ===========================================================================
// -- 地图: 几何 -------------------------------------------------------------
// ===========================================================================

boost::optional<Waypoint> Map::GetClosestWaypointOnRoad(
    const geom::Location &pos,
    int32_t lane_type) const {
    std::vector<Rtree::TreeElement> query_result =
        _rtree.GetNearestNeighboursWithFilter(Rtree::BPoint(pos.x, pos.y, pos.z), // 获取与位置最近的邻居节点
        [&](Rtree::TreeElement const &element) {
            const Lane &lane = GetLane(element.second.first); // 获取车道
            return (lane_type & static_cast<int32_t>(lane.GetType())) > 0; // 检查车道类型是否匹配
        });

    if (query_result.size() == 0) { // 如果没有找到结果
        return boost::optional<Waypoint>{}; // 返回空的航点
    }

    Rtree::BSegment segment = query_result.front().first; // 获取最近的线段
    Rtree::BPoint s1 = segment.first; // 线段的起点
    Rtree::BPoint s2 = segment.second; // 线段的终点
    auto distance_to_segment = geom::Math::DistanceSegmentToPoint(pos,
        geom::Vector3D(s1.get<0>(), s1.get<1>(), s1.get<2>()), // 计算点到线段的距离
        geom::Vector3D(s2.get<0>(), s2.get<1>(), s2.get<2>()));

    Waypoint result_start = query_result.front().second.first; // 最近的起始航点
    Waypoint result_end = query_result.front().second.second; // 最近的结束航点

    if (result_start.lane_id < 0) { // 如果起始航点的车道ID小于0
        double delta_s = distance_to_segment.first; // 计算距离差
        double final_s = result_start.s + delta_s; // 计算最终的s值
        if (final_s >= result_end.s) { // 如果超出结束航点的s值
            return result_end; // 返回结束航点
        } else if (delta_s <= 0) { // 如果距离差小于等于0
            return result_start; // 返回起始航点
        } else {
            return GetNext(result_start, delta_s).front(); // 返回下一个航点
        }
    } else { // 如果起始航点的车道ID大于等于0
        double delta_s = distance_to_segment.first; // 计算距离差
        double final_s = result_start.s - delta_s; // 计算最终的s值
        if (final_s <= result_end.s) { // 如果不超过结束航点的s值
            return result_end; // 返回结束航点
        } else if (delta_s <= 0) { // 如果距离差小于等于0
            return result_start; // 返回起始航点
        } else {
            return GetNext(result_start, delta_s).front(); // 返回下一个航点
        }
    }
}

boost::optional<Waypoint> Map::GetWaypoint(
    const geom::Location &pos,
    int32_t lane_type) const {
    boost::optional<Waypoint> w = GetClosestWaypointOnRoad(pos, lane_type); // 获取最近的航点

    if (!w.has_value()) { // 如果没有找到航点
        return w; // 返回空
    }

    const auto dist = geom::Math::Distance2D(ComputeTransform(*w).location, pos); // 计算输入位置与航点之间的距离
    const auto lane_width_info = GetLane(*w).GetInfo<RoadInfoLaneWidth>(w->s); // 获取车道宽度信息
    const auto half_lane_width =
        lane_width_info->GetPolynomial().Evaluate(w->s) * 0.5; // 计算车道的一半宽度

    if (dist < half_lane_width) { // 如果距离小于半车道宽度
        return w; // 返回航点
    }

    return boost::optional<Waypoint>{}; // 否则返回空
}

boost::optional<Waypoint> Map::GetWaypoint(
    RoadId road_id,
    LaneId lane_id,
    float s) const {

    // 用已知参数定义航点
    Waypoint waypoint;
    waypoint.road_id = road_id; // 设置道路ID
    waypoint.lane_id = lane_id; // 设置车道ID
    waypoint.s = s; // 设置s参数

    // 检查道路
    if (!_data.ContainsRoad(waypoint.road_id)) { // 如果数据中不包含该道路
        return boost::optional<Waypoint>{}; // 返回空航点
    }
    const Road &road = _data.GetRoad(waypoint.road_id); // 获取对应道路


// 检查's'的距离
if (s < 0.0f || s >= road.GetLength()) {
  return boost::optional<Waypoint>{}; // 如果s不在有效范围内，返回空的Waypoint
}

// 检查车道段
bool lane_found = false; // 初始化车道找到标志为false
for (auto &section : road.GetLaneSectionsAt(s)) { // 遍历道路中指定位置的车道段
  if (section.ContainsLane(lane_id)) { // 检查当前段是否包含特定车道
    waypoint.section_id = section.GetId(); // 设置Waypoint的段ID
    lane_found = true; // 找到车道，标志设为true
    break; // 结束循环
  }
}

// 检查车道ID
if (!lane_found) { // 如果没有找到车道
  return boost::optional<Waypoint>{}; // 返回空的Waypoint
}

return waypoint; // 返回找到的Waypoint

// ===========================================================================
// -- Map: 地图信息 -----------------------------------------------------------
// ===========================================================================

// 获取车道类型
Lane::LaneType Map::GetLaneType(const Waypoint waypoint) const {
  return GetLane(waypoint).GetType(); // 返回指定Waypoint的车道类型
}

// 获取车道宽度
double Map::GetLaneWidth(const Waypoint waypoint) const {
  const auto s = waypoint.s; // 从Waypoint中获取s值

  const auto &lane = GetLane(waypoint); // 获取对应的车道
  RELEASE_ASSERT(lane.GetRoad() != nullptr); // 确保车道存在
  RELEASE_ASSERT(s <= lane.GetRoad()->GetLength()); // 确保s在车道长度范围内

  const auto lane_width_info = lane.GetInfo<RoadInfoLaneWidth>(s); // 获取车道宽度信息
  RELEASE_ASSERT(lane_width_info != nullptr); // 确保车道宽度信息存在

  return lane_width_info->GetPolynomial().Evaluate(s); // 计算并返回车道宽度
}

// 获取交叉口ID
JuncId Map::GetJunctionId(RoadId road_id) const {
  return _data.GetRoad(road_id).GetJunctionId(); // 返回指定道路的交叉口ID
}

// 检查是否为交叉口
bool Map::IsJunction(RoadId road_id) const {
  return _data.GetRoad(road_id).IsJunction(); // 返回指定道路是否为交叉口
}

// 获取标记记录
std::pair<const RoadInfoMarkRecord *, const RoadInfoMarkRecord *>
    Map::GetMarkRecord(const Waypoint waypoint) const {
  // 如果车道ID为0，返回一对空指针
  if (waypoint.lane_id == 0)
    return std::make_pair(nullptr, nullptr);

  const auto s = waypoint.s; // 从Waypoint中获取s值

  const auto &current_lane = GetLane(waypoint); // 获取当前车道
  RELEASE_ASSERT(current_lane.GetRoad() != nullptr); // 确保车道存在
  RELEASE_ASSERT(s <= current_lane.GetRoad()->GetLength()); // 确保s在车道长度范围内

  const auto inner_lane_id = waypoint.lane_id < 0 ?
      waypoint.lane_id + 1 : // 计算内侧车道ID
      waypoint.lane_id - 1;

  const auto &inner_lane = current_lane.GetRoad()->GetLaneById(waypoint.section_id, inner_lane_id); // 获取内侧车道

  auto current_lane_info = current_lane.GetInfo<RoadInfoMarkRecord>(s); // 获取当前车道的标记记录
  auto inner_lane_info = inner_lane.GetInfo<RoadInfoMarkRecord>(s); // 获取内侧车道的标记记录

  return std::make_pair(current_lane_info, inner_lane_info); // 返回一对标记记录
}

// 获取指定距离内的信号
std::vector<Map::SignalSearchData> Map::GetSignalsInDistance(
    Waypoint waypoint, double distance, bool stop_at_junction) const {

  const auto &lane = GetLane(waypoint); // 获取Waypoint对应的车道
  const bool forward = (waypoint.lane_id <= 0); // 判断移动方向
  const double signed_distance = forward ? distance : -distance; // 根据方向设置带符号的距离
  const double relative_s = waypoint.s - lane.GetDistance(); // 计算相对s
  const double remaining_lane_length = forward ? lane.GetLength() - relative_s : relative_s; // 计算剩余车道长度
  DEBUG_ASSERT(remaining_lane_length >= 0.0); // 确保剩余长度非负

  auto &road =_data.GetRoad(waypoint.road_id); // 获取对应的道路
  std::vector<SignalSearchData> result; // 存储结果信号数据的向量

  // 如果减去距离后仍在同一车道，则返回同一Waypoint和额外距离
  if (distance <= remaining_lane_length) {
    auto signals = road.GetInfosInRange<RoadInfoSignal>(
        waypoint.s, waypoint.s + signed_distance); // 在指定范围内获取信号信息
    for(auto* signal : signals){ // 遍历所有信号
      double distance_to_signal = 0;
      if (waypoint.lane_id < 0){ // 判断车道方向
        distance_to_signal = signal->GetDistance() - waypoint.s; // 计算信号与Waypoint的距离
      } else {
        distance_to_signal = waypoint.s - signal->GetDistance(); // 计算信号与Waypoint的距离
      }
      // 检查信号是否影响Waypoint
      bool is_valid = false; // 初始化有效性标志为false
      for (auto &validity : signal->GetValidities()) { // 遍历信号的有效性范围
        if (waypoint.lane_id >= validity._from_lane && // 检查Waypoint的lane_id是否在有效范围内
            waypoint.lane_id <= validity._to_lane) {
          is_valid = true; // 有效性标志设为true
          break; // 结束循环
        }
      }
 if(!is_valid){ // 如果信号不有效
    continue; // 跳过当前循环，继续下一个信号
}
if (distance_to_signal == 0) { // 如果信号与Waypoint的距离为0
    result.emplace_back(SignalSearchData // 添加信号数据到结果中
        {signal, waypoint, // 将信号和Waypoint放入SignalSearchData结构中
        distance_to_signal}); // 记录距离为0
} else {
    result.emplace_back(SignalSearchData // 添加信号数据到结果中
        {signal, GetNext(waypoint, distance_to_signal).front(), // 获取下一个Waypoint并存储
        distance_to_signal}); // 记录与信号的距离
}

}
return result; // 返回结果

const double signed_remaining_length = forward ? remaining_lane_length : -remaining_lane_length; // 根据方向设置带符号的剩余长度

//result = road.GetInfosInRange<RoadInfoSignal>(waypoint.s, waypoint.s + signed_remaining_length); // 注释掉的代码：获取信号信息

auto signals = road.GetInfosInRange<RoadInfoSignal>( // 在指定范围内获取信号信息
    waypoint.s, waypoint.s + signed_remaining_length);
for(auto* signal : signals){ // 遍历所有信号
    double distance_to_signal = 0; // 初始化信号与Waypoint的距离
    if (waypoint.lane_id < 0){ // 判断车道方向
        distance_to_signal = signal->GetDistance() - waypoint.s; // 计算信号与Waypoint的距离
    } else {
        distance_to_signal = waypoint.s - signal->GetDistance(); // 计算信号与Waypoint的距离
    }
    // 检查信号是否影响Waypoint
    bool is_valid = false; // 初始化有效性标志为false
    for (auto &validity : signal->GetValidities()) { // 遍历信号的有效性范围
        if (waypoint.lane_id >= validity._from_lane && // 检查Waypoint的lane_id是否在有效范围内
            waypoint.lane_id <= validity._to_lane) {
            is_valid = true; // 有效性标志设为true
            break; // 结束循环
        }
    }
    if(!is_valid){ // 如果信号无效
        continue; // 跳过当前信号
    }
    if (distance_to_signal == 0) { // 如果信号与Waypoint的距离为0
        result.emplace_back(SignalSearchData // 添加信号数据到结果中
            {signal, waypoint, // 将信号和Waypoint放入SignalSearchData结构中
            distance_to_signal}); // 记录距离为0
    } else {
        result.emplace_back(SignalSearchData // 添加信号数据到结果中
            {signal, GetNext(waypoint, distance_to_signal).front(), // 获取下一个Waypoint并存储
            distance_to_signal}); // 记录与信号的距离
    }
}

// 如果剩余车道长度用尽，必须查看后继
for (auto &successor : GetSuccessors(waypoint)) { // 遍历Waypoint的后继节点
    if(_data.GetRoad(successor.road_id).IsJunction() && stop_at_junction){ // 如果后继是交叉口并且需要停止
        continue; // 跳过此后继
    }
    auto& sucessor_lane = _data.GetRoad(successor.road_id). // 获取后继车道
        GetLaneByDistance(successor.s, successor.lane_id);
    if (successor.lane_id < 0) { // 如果后继车道ID为负
        successor.s = sucessor_lane.GetDistance(); // 设置后继s为车道的起始距离
    } else {
        successor.s = sucessor_lane.GetDistance() + sucessor_lane.GetLength(); // 设置后继s为车道的结束距离
    }
    auto sucessor_signals = GetSignalsInDistance( // 获取后继信号在指定距离内的信号
        successor, distance - remaining_lane_length, stop_at_junction);
    for(auto& signal : sucessor_signals){ // 遍历后继信号
        signal.accumulated_s += remaining_lane_length; // 更新累积的s值
    }
    result = ConcatVectors(result, sucessor_signals); // 合并结果信号和后继信号
}
return result; // 返回结果
}

std::vector<const element::RoadInfoSignal*> // 获取所有信号引用
Map::GetAllSignalReferences() const {
    std::vector<const element::RoadInfoSignal*> result; // 存储信号引用的向量
    for (const auto& road_pair : _data.GetRoads()) { // 遍历所有道路
        const auto &road = road_pair.second; // 获取道路对象
        auto road_infos = road.GetInfos<element::RoadInfoSignal>(); // 获取道路上的信号信息
        for(const auto* road_info : road_infos) { // 遍历所有信号信息
            result.push_back(road_info); // 将信号信息添加到结果向量中
        }
    }
    return result; // 返回所有信号引用
}

std::vector<LaneMarking> Map::CalculateCrossedLanes( // 计算交叉的车道
    const geom::Location &origin, // 起点位置
    const geom::Location &destination) const { // 终点位置
    return LaneCrossingCalculator::Calculate(*this, origin, destination); // 调用车道交叉计算器计算结果
}


std::vector<geom::Location> Map::GetAllCrosswalkZones() const {
    std::vector<geom::Location> result; // 存储所有人行横道区域的位置

    for (const auto &pair : _data.GetRoads()) { // 遍历所有道路
        const auto &road = pair.second; // 获取道路信息
        std::vector<const RoadInfoCrosswalk *> crosswalks = road.GetInfos<RoadInfoCrosswalk>(); // 获取道路上的人行横道信息
        if (crosswalks.size() > 0) { // 如果存在人行横道
            for (auto crosswalk : crosswalks) { // 遍历每个横道
                std::vector<geom::Location> points; // 存储点的位置
                Waypoint waypoint; // 创建一个航点
                geom::Transform base; // 存储基准变换
                for (const auto &section : road.GetLaneSectionsAt(crosswalk->GetS())) { // 获取横道位置的车道段
                    for (const auto &lane : section.GetLanes()) { // 遍历车道
                        if (lane.first == 0) { // 如果是中心线
                            waypoint.road_id = pair.first; // 设置道路ID
                            waypoint.section_id = section.GetId(); // 设置车道段ID
                            waypoint.lane_id = 0; // 设置车道ID为0（中心车道）
                            waypoint.s = crosswalk->GetS(); // 设置横道的S位置
                            base = ComputeTransform(waypoint); // 计算基准变换
                        }
                    }
                }

                // 移动到垂直方向（'t'）
                geom::Transform pivot = base; // 复制基准变换
                pivot.rotation.yaw -= geom::Math::ToDegrees<float>(static_cast<float>(crosswalk->GetHeading())); // 调整朝向
                pivot.rotation.yaw -= 90; // 旋转90度，移动到横道的侧面
                geom::Vector3D v(static_cast<float>(crosswalk->GetT()), 0.0f, 0.0f); // 创建一个向量
                pivot.TransformPoint(v); // 转换该点
                // 恢复支点位置和方向
                pivot = base; // 恢复为基准变换
                pivot.location = v; // 设置位置为刚才转换过的位置
                pivot.rotation.yaw -= geom::Math::ToDegrees<float>(static_cast<float>(crosswalk->GetHeading())); // 再次调整朝向

                // 计算所有的角落
                for (auto corner : crosswalk->GetPoints()) { // 遍历横道的每一个角落
                    geom::Vector3D v2(
                        static_cast<float>(corner.u), // 获取角落的u坐标
                        static_cast<float>(corner.v), // 获取角落的v坐标
                        static_cast<float>(corner.z)); // 获取角落的z坐标
                    // 设置宽度以确保与人行道接触（以防有排水沟区域）
                    if (corner.u < 0) { // 如果u坐标小于0
                        v2.x -= 1.0f; // 向左扩展
                    } else { // 如果u坐标大于等于0
                        v2.x += 1.0f; // 向右扩展
                    }
                    pivot.TransformPoint(v2); // 转换角落的位置
                    result.push_back(v2); // 将角落位置添加到结果中
                }
            }
        }
    }
    return result; // 返回所有人行横道区域的位置
}

// ===========================================================================
// -- Map: 航点生成 ---------------------------------------------------------
// ===========================================================================

std::vector<Waypoint> Map::GetSuccessors(const Waypoint waypoint) const {
    const auto &next_lanes = GetLane(waypoint).GetNextLanes(); // 获取下一个车道
    std::vector<Waypoint> result; // 存储结果
    result.reserve(next_lanes.size()); // 预留空间
    for (auto *next_lane : next_lanes) { // 遍历每个下一个车道
        RELEASE_ASSERT(next_lane != nullptr); // 确保车道不为空
        const auto lane_id = next_lane->GetId(); // 获取车道ID
        RELEASE_ASSERT(lane_id != 0); // 确保车道ID有效
        const auto *section = next_lane->GetLaneSection(); // 获取车道段
        RELEASE_ASSERT(section != nullptr); // 确保车道段不为空
        const auto *road = next_lane->GetRoad(); // 获取道路
        RELEASE_ASSERT(road != nullptr); // 确保道路不为空
        const auto distance = GetDistanceAtStartOfLane(*next_lane); // 获取下一个车道起始位置的距离
        result.emplace_back(Waypoint{road->GetId(), section->GetId(), lane_id, distance}); // 添加航点到结果中
    }
    return result; // 返回下一个航点
}

std::vector<Waypoint> Map::GetPredecessors(const Waypoint waypoint) const {
    const auto &prev_lanes = GetLane(waypoint).GetPreviousLanes(); // 获取前一个车道
    std::vector<Waypoint> result; // 存储结果
    result.reserve(prev_lanes.size()); // 预留空间
    for (auto *next_lane : prev_lanes) { // 遍历每个前一个车道
        RELEASE_ASSERT(next_lane != nullptr); // 确保车道不为空
        const auto lane_id = next_lane->GetId(); // 获取车道ID
        RELEASE_ASSERT(lane_id != 0); // 确保车道ID有效
        const auto *section = next_lane->GetLaneSection(); // 获取车道段
        RELEASE_ASSERT(section != nullptr); // 确保车道段不为空
        const auto *road = next_lane->GetRoad(); // 获取道路
        RELEASE_ASSERT(road != nullptr); // 确保道路不为空
        const auto distance = GetDistanceAtEndOfLane(*next_lane); // 获取前一个车道末端位置的距离
        result.emplace_back(Waypoint{road->GetId(), section->GetId(), lane_id, distance}); // 添加航点到结果中
    }
    return result; // 返回前一个航点
}

std::vector<Waypoint> Map::GetNext(
      const Waypoint waypoint,
      const double distance) const {
    RELEASE_ASSERT(distance > 0.0); // 确保距离大于0
    if (distance <= EPSILON) { // 如果距离很小（近似为0）
      return {waypoint}; // 返回当前的waypoint
    }
    const auto &lane = GetLane(waypoint); // 获取当前waypoint所在的车道
    const bool forward = (waypoint.lane_id <= 0); // 判断移动方向（正向或反向）
    const double signed_distance = forward ? distance : -distance; // 根据方向确定带符号的距离
    const double relative_s = waypoint.s - lane.GetDistance(); // 计算相对位置s
    const double remaining_lane_length = forward ? lane.GetLength() - relative_s : relative_s; // 剩余车道长度
    DEBUG_ASSERT(remaining_lane_length >= 0.0); // 确保剩余车道长度非负

    // 如果在同一车道内，返回增加了距离的waypoint
    if (distance <= remaining_lane_length) {
      Waypoint result = waypoint; // 创建结果waypoint
      result.s += signed_distance; // 更新s值
      result.s += forward ? -EPSILON : EPSILON; // 调整s值以避免浮点数精度问题
      RELEASE_ASSERT(result.s > 0.0); // 确保s值大于0
      return { result }; // 返回结果
    }

    // 如果没有剩余车道长度，则需要转到后继节点
    std::vector<Waypoint> result; // 存储结果的vector
    for (const auto &successor : GetSuccessors(waypoint)) { // 遍历所有后继waypoints
      DEBUG_ASSERT(
          successor.road_id != waypoint.road_id || // 确保不在同一路段
          successor.section_id != waypoint.section_id || // 确保不在同一部分
          successor.lane_id != waypoint.lane_id); // 确保不在同一车道
      result = ConcatVectors(result, GetNext(successor, distance - remaining_lane_length)); // 递归获取下一个waypoint
    }
    return result; // 返回所有找到的waypoints
  }

  std::vector<Waypoint> Map::GetPrevious(
      const Waypoint waypoint,
      const double distance) const {
    RELEASE_ASSERT(distance > 0.0); // 确保距离大于0
    if (distance <= EPSILON) { // 如果距离很小（近似为0）
      return {waypoint}; // 返回当前的waypoint
    }
    const auto &lane = GetLane(waypoint); // 获取当前waypoint所在的车道
    const bool forward = !(waypoint.lane_id <= 0); // 判断移动方向（正向或反向）
    const double signed_distance = forward ? distance : -distance; // 根据方向确定带符号的距离
    const double relative_s = waypoint.s - lane.GetDistance(); // 计算相对位置s
    const double remaining_lane_length = forward ? lane.GetLength() - relative_s : relative_s; // 剩余车道长度
    DEBUG_ASSERT(remaining_lane_length >= 0.0); // 确保剩余车道长度非负

    // 如果在同一车道内，返回增加了距离的waypoint
    if (distance <= remaining_lane_length) {
      Waypoint result = waypoint; // 创建结果waypoint
      result.s += signed_distance; // 更新s值
      result.s += forward ? -EPSILON : EPSILON; // 调整s值以避免浮点数精度问题
      RELEASE_ASSERT(result.s > 0.0); // 确保s值大于0
      return { result }; // 返回结果
    }

    // 如果没有剩余车道长度，则需要转到前驱节点
    std::vector<Waypoint> result; // 存储结果的vector
    for (const auto &successor : GetPredecessors(waypoint)) { // 遍历所有前驱waypoints
      DEBUG_ASSERT(
          successor.road_id != waypoint.road_id || // 确保不在同一路段
          successor.section_id != waypoint.section_id || // 确保不在同一部分
          successor.lane_id != waypoint.lane_id); // 确保不在同一车道
      result = ConcatVectors(result, GetPrevious(successor, distance - remaining_lane_length)); // 递归获取前一个waypoint
    }
    return result; // 返回所有找到的waypoints
  }

  boost::optional<Waypoint> Map::GetRight(Waypoint waypoint) const {
    RELEASE_ASSERT(waypoint.lane_id != 0); // 确保车道ID不为0
    if (waypoint.lane_id > 0) { // 如果当前车道ID为正
      ++waypoint.lane_id; // 向右移动到下一个车道
    } else {
      --waypoint.lane_id; // 向左移动到下一个车道
    }
    return IsLanePresent(_data, waypoint) ? waypoint : boost::optional<Waypoint>{}; // 检查新车道是否存在
  }

  boost::optional<Waypoint> Map::GetLeft(Waypoint waypoint) const {
    RELEASE_ASSERT(waypoint.lane_id != 0); // 确保车道ID不为0
    if (std::abs(waypoint.lane_id) == 1) { // 如果当前车道ID绝对值为1
      waypoint.lane_id *= -1; // 切换到另一侧的车道
    } else if (waypoint.lane_id > 0) { // 如果当前车道ID为正
      --waypoint.lane_id; // 向左移动到下一个车道
    } else {
      ++waypoint.lane_id; // 向右移动到下一个车道
    }
    return IsLanePresent(_data, waypoint) ? waypoint : boost::optional<Waypoint>{}; // 检查新车道是否存在
  }

  std::vector<Waypoint> Map::GenerateWaypoints(const double distance) const {
    RELEASE_ASSERT(distance > 0.0); // 确保距离大于0
    std::vector<Waypoint> result; // 存储生成的waypoints
    for (const auto &pair : _data.GetRoads()) { // 遍历所有道路
      const auto &road = pair.second; // 获取当前道路
      for (double s = EPSILON; s < (road.GetLength() - EPSILON); s += distance) { // 从0到道路长度生成waypoints
        ForEachDrivableLaneAt(road, s, [&](auto &&waypoint) { // 对每个可驾驶车道执行操作
          result.emplace_back(waypoint); // 将waypoint添加到结果中
        });
      }
    }
    return result; // 返回生成的waypoints
  }

 std::vector<Waypoint> Map::GenerateWaypointsOnRoadEntries(Lane::LaneType lane_type) const {
    std::vector<Waypoint> result; // 创建一个空的 Waypoint 向量来存储结果
    for (const auto &pair : _data.GetRoads()) { // 遍历所有道路
        const auto &road = pair.second; // 获取当前道路
        // 右侧车道从 s = 0 开始
        for (const auto &lane_section : road.GetLaneSectionsAt(0.0)) { // 在 s=0 处获取车道段
            for (const auto &lane : lane_section.GetLanes()) { // 遍历当前车道段的车道
                // 仅添加右侧（负值）车道
                if (lane.first < 0 && 
                    static_cast<int32_t>(lane.second.GetType()) & static_cast<int32_t>(lane_type)) {
                    result.emplace_back(Waypoint{ road.GetId(), lane_section.GetId(), lane.second.GetId(), 0.0 }); // 添加到结果
                }
            }
        }
        // 左侧车道从 s = max 开始
        const auto road_len = road.GetLength(); // 获取道路长度
        for (const auto &lane_section : road.GetLaneSectionsAt(road_len)) { // 在 s=road_len 处获取车道段
            for (const auto &lane : lane_section.GetLanes()) { // 遍历当前车道段的车道
                // 仅添加左侧（正值）车道
                if (lane.first > 0 && 
                    static_cast<int32_t>(lane.second.GetType()) & static_cast<int32_t>(lane_type)) {
                    result.emplace_back(
                        Waypoint{ road.GetId(), lane_section.GetId(), lane.second.GetId(), road_len }); // 添加到结果
                }
            }
        }
    }
    return result; // 返回生成的 Waypoint 向量
}

std::vector<Waypoint> Map::GenerateWaypointsInRoad(
    RoadId road_id,
    Lane::LaneType lane_type) const {
    std::vector<Waypoint> result; // 创建一个空的 Waypoint 向量来存储结果
    if(_data.GetRoads().count(road_id)) { // 检查道路是否存在
        const auto &road = _data.GetRoads().at(road_id); // 获取指定道路
        // 右侧车道从 s = 0 开始
        for (const auto &lane_section : road.GetLaneSectionsAt(0.0)) { // 在 s=0 处获取车道段
            for (const auto &lane : lane_section.GetLanes()) { // 遍历当前车道段的车道
                // 仅添加右侧（负值）车道
                if (lane.first < 0 && 
                    static_cast<int32_t>(lane.second.GetType()) & static_cast<int32_t>(lane_type)) {
                    result.emplace_back(Waypoint{ road.GetId(), lane_section.GetId(), lane.second.GetId(), 0.0 }); // 添加到结果
                }
            }
        }
        // 左侧车道从 s = max 开始
        const auto road_len = road.GetLength(); // 获取道路长度
        for (const auto &lane_section : road.GetLaneSectionsAt(road_len)) { // 在 s=road_len 处获取车道段
            for (const auto &lane : lane_section.GetLanes()) { // 遍历当前车道段的车道
                // 仅添加左侧（正值）车道
                if (lane.first > 0 && 
                    static_cast<int32_t>(lane.second.GetType()) & static_cast<int32_t>(lane_type)) {
                    result.emplace_back(
                        Waypoint{ road.GetId(), lane_section.GetId(), lane.second.GetId(), road_len }); // 添加到结果
                }
            }
        }
    }
    return result; // 返回生成的 Waypoint 向量
}

std::vector<std::pair<Waypoint, Waypoint>> Map::GenerateTopology() const {
    std::vector<std::pair<Waypoint, Waypoint>> result; // 创建一个空的 Waypoint 对向量来存储结果
    for (const auto &pair : _data.GetRoads()) { // 遍历所有道路
        const auto &road = pair.second; // 获取当前道路
        ForEachDrivableLane(road, [&](auto &&waypoint) { // 对每个可驾驶车道执行操作
            auto successors = GetSuccessors(waypoint); // 获取当前 waypoint 的后继 waypoint
            if (successors.size() == 0) { // 如果没有后继
                auto distance = static_cast<float>(GetDistanceAtEndOfLane(GetLane(waypoint))); // 获取车道末尾的距离
                auto last_waypoint = GetWaypoint(waypoint.road_id, waypoint.lane_id, distance); // 获取最后一个 waypoint
                if (last_waypoint.has_value()) { // 如果存在最后一个 waypoint
                    result.push_back({waypoint, *last_waypoint}); // 添加到结果
                }
            } else { // 如果有后继
                for (auto &&successor : GetSuccessors(waypoint)) { // 遍历所有后继
                    result.push_back({waypoint, successor}); // 添加到结果
                }
            }
        });
    }
    return result; // 返回生成的 waypoint 对向量
}


std::vector<std::pair<Waypoint, Waypoint>> Map::GetJunctionWaypoints(JuncId id, Lane::LaneType lane_type) const {
    std::vector<std::pair<Waypoint, Waypoint>> result; // 存储结果的向量
    const Junction * junction = GetJunction(id); // 获取指定ID的交叉口

    for(auto &connections : junction->GetConnections()) { // 遍历交叉口的所有连接
        const Road &road = _data.GetRoad(connections.second.connecting_road); // 获取连接的道路
        ForEachLane(road, lane_type, [&](auto &&waypoint) { // 对于每条车道
            const auto& lane = GetLane(waypoint); // 获取车道信息
            const double final_s = GetDistanceAtEndOfLane(lane); // 计算车道末端的距离
            Waypoint lane_end(waypoint); // 创建一个新的Waypoint对象
            lane_end.s = final_s; // 设置新的Waypoint的s值为末端距离
            result.push_back({waypoint, lane_end}); // 将原Waypoint和末端Waypoint存入结果向量
        });
    }
    return result; // 返回结果
}

std::unordered_map<road::RoadId, std::unordered_set<road::RoadId>>
Map::ComputeJunctionConflicts(JuncId id) const {
    const float epsilon = 0.0001f; // 设置一个小的误差值，防止数值错误
    const Junction *junction = GetJunction(id); // 获取指定ID的交叉口
    std::unordered_map<road::RoadId, std::unordered_set<road::RoadId>> conflicts; // 存储冲突的道路ID

    // 2D类型定义
    typedef boost::geometry::model::point<float, 2, boost::geometry::cs::cartesian> Point2d; // 定义2D点
    typedef boost::geometry::model::segment<Point2d> Segment2d; // 定义2D线段
    typedef boost::geometry::model::box<Rtree::BPoint> Box; // 定义包围盒

    // 计算包围盒范围
    auto bbox_pos = junction->GetBoundingBox().location; // 获取交叉口的中心位置
    auto bbox_ext = junction->GetBoundingBox().extent; // 获取交叉口的扩展范围
    auto min_corner = geom::Vector3D( // 计算最小角点
        bbox_pos.x - bbox_ext.x,
        bbox_pos.y - bbox_ext.y,
        bbox_pos.z - bbox_ext.z - epsilon);
    auto max_corner = geom::Vector3D( // 计算最大角点
        bbox_pos.x + bbox_ext.x,
        bbox_pos.y + bbox_ext.y,
        bbox_pos.z + bbox_ext.z + epsilon);
    Box box({min_corner.x, min_corner.y, min_corner.z}, // 创建包围盒
        {max_corner.x, max_corner.y, max_corner.z});
    auto segments = _rtree.GetIntersections(box); // 获取与包围盒相交的线段

    for (size_t i = 0; i < segments.size(); ++i) { // 遍历所有线段
        auto &segment1 = segments[i]; // 获取当前线段
        auto waypoint1 = segment1.second.first; // 获取对应的第一个Waypoint
        JuncId junc_id1 = _data.GetRoad(waypoint1.road_id).GetJunctionId(); // 获取该Waypoint所在道路的交叉口ID
        // 只处理在当前交叉口的线段
        if(junc_id1 != id) {
            continue; // 如果不在当前交叉口则跳过
        }
        Segment2d seg1{{segment1.first.first.get<0>(), segment1.first.first.get<1>()}, // 构建第一个线段
            {segment1.first.second.get<0>(), segment1.first.second.get<1()}};
        for (size_t j = i + 1; j < segments.size(); ++j) { // 遍历后续的线段
            auto &segment2 = segments[j]; // 获取第二个线段
            auto waypoint2 = segment2.second.first; // 获取对应的Waypoint
            JuncId junc_id2 = _data.GetRoad(waypoint2.road_id).GetJunctionId(); // 获取该Waypoint所在道路的交叉口ID
            // 只处理在当前交叉口的线段
            if(junc_id2 != id) {
                continue; // 如果不在当前交叉口则跳过
            }
            // 排除同一路径
            if(waypoint1.road_id == waypoint2.road_id) {
                continue; // 如果是同一路段则跳过
            }
            Segment2d seg2{{segment2.first.first.get<0>(), segment2.first.first.get<1>()}, // 构建第二个线段
                {segment2.first.second.get<0>(), segment2.first.second.get<1>()}};

            double distance = boost::geometry::distance(seg1, seg2); // 计算两线段之间的距离
            // 设定距离阈值
            if(distance > 2.0) {
                continue; // 如果距离大于2.0则跳过
            }
            if(conflicts[waypoint1.road_id].count(waypoint2.road_id) == 0) {
                conflicts[waypoint1.road_id].insert(waypoint2.road_id); // 记录冲突
            }
            if(conflicts[waypoint2.road_id].count(waypoint1.road_id) == 0) {
                conflicts[waypoint2.road_id].insert(waypoint1.road_id); // 记录冲突
            }
        }
    }
    return conflicts; // 返回所有冲突
}

const Lane &Map::GetLane(Waypoint waypoint) const {
    // 根据给定的Waypoint获取对应的车道
    return _data.GetRoad(waypoint.road_id).GetLaneById(waypoint.section_id, waypoint.lane_id);
}

// ===========================================================================
// -- Map: Private functions -------------------------------------------------
// ===========================================================================

// 使用线段两端的Waypoints位置将新元素添加到R树元素列表中
void Map::AddElementToRtree(
    std::vector<Rtree::TreeElement> &rtree_elements, // R树元素列表
    geom::Transform &current_transform,               // 当前变换
    geom::Transform &next_transform,                  // 下一个变换
    Waypoint &current_waypoint,                       // 当前Waypoint
    Waypoint &next_waypoint) {                        // 下一个Waypoint
    // 初始化点
    Rtree::BPoint init =
        Rtree::BPoint(
            current_transform.location.x,
            current_transform.location.y,
            current_transform.location.z);
    // 结束点
    Rtree::BPoint end =
        Rtree::BPoint(
            next_transform.location.x,
            next_transform.location.y,
            next_transform.location.z);
    // 将线段和相应的Waypoints加入R树元素列表
    rtree_elements.emplace_back(std::make_pair(Rtree::BSegment(init, end),
        std::make_pair(current_waypoint, next_waypoint)));
}

// 使用Waypoints的位置将新元素添加到R树元素列表中，并更新变换
void Map::AddElementToRtreeAndUpdateTransforms(
    std::vector<Rtree::TreeElement> &rtree_elements, // R树元素列表
    geom::Transform &current_transform,               // 当前变换
    Waypoint &current_waypoint,                       // 当前Waypoint
    Waypoint &next_waypoint) {                        // 下一个Waypoint
    // 计算下一个Waypoint的变换
    geom::Transform next_transform = ComputeTransform(next_waypoint);
    // 添加元素到R树
    AddElementToRtree(rtree_elements, current_transform, next_transform,
                      current_waypoint, next_waypoint);
    // 更新当前Waypoint和变换
    current_waypoint = next_waypoint;
    current_transform = next_transform;
}

// 根据车道方向返回几何图形的剩余长度
double GetRemainingLength(const Lane &lane, double current_s) {
    if (lane.GetId() < 0) {
        // 如果车道ID小于0，计算剩余长度
        return (lane.GetDistance() + lane.GetLength() - current_s);
    } else {
        // 如果车道ID大于等于0，返回从当前s到车道起始位置的长度
        return (current_s - lane.GetDistance());
    }
}

// 创建R树
void Map::CreateRtree() {
    const double epsilon = 0.000001; // 设置一个小的增量以防止数值误差
    const double min_delta_s = 1;    // 每个段的最小长度为1米

    // 1.8度，曲线中放置线段的最大角度阈值
    constexpr double angle_threshold = geom::Math::Pi<double>() / 100.0;
    // 线段的最大长度
    constexpr double max_segment_length = 100.0;

    // 在每条车道的起始位置生成Waypoints
    std::vector<Waypoint> topology; // 存储所有Waypoints
    for (const auto &pair : _data.GetRoads()) { // 遍历所有道路
        const auto &road = pair.second; // 获取道路信息
        // 对每条车道进行操作
        ForEachLane(road, Lane::LaneType::Any, [&](auto &&waypoint) {
            if(waypoint.lane_id != 0) { // 排除ID为0的车道
                topology.push_back(waypoint); // 将Waypoint加入到topology中
            }
        });
    }
}

    // Container of segments and waypoints
    std::vector<Rtree::TreeElement> rtree_elements;
    // Loop through all lanes
    for (auto &waypoint : topology) {
      auto &lane_start_waypoint = waypoint;

      auto current_waypoint = lane_start_waypoint;

      const Lane &lane = GetLane(current_waypoint);

      geom::Transform current_transform = ComputeTransform(current_waypoint);

      // Save computation time in straight lines
      if (lane.IsStraight()) {
        double delta_s = min_delta_s;
        double remaining_length =
            GetRemainingLength(lane, current_waypoint.s);
        remaining_length -= epsilon;
        delta_s = remaining_length;
        if (delta_s < epsilon) {
          continue;
        }
        auto next = GetNext(current_waypoint, delta_s);

        RELEASE_ASSERT(next.size() == 1);
        RELEASE_ASSERT(next.front().road_id == current_waypoint.road_id);
        auto next_waypoint = next.front();

        AddElementToRtreeAndUpdateTransforms(
            rtree_elements,
            current_transform,
            current_waypoint,
            next_waypoint);
        // end of lane
      } else {
        auto next_waypoint = current_waypoint;

        // Loop until the end of the lane
        // Advance in small s-increments
        while (true) {
          double delta_s = min_delta_s;
          double remaining_length =
              GetRemainingLength(lane, next_waypoint.s);
          remaining_length -= epsilon;
          delta_s = std::min(delta_s, remaining_length);

          if (delta_s < epsilon) {
            AddElementToRtreeAndUpdateTransforms(
                rtree_elements,
                current_transform,
                current_waypoint,
                next_waypoint);
            break;
          }

          auto next = GetNext(next_waypoint, delta_s);
          if (next.size() != 1 ||
          current_waypoint.section_id != next.front().section_id) {
            AddElementToRtreeAndUpdateTransforms(
                rtree_elements,
                current_transform,
                current_waypoint,
                next_waypoint);
            break;
          }

          next_waypoint = next.front();
          geom::Transform next_transform = ComputeTransform(next_waypoint);
          double angle = geom::Math::GetVectorAngle(
              current_transform.GetForwardVector(), next_transform.GetForwardVector());

          if (std::abs(angle) > angle_threshold ||
              std::abs(current_waypoint.s - next_waypoint.s) > max_segment_length) {
            AddElementToRtree(
                rtree_elements,
                current_transform,
                next_transform,
                current_waypoint,
                next_waypoint);
            current_waypoint = next_waypoint;
            current_transform = next_transform;
          }
        }
      }
    }
    // Add segments to Rtree
    _rtree.InsertElements(rtree_elements);
  }

  Junction* Map::GetJunction(JuncId id) {
    return _data.GetJunction(id);
  }

  const Junction* Map::GetJunction(JuncId id) const {
    return _data.GetJunction(id);
  }

  geom::Mesh Map::GenerateMesh(
      const double distance,
      const float extra_width,
      const  bool smooth_junctions) const {
    RELEASE_ASSERT(distance > 0.0);
    geom::MeshFactory mesh_factory;
    geom::Mesh out_mesh;

    mesh_factory.road_param.resolution = static_cast<float>(distance);
    mesh_factory.road_param.extra_lane_width = extra_width;

    // Generate roads outside junctions
    for (auto &&pair : _data.GetRoads()) {
      const auto &road = pair.second;
      if (road.IsJunction()) {
        continue;
      }
      out_mesh += *mesh_factory.Generate(road);
    }

    // Generate roads within junctions and smooth them
    for (const auto &junc_pair : _data.GetJunctions()) {
      const auto &junction = junc_pair.second;
      std::vector<std::unique_ptr<geom::Mesh>> lane_meshes;
      for(const auto &connection_pair : junction.GetConnections()) {
        const auto &connection = connection_pair.second;
        const auto &road = _data.GetRoads().at(connection.connecting_road);
        for (auto &&lane_section : road.GetLaneSections()) {
          for (auto &&lane_pair : lane_section.GetLanes()) {
            lane_meshes.push_back(mesh_factory.Generate(lane_pair.second));
          }
        }
      }
      if(smooth_junctions) {
        out_mesh += *mesh_factory.MergeAndSmooth(lane_meshes);
      } else {
        geom::Mesh junction_mesh;
        for(auto& lane : lane_meshes) {
          junction_mesh += *lane;
        }
        out_mesh += junction_mesh;
      }
    }

    return out_mesh;
  }


  std::vector<std::unique_ptr<geom::Mesh>> Map::GenerateChunkedMesh(
      const rpc::OpendriveGenerationParameters& params) const {
    geom::MeshFactory mesh_factory(params);
    std::vector<std::unique_ptr<geom::Mesh>> out_mesh_list;

    std::unordered_map<JuncId, geom::Mesh> junction_map;
    for (auto &&pair : _data.GetRoads()) {
      const auto &road = pair.second;
      if (!road.IsJunction()) {
        std::vector<std::unique_ptr<geom::Mesh>> road_mesh_list =
            mesh_factory.GenerateAllWithMaxLen(road);

        out_mesh_list.insert(
            out_mesh_list.end(),
            std::make_move_iterator(road_mesh_list.begin()),
            std::make_move_iterator(road_mesh_list.end()));
      }
    }

    // Generate roads within junctions and smooth them
    for (const auto &junc_pair : _data.GetJunctions()) {
      const auto &junction = junc_pair.second;
      std::vector<std::unique_ptr<geom::Mesh>> lane_meshes;
      std::vector<std::unique_ptr<geom::Mesh>> sidewalk_lane_meshes;
      for(const auto &connection_pair : junction.GetConnections()) {
        const auto &connection = connection_pair.second;
        const auto &road = _data.GetRoads().at(connection.connecting_road);
        for (auto &&lane_section : road.GetLaneSections()) {
          for (auto &&lane_pair : lane_section.GetLanes()) {
            const auto &lane = lane_pair.second;
            if (lane.GetType() != road::Lane::LaneType::Sidewalk) {
              lane_meshes.push_back(mesh_factory.Generate(lane));
            } else {
              sidewalk_lane_meshes.push_back(mesh_factory.Generate(lane));
            }
          }
        }
      }
      if(params.smooth_junctions) {
        auto merged_mesh = mesh_factory.MergeAndSmooth(lane_meshes);
        for(auto& lane : sidewalk_lane_meshes) {
          *merged_mesh += *lane;
        }
        out_mesh_list.push_back(std::move(merged_mesh));
      } else {
        std::unique_ptr<geom::Mesh> junction_mesh = std::make_unique<geom::Mesh>();
        for(auto& lane : lane_meshes) {
          *junction_mesh += *lane;
        }
        for(auto& lane : sidewalk_lane_meshes) {
          *junction_mesh += *lane;
        }
        out_mesh_list.push_back(std::move(junction_mesh));
      }
    }

    auto min_pos = geom::Vector2D(
        out_mesh_list.front()->GetVertices().front().x,
        out_mesh_list.front()->GetVertices().front().y);
    auto max_pos = min_pos;
    for (auto & mesh : out_mesh_list) {
      auto vertex = mesh->GetVertices().front();
      min_pos.x = std::min(min_pos.x, vertex.x);
      min_pos.y = std::min(min_pos.y, vertex.y);
      max_pos.x = std::max(max_pos.x, vertex.x);
      max_pos.y = std::max(max_pos.y, vertex.y);
    }
    size_t mesh_amount_x = static_cast<size_t>((max_pos.x - min_pos.x)/params.max_road_length) + 1;
    size_t mesh_amount_y = static_cast<size_t>((max_pos.y - min_pos.y)/params.max_road_length) + 1;
    std::vector<std::unique_ptr<geom::Mesh>> result;
    result.reserve(mesh_amount_x*mesh_amount_y);
    for (size_t i = 0; i < mesh_amount_x*mesh_amount_y; ++i) {
      result.emplace_back(std::make_unique<geom::Mesh>());
    }
    for (auto & mesh : out_mesh_list) {
      auto vertex = mesh->GetVertices().front();
      size_t x_pos = static_cast<size_t>((vertex.x - min_pos.x) / params.max_road_length);
      size_t y_pos = static_cast<size_t>((vertex.y - min_pos.y) / params.max_road_length);
      *(result[x_pos + mesh_amount_x*y_pos]) += *mesh;
    }

    return result;
  }

  std::map<road::Lane::LaneType , std::vector<std::unique_ptr<geom::Mesh>>>
    Map::GenerateOrderedChunkedMeshInLocations( const rpc::OpendriveGenerationParameters& params,
                                     const geom::Vector3D& minpos,
                                     const geom::Vector3D& maxpos) const
  {

    geom::MeshFactory mesh_factory(params);
    std::map<road::Lane::LaneType, std::vector<std::unique_ptr<geom::Mesh>>> road_out_mesh_list;
    std::map<road::Lane::LaneType, std::vector<std::unique_ptr<geom::Mesh>>> junction_out_mesh_list;

    std::thread juntction_thread( &Map::GenerateJunctions, this, mesh_factory, params,
      minpos, maxpos, &junction_out_mesh_list);

    const std::vector<RoadId> RoadsIDToGenerate = FilterRoadsByPosition(minpos, maxpos);

    size_t num_roads = RoadsIDToGenerate.size();
    size_t num_roads_per_thread = 30;
    size_t num_threads = (num_roads / num_roads_per_thread) + 1;
    num_threads = num_threads > 1 ? num_threads : 1;
    std::vector<std::thread> workers;
    std::mutex write_mutex;
    std::cout << "Generating " << std::to_string(num_roads) << " roads" << std::endl;

    for ( size_t i = 0; i < num_threads; ++i ) {
      std::thread neworker(
        [this, &write_mutex, &mesh_factory, &RoadsIDToGenerate, &road_out_mesh_list, i, num_roads_per_thread]() {
        std::map<road::Lane::LaneType, std::vector<std::unique_ptr<geom::Mesh>>> Current =
          std::move(GenerateRoadsMultithreaded(mesh_factory, RoadsIDToGenerate,i, num_roads_per_thread ));
        std::lock_guard<std::mutex> guard(write_mutex);
        for ( auto&& pair : Current ) {
          if (road_out_mesh_list.find(pair.first) != road_out_mesh_list.end()) {
            road_out_mesh_list[pair.first].insert(road_out_mesh_list[pair.first].end(),
              std::make_move_iterator(pair.second.begin()),
              std::make_move_iterator(pair.second.end()));
          } else {
            road_out_mesh_list[pair.first] = std::move(pair.second);
          }
        }
      });
      workers.push_back(std::move(neworker));
    }

    for (size_t i = 0; i < workers.size(); ++i) {
      workers[i].join();
    }
    workers.clear();
    for (size_t i = 0; i < workers.size(); ++i) {
      if (workers[i].joinable()) {
        workers[i].join();
      }
    }

    juntction_thread.join();
    for (auto&& pair : junction_out_mesh_list) {
      if (road_out_mesh_list.find(pair.first) != road_out_mesh_list.end())
      {
        road_out_mesh_list[pair.first].insert(road_out_mesh_list[pair.first].end(),
          std::make_move_iterator(pair.second.begin()),
          std::make_move_iterator(pair.second.end()));
      }
      else
      {
        road_out_mesh_list[pair.first] = std::move(pair.second);
      }
    }
    std::cout << "Generated " << std::to_string(num_roads) << " roads" << std::endl;

    return road_out_mesh_list;
  }

  std::vector<std::pair<geom::Transform, std::string>> Map::GetTreesTransform(
    const geom::Vector3D& minpos,
    const geom::Vector3D& maxpos,
    float distancebetweentrees,
    float distancefromdrivinglineborder,
    float s_offset) const {

    std::vector<std::pair<geom::Transform, std::string>> transforms;

    const std::vector<RoadId> RoadsIDToGenerate = FilterRoadsByPosition(minpos, maxpos);
    for ( RoadId id : RoadsIDToGenerate ) {
      const auto& road = _data.GetRoads().at(id);
      if (!road.IsJunction()) {
        for (auto &&lane_section : road.GetLaneSections()) {
          LaneId min_lane = 0;
          for (auto &pairlane : lane_section.GetLanes()) {
            if (min_lane > pairlane.first && pairlane.second.GetType() == Lane::LaneType::Driving) {
              min_lane = pairlane.first;
            }
          }

          const road::Lane* lane = lane_section.GetLane(min_lane);
          if( lane ) {
            double s_current = lane_section.GetDistance() + s_offset;
            const double s_end = lane_section.GetDistance() + lane_section.GetLength();
            while(s_current < s_end){
              if(lane->GetWidth(s_current) != 0.0f){
                const auto edges = lane->GetCornerPositions(s_current, 0);
                if (edges.first == edges.second) continue;
                geom::Vector3D director = edges.second - edges.first;
                geom::Vector3D treeposition = edges.first - director.MakeUnitVector() * distancefromdrivinglineborder;
                geom::Transform lanetransform = lane->ComputeTransform(s_current);
                geom::Transform treeTransform(treeposition, lanetransform.rotation);
                const carla::road::element::RoadInfoSpeed* roadinfo = lane->GetInfo<carla::road::element::RoadInfoSpeed>(s_current);
                if(roadinfo){
                  transforms.push_back(std::make_pair(treeTransform, roadinfo->GetType()));
                }else{
                  transforms.push_back(std::make_pair(treeTransform, "urban"));
                }
              }
              s_current += distancebetweentrees;
            }

          }
        }
      }
    }
    return transforms;
  }

  geom::Mesh Map::GetAllCrosswalkMesh() const {
    geom::Mesh out_mesh;

    // Get the crosswalk vertices for the current map
    const std::vector<geom::Location> crosswalk_vertex = GetAllCrosswalkZones();
    if (crosswalk_vertex.empty()) {
      return out_mesh;
    }

    // Create a a list of triangle fans with material "crosswalk"
    out_mesh.AddMaterial("crosswalk");
    size_t start_vertex_index = 0;
    size_t i = 0;
    std::vector<geom::Vector3D> vertices;
    // Iterate the vertices until a repeated one is found, this indicates
    // the triangle fan is done and another one must start
    do {
      // Except for the first iteration && triangle fan done
      if (i != 0 && crosswalk_vertex[start_vertex_index] == crosswalk_vertex[i]) {
        // Create the actual fan
        out_mesh.AddTriangleFan(vertices);
        vertices.clear();
        // End the loop if i reached the end of the vertex list
        if (i >= crosswalk_vertex.size() - 1) {
          break;
        }
        start_vertex_index = ++i;
      }
      // Append a new Vector3D that will be added to the triangle fan
      vertices.push_back(crosswalk_vertex[i++]);
    } while (i < crosswalk_vertex.size());

    out_mesh.EndMaterial();
    return out_mesh;
  }

  /// Buids a list of meshes related with LineMarkings
  std::vector<std::unique_ptr<geom::Mesh>> Map::GenerateLineMarkings(
    const rpc::OpendriveGenerationParameters& params,
    const geom::Vector3D& minpos,
    const geom::Vector3D& maxpos,
    std::vector<std::string>& outinfo ) const
  {
    std::vector<std::unique_ptr<geom::Mesh>> LineMarks;
    geom::MeshFactory mesh_factory(params);

    const std::vector<RoadId> RoadsIDToGenerate = FilterRoadsByPosition(minpos, maxpos);
    for ( RoadId id : RoadsIDToGenerate ) {
      const auto& road = _data.GetRoads().at(id);
      if (!road.IsJunction()) {
        mesh_factory.GenerateLaneMarkForRoad(road, LineMarks, outinfo);
      }
    }

    return std::move(LineMarks);
  }

  std::vector<carla::geom::BoundingBox> Map::GetJunctionsBoundingBoxes() const {
    std::vector<carla::geom::BoundingBox> returning;
    for ( const auto& junc_pair : _data.GetJunctions() ) {
      const auto& junction = junc_pair.second;
      float box_extraextension_factor = 1.5f;
      carla::geom::BoundingBox bb = junction.GetBoundingBox();
      bb.extent *= box_extraextension_factor;
      returning.push_back(bb);
    }
    return returning;
  }

  inline float Map::GetZPosInDeformation(float posx, float posy) const {
    return geom::deformation::GetZPosInDeformation(posx, posy) +
      geom::deformation::GetBumpDeformation(posx,posy);
  }

  std::map<road::Lane::LaneType, std::vector<std::unique_ptr<geom::Mesh>>>
      Map::GenerateRoadsMultithreaded( const carla::geom::MeshFactory& mesh_factory,
                                        const std::vector<RoadId>& RoadsId,
                                        const size_t index, const size_t number_of_roads_per_thread) const
  {
    std::map<road::Lane::LaneType, std::vector<std::unique_ptr<geom::Mesh>>> out;

    size_t start = index * number_of_roads_per_thread;
    size_t endoffset = (index+1) * number_of_roads_per_thread;
    size_t end = RoadsId.size();

    for (int i = start; i < endoffset && i < end; ++i) {
      const auto& road = _data.GetRoads().at(RoadsId[i]);
      if (!road.IsJunction()) {
        mesh_factory.GenerateAllOrderedWithMaxLen(road, out);
      }
    }
    std::cout << "Generated roads from " + std::to_string(index * number_of_roads_per_thread) + " to " + std::to_string((index+1) * number_of_roads_per_thread ) << std::endl;
    return out;
  }

  void Map::GenerateJunctions(const carla::geom::MeshFactory& mesh_factory,
    const rpc::OpendriveGenerationParameters& params,
    const geom::Vector3D& minpos,
    const geom::Vector3D& maxpos,
    std::map<road::Lane::LaneType,
    std::vector<std::unique_ptr<geom::Mesh>>>* junction_out_mesh_list) const {

    std::vector<JuncId> JunctionsToGenerate = FilterJunctionsByPosition(minpos, maxpos);
    size_t num_junctions = JunctionsToGenerate.size();
    std::cout << "Generating " << std::to_string(num_junctions) << " junctions" << std::endl;
    size_t junctionindex = 0;
    size_t num_junctions_per_thread = 5;
    size_t num_threads = (num_junctions / num_junctions_per_thread) + 1;
    num_threads = num_threads > 1 ? num_threads : 1;
    std::vector<std::thread> workers;
    std::mutex write_mutex;

    for ( size_t i = 0; i < num_threads; ++i ) {
      std::thread neworker(
        [this, &write_mutex, &mesh_factory, &junction_out_mesh_list, JunctionsToGenerate, i, num_junctions_per_thread, num_junctions]() {
        std::map<road::Lane::LaneType,
          std::vector<std::unique_ptr<geom::Mesh>>> junctionsofthisthread;

        size_t minimum = 0;
        if( (i + 1) * num_junctions_per_thread < num_junctions ){
          minimum = (i + 1) * num_junctions_per_thread;
        }else{
          minimum = num_junctions;
        }
        std::cout << "Generating Junctions between  " << std::to_string(i * num_junctions_per_thread) << " and " << std::to_string(minimum) << std::endl;

        for ( size_t junctionindex = i * num_junctions_per_thread;
                        junctionindex < minimum;
                        ++junctionindex )
        {
          GenerateSingleJunction(mesh_factory, JunctionsToGenerate[junctionindex], &junctionsofthisthread);
        }
        std::cout << "Generated Junctions between  " << std::to_string(i * num_junctions_per_thread) << " and " << std::to_string(minimum) << std::endl;
        std::lock_guard<std::mutex> guard(write_mutex);
        for ( auto&& pair : junctionsofthisthread ) {
          if ((*junction_out_mesh_list).find(pair.first) != (*junction_out_mesh_list).end()) {
            (*junction_out_mesh_list)[pair.first].insert((*junction_out_mesh_list)[pair.first].end(),
              std::make_move_iterator(pair.second.begin()),
              std::make_move_iterator(pair.second.end()));
          } else {
            (*junction_out_mesh_list)[pair.first] = std::move(pair.second);
          }
        }
      });
      workers.push_back(std::move(neworker));
    }

    for (size_t i = 0; i < workers.size(); ++i) {
      workers[i].join();
    }
    workers.clear();
    for (size_t i = 0; i < workers.size(); ++i) {
      if (workers[i].joinable()) {
        workers[i].join();
      }
    }
  }

  std::vector<JuncId> Map::FilterJunctionsByPosition( const geom::Vector3D& minpos,
    const geom::Vector3D& maxpos ) const {

    std::cout << "Filtered from " + std::to_string(_data.GetJunctions().size() ) + " junctions " << std::endl;
    std::vector<JuncId> ToReturn;
    for( auto& junction : _data.GetJunctions() ){
      geom::Location junctionLocation = junction.second.GetBoundingBox().location;
      if( minpos.x < junctionLocation.x && junctionLocation.x < maxpos.x &&
            minpos.y > junctionLocation.y && junctionLocation.y > maxpos.y ) {
        ToReturn.push_back(junction.first);
      }
    }
    std::cout << "To " + std::to_string(ToReturn.size() ) + " junctions " << std::endl;

    return ToReturn;
  }

  std::vector<RoadId> Map::FilterRoadsByPosition( const geom::Vector3D& minpos,
    const geom::Vector3D& maxpos ) const {

    std::vector<RoadId> ToReturn;
    std::cout << "Filtered from " + std::to_string(_data.GetRoads().size() ) + " roads " << std::endl;
    for( auto& road : _data.GetRoads() ){
      auto &&lane_section = (*road.second.GetLaneSections().begin());
      const road::Lane* lane = lane_section.GetLane(-1);
      if( lane ) {
        const double s_check = lane_section.GetDistance() + lane_section.GetLength() * 0.5;
        geom::Location roadLocation = lane->ComputeTransform(s_check).location;
        if( minpos.x < roadLocation.x && roadLocation.x < maxpos.x &&
              minpos.y > roadLocation.y && roadLocation.y > maxpos.y ) {
          ToReturn.push_back(road.first);
        }
      }
    }
    std::cout << "To " + std::to_string(ToReturn.size() ) + " roads " << std::endl;
    return ToReturn;
  }

  std::unique_ptr<geom::Mesh> Map::SDFToMesh(const road::Junction& jinput,
    const std::vector<geom::Vector3D>& sdfinput,
    int grid_cells_per_dim) const {

    int junctionid = jinput.GetId();
    float box_extraextension_factor = 1.2f;
    const double CubeSize = 0.5;
    carla::geom::BoundingBox bb = jinput.GetBoundingBox();
    carla::geom::Vector3D MinOffset = bb.location - geom::Location(bb.extent * box_extraextension_factor);
    carla::geom::Vector3D MaxOffset = bb.location + geom::Location(bb.extent * box_extraextension_factor);
    carla::geom::Vector3D OffsetPerCell = ( bb.extent * box_extraextension_factor * 2 ) / grid_cells_per_dim;

    auto junctionsdf = [this, OffsetPerCell, CubeSize, MinOffset, junctionid](MeshReconstruction::Vec3 const& pos)
    {
      geom::Vector3D worldloc(pos.x, pos.y, pos.z);
      boost::optional<element::Waypoint> CheckingWaypoint = GetWaypoint(geom::Location(worldloc), 0x1 << 1);
      if (CheckingWaypoint) {
        if ( pos.z < 0.2) {
          return 0.0;
        } else {
          return -abs(pos.z);
        }
      }
      boost::optional<element::Waypoint> InRoadWaypoint = GetClosestWaypointOnRoad(geom::Location(worldloc), 0x1 << 1);
      geom::Transform InRoadWPTransform = ComputeTransform(*InRoadWaypoint);

      geom::Vector3D director = geom::Location(worldloc) - (InRoadWPTransform.location);
      geom::Vector3D laneborder = InRoadWPTransform.location + geom::Location(director.MakeUnitVector() * GetLaneWidth(*InRoadWaypoint) * 0.5f);

      geom::Vector3D Distance = laneborder - worldloc;
      if (Distance.Length2D() < CubeSize * 1.1 && pos.z < 0.2) {
        return 0.0;
      }
      return Distance.Length() * -1.0;
    };

    double gridsizeindouble = grid_cells_per_dim;
    MeshReconstruction::Rect3 domain;
    domain.min = { MinOffset.x, MinOffset.y, MinOffset.z };
    domain.size = { bb.extent.x * box_extraextension_factor * 2, bb.extent.y * box_extraextension_factor * 2, 0.4 };

    MeshReconstruction::Vec3 cubeSize{ CubeSize, CubeSize, 0.2 };
    auto mesh = MeshReconstruction::MarchCube(junctionsdf, domain, cubeSize );
    carla::geom::Rotation inverse = bb.rotation;
    carla::geom::Vector3D trasltation = bb.location;
    geom::Mesh out_mesh;

    for (auto& cv : mesh.vertices) {
      geom::Vector3D newvertex;
      newvertex.x = cv.x;
      newvertex.y = cv.y;
      newvertex.z = cv.z;
      out_mesh.AddVertex(newvertex);
    }

    auto finalvertices = out_mesh.GetVertices();
    for (auto ct : mesh.triangles) {
      out_mesh.AddIndex(ct[1] + 1);
      out_mesh.AddIndex(ct[0] + 1);
      out_mesh.AddIndex(ct[2] + 1);
    }

    for (auto& cv : out_mesh.GetVertices() ) {
      boost::optional<element::Waypoint> CheckingWaypoint = GetWaypoint(geom::Location(cv), 0x1 << 1);
      if (!CheckingWaypoint)
      {
        boost::optional<element::Waypoint> InRoadWaypoint = GetClosestWaypointOnRoad(geom::Location(cv), 0x1 << 1);
        geom::Transform InRoadWPTransform = ComputeTransform(*InRoadWaypoint);

        geom::Vector3D director = geom::Location(cv) - (InRoadWPTransform.location);
        geom::Vector3D laneborder = InRoadWPTransform.location + geom::Location(director.MakeUnitVector() * GetLaneWidth(*InRoadWaypoint) * 0.5f);
        cv = laneborder;
      }
    }
    return std::make_unique<geom::Mesh>(out_mesh);
  }

  void Map::GenerateSingleJunction(const carla::geom::MeshFactory& mesh_factory,
      const JuncId Id,
      std::map<road::Lane::LaneType, std::vector<std::unique_ptr<geom::Mesh>>>*
      junction_out_mesh_list) const {

      const auto& junction = _data.GetJunctions().at(Id);
      if (junction.GetConnections().size() > 2) {
        std::vector<std::unique_ptr<geom::Mesh>> lane_meshes;
        std::vector<std::unique_ptr<geom::Mesh>> sidewalk_lane_meshes;
        std::vector<carla::geom::Vector3D> perimeterpoints;

        auto pmesh = SDFToMesh(junction, perimeterpoints, 75);
        (*junction_out_mesh_list)[road::Lane::LaneType::Driving].push_back(std::move(pmesh));

        for (const auto& connection_pair : junction.GetConnections()) {
          const auto& connection = connection_pair.second;
          const auto& road = _data.GetRoads().at(connection.connecting_road);
          for (auto&& lane_section : road.GetLaneSections()) {
            for (auto&& lane_pair : lane_section.GetLanes()) {
              const auto& lane = lane_pair.second;
              if ( lane.GetType() == road::Lane::LaneType::Sidewalk ) {
                boost::optional<element::Waypoint> sw =
                  GetWaypoint(road.GetId(), lane_pair.first, lane.GetDistance() + (lane.GetLength() * 0.5f));
                if( GetWaypoint(ComputeTransform(*sw).location).get_ptr () == nullptr ){
                  sidewalk_lane_meshes.push_back(mesh_factory.GenerateSidewalk(lane));
                }
              }
            }
          }
        }
        std::unique_ptr<geom::Mesh> sidewalk_mesh = std::make_unique<geom::Mesh>();
        for (auto& lane : sidewalk_lane_meshes) {
          *sidewalk_mesh += *lane;
        }
        (*junction_out_mesh_list)[road::Lane::LaneType::Sidewalk].push_back(std::move(sidewalk_mesh));
      } else {
        std::vector<std::unique_ptr<geom::Mesh>> lane_meshes;
        std::vector<std::unique_ptr<geom::Mesh>> sidewalk_lane_meshes;
        for (const auto& connection_pair : junction.GetConnections()) {
          const auto& connection = connection_pair.second;
          const auto& road = _data.GetRoads().at(connection.connecting_road);
          for (auto&& lane_section : road.GetLaneSections()) {
            for (auto&& lane_pair : lane_section.GetLanes()) {
              const auto& lane = lane_pair.second;
              if (lane.GetType() != road::Lane::LaneType::Sidewalk) {
                lane_meshes.push_back(mesh_factory.GenerateTesselated(lane));
              }
              else {
                sidewalk_lane_meshes.push_back(mesh_factory.GenerateSidewalk(lane));
              }
            }
          }
        }
        std::unique_ptr<geom::Mesh> merged_mesh = std::make_unique<geom::Mesh>();
        for (auto& lane : lane_meshes) {
          *merged_mesh += *lane;
        }
        std::unique_ptr<geom::Mesh> sidewalk_mesh = std::make_unique<geom::Mesh>();
        for (auto& lane : sidewalk_lane_meshes) {
          *sidewalk_mesh += *lane;
        }

        (*junction_out_mesh_list)[road::Lane::LaneType::Driving].push_back(std::move(merged_mesh));
        (*junction_out_mesh_list)[road::Lane::LaneType::Sidewalk].push_back(std::move(sidewalk_mesh));
      }
    }

} // namespace road
} // namespace carla
