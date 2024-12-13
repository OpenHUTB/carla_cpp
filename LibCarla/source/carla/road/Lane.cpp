// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/road/Lane.h"

#include <limits>

#include "carla/Debug.h"
#include "carla/geom/Math.h"
#include "carla/road/element/Geometry.h"
#include "carla/road/element/RoadInfoElevation.h"
#include "carla/road/element/RoadInfoGeometry.h"
#include "carla/road/element/RoadInfoLaneOffset.h"
#include "carla/road/element/RoadInfoLaneWidth.h"
#include "carla/road/LaneSection.h"
#include "carla/road/MapData.h"
#include "carla/road/Road.h"

namespace carla {
namespace road {

  // 获取当前车道所在的车道段
  const LaneSection *Lane::GetLaneSection() const {
    return _lane_section; // 返回车道段指针
  }

  // 获取当前车道所属的道路
  Road *Lane::GetRoad() const {
    DEBUG_ASSERT(_lane_section != nullptr); // 确保车道段不为空
    return _lane_section->GetRoad(); // 返回车道段对应的道路
  }

  // 获取当前车道的ID
  LaneId Lane::GetId() const {
    return _id; // 返回车道ID
  }

  // 获取当前车道的类型
  Lane::LaneType Lane::GetType() const {
    return _type; // 返回车道类型
  }

  // 获取当前车道的等级
  bool Lane::GetLevel() const {
    return _level; // 返回车道等级
  }

  // 获取当前车道在道路上的距离
  double Lane::GetDistance() const {
    DEBUG_ASSERT(_lane_section != nullptr); // 确保车道段不为空
    return _lane_section->GetDistance(); // 返回距离
  }

  // 获取当前车道的长度
  double Lane::GetLength() const {
    const auto *road = GetRoad(); // 获取所属道路
    DEBUG_ASSERT(road != nullptr); // 确保道路不为空
    const auto s = GetDistance(); // 获取车道的距离
    return road->UpperBound(s) - s; // 计算并返回车道长度
  }

  // 获取指定位置s处的车道宽度
  double Lane::GetWidth(const double s) const {
    RELEASE_ASSERT(s <= GetRoad()->GetLength()); // 确保s不超过道路的长度
    const auto width_info = GetInfo<element::RoadInfoLaneWidth>(s); // 获取宽度信息
    if(width_info != nullptr){
      return width_info->GetPolynomial().Evaluate(s); // 根据多项式计算并返回宽度
    }
    return 0.0f; // 如果没有宽度信息，返回0
  }

  // 检查当前车道是否为直线
  bool Lane::IsStraight() const {
    Road *road = GetRoad(); // 获取所属道路
    RELEASE_ASSERT(road != nullptr); // 确保道路不为空
    auto *geometry = road->GetInfo<element::RoadInfoGeometry>(GetDistance()); // 获取几何信息
    DEBUG_ASSERT(geometry != nullptr); // 确保几何信息不为空
    auto geometry_type = geometry->GetGeometry().GetType(); // 获取几何类型
    if (geometry_type != element::GeometryType::LINE) {
      return false; // 如果不是直线，返回false
    }
    // 检查距离是否在几何范围内
    if(GetDistance() < geometry->GetDistance() ||
        GetDistance() + GetLength() >
        geometry->GetDistance() + geometry->GetGeometry().GetLength()) {
      return false; // 如果不在范围内，返回false
    }
    // 检查车道偏移信息
    auto lane_offsets = GetInfos<element::RoadInfoLaneOffset>();
    for (auto *lane_offset : lane_offsets) {
      if (std::abs(lane_offset->GetPolynomial().GetC()) > 0 ||
          std::abs(lane_offset->GetPolynomial().GetD()) > 0) {
        return false; // 如果偏移量不为0，返回false
      }
    }
    // 检查道路高程信息
    auto elevations = road->GetInfos<element::RoadInfoElevation>();
    for (auto *elevation : elevations) {
      if (std::abs(elevation->GetPolynomial().GetC()) > 0 ||
          std::abs(elevation->GetPolynomial().GetD()) > 0) {
        return false; // 如果高程不为0，返回false
      }
    }
    return true; // 如果所有检查通过，返回true
  }

  /// 返回一对包含特定车道在给定s和车道迭代器下的宽度和切线
  template <typename T>
  static std::pair<double, double> ComputeTotalLaneWidth(
      const T container,
      const double s,
      const LaneId lane_id) {

    // lane_id不能为0
  RELEASE_ASSERT(lane_id != 0);  // 断言 lane_id 不为 0

const bool negative_lane_id = lane_id < 0;  // 判断 lane_id 是否为负
double dist = 0.0;  // 初始化距离
double tangent = 0.0;  // 初始化切线

for (const auto &lane : container) {  // 遍历所有车道
    auto info = lane.second.template GetInfo<element::RoadInfoLaneWidth>(s);  // 获取当前车道宽度信息
    RELEASE_ASSERT(info != nullptr);  // 断言信息不为空
    const auto current_polynomial = info->GetPolynomial();  // 获取当前多项式
    auto current_dist = current_polynomial.Evaluate(s);  // 计算当前距离
    auto current_tang = current_polynomial.Tangent(s);  // 计算当前切线

    if (lane.first != lane_id) {  // 如果当前车道 ID 不等于给定的 lane_id
        dist += negative_lane_id ? current_dist : -current_dist;  // 根据 lane_id 的正负更新距离
        tangent += negative_lane_id ? current_tang : -current_tang;  // 根据 lane_id 的正负更新切线
    } else {  // 如果当前车道 ID 等于给定的 lane_id
        current_dist *= 0.5;  // 将当前距离乘以 0.5
        dist += negative_lane_id ? current_dist : -current_dist;  // 更新距离
        tangent += (negative_lane_id ? current_tang : -current_tang) * 0.5;  // 更新切线
        break;  // 跳出循环
    }
}
return std::make_pair(dist, tangent);  // 返回距离和切线的对

geom::Transform Lane::ComputeTransform(const double s) const {  // 计算车道变换
    const Road* road = GetRoad();  // 获取道路对象
    DEBUG_ASSERT(road != nullptr);  // 断言道路对象不为空

    // 确保 s 小于等于道路长度且大于等于 0
    RELEASE_ASSERT(s <= road->GetLength());
    RELEASE_ASSERT(s >= 0.0);

    const auto* lane_section = GetLaneSection();  // 获取车道段
    DEBUG_ASSERT(lane_section != nullptr);  // 断言车道段不为空
    const std::map<LaneId, Lane>& lanes = lane_section->GetLanes();  // 获取车道映射

    // 检查当前 s 上是否存在 lane_id
    RELEASE_ASSERT(!lanes.empty());
    RELEASE_ASSERT(GetId() >= lanes.begin()->first);  // 断言 lane_id 在有效范围内
    RELEASE_ASSERT(GetId() <= lanes.rbegin()->first);  // 断言 lane_id 在有效范围内

    // 累积当前车道与车道 0 之间的横向偏移 (t) 和车道方向
    float lane_t_offset = 0.0f;  // 初始化车道横向偏移
    float lane_tangent = 0.0f;  // 初始化车道方向

    if (GetId() < 0) {  // 如果是右侧车道
        const auto side_lanes = MakeListView(
            std::make_reverse_iterator(lanes.lower_bound(0)), lanes.rend());  // 获取从 0 到当前车道的车道列表
        const auto computed_width =
            ComputeTotalLaneWidth(side_lanes, s, GetId());  // 计算总车道宽度
        lane_t_offset = static_cast<float>(computed_width.first);  // 设置车道横向偏移
        lane_tangent = static_cast<float>(computed_width.second);  // 设置车道方向
    }
    else if (GetId() > 0) {  // 如果是左侧车道
        const auto side_lanes = MakeListView(lanes.lower_bound(1), lanes.end());  // 获取从 1 到当前车道的车道列表
        const auto computed_width =
            ComputeTotalLaneWidth(side_lanes, s, GetId());  // 计算总车道宽度
        lane_t_offset = static_cast<float>(computed_width.first);  // 设置车道横向偏移
        lane_tangent = static_cast<float>(computed_width.second);  // 设置车道方向
    }

    // 计算当前 s 的道路（车道 0）的“laneOffset”切线
    const auto lane_offset_info = road->GetInfo<element::RoadInfoLaneOffset>(s);  // 获取车道偏移信息
    const auto lane_offset_tangent =
        static_cast<float>(lane_offset_info->GetPolynomial().Tangent(s));  // 获取车道偏移切线

    // 用当前 s 更新道路切线，减去“laneOffset”信息
    lane_tangent -= lane_offset_tangent;

    // 获取在当前 s 的车道中心的有向点
    element::DirectedPoint dp = road->GetDirectedPointIn(s);

    // 从道路中心转换到车道中心
    dp.ApplyLateralOffset(lane_t_offset);

    // 用当前 s 更新车道切线，减去道路的“laneOffset”
    dp.tangent -= lane_tangent;

    // Unreal 的 Y 轴转换
    dp.location.y *= -1;  // 反转 Y 轴
    dp.tangent *= -1;  // 反转切线

    geom::Rotation rot(
        geom::Math::ToDegrees(static_cast<float>(dp.pitch)),  // 将俯仰角转换为度
        geom::Math::ToDegrees(static_cast<float>(dp.tangent)),  // 将切线角转换为度
        0.0f);  // Z 轴角度设为 0    // Fix the direction of the possitive lanes
    if (GetId() > 0) {  // 如果车道 ID 大于 0
        rot.yaw += 180.0f;  // 将偏航角加上 180 度
        rot.pitch = 360.0f - rot.pitch;  // 将俯仰角调整为 360 度减去当前俯仰角
    }

    return geom::Transform(dp.location, rot);  // 返回位置和旋转变换
}

std::pair<geom::Vector3D, geom::Vector3D> Lane::GetCornerPositions(  // 定义获取车道角落位置的方法
    const double s, const float extra_width) const {  // 接受参数 s 和额外的宽度
    const Road *road = GetRoad();  // 获取道路对象
    DEBUG_ASSERT(road != nullptr);  // 断言道路对象不为空

    const auto *lane_section = GetLaneSection();  // 获取车道段
    DEBUG_ASSERT(lane_section != nullptr);  // 断言车道段不为空
    const std::map<LaneId, Lane> &lanes = lane_section->GetLanes();  // 获取车道映射

    // 检查当前 s 上是否存在 lane_id
    RELEASE_ASSERT(!lanes.empty());  // 断言车道不为空
    RELEASE_ASSERT(GetId() >= lanes.begin()->first);  // 断言 lane_id 不小于最小值
    RELEASE_ASSERT(GetId() <= lanes.rbegin()->first);  // 断言 lane_id 不大于最大值

    float lane_t_offset = 0.0f;  // 初始化车道横向偏移

    if (GetId() < 0) {  // 如果是右侧车道
        // 获取从 0 到当前车道的车道列表
        const auto side_lanes = MakeListView(
            std::make_reverse_iterator(lanes.lower_bound(0)), lanes.rend());
        const auto computed_width =
            ComputeTotalLaneWidth(side_lanes, s, GetId());  // 计算总车道宽度
        lane_t_offset = static_cast<float>(computed_width.first);  // 设置车道横向偏移
    } else if (GetId() > 0) {  // 如果是左侧车道
        // 获取从 1 到当前车道的车道列表
        const auto side_lanes = MakeListView(lanes.lower_bound(1), lanes.end());
        const auto computed_width =
            ComputeTotalLaneWidth(side_lanes, s, GetId());  // 计算总车道宽度
        lane_t_offset = static_cast<float>(computed_width.first);  // 设置车道横向偏移
    }

    float lane_width = static_cast<float>(GetWidth(s)) / 2.0f;  // 获取当前车道宽度的一半
    if (extra_width != 0.f && road->IsJunction() && GetType() == Lane::LaneType::Driving) {  // 如果有额外宽度且是交叉口且车道类型为驾驶
        lane_width += extra_width;  // 增加额外宽度
    }

    // 获取在给定 s 上道路中心的两个点
    element::DirectedPoint dp_r, dp_l;  // 初始化右侧和左侧的有向点
    dp_r = dp_l = road->GetDirectedPointIn(s);  // 获取道路中心的有向点

    // 从道路中心转换到每个车道角落
    dp_r.ApplyLateralOffset(lane_t_offset + lane_width);  // 右侧车道角落
    dp_l.ApplyLateralOffset(lane_t_offset - lane_width);  // 左侧车道角落

    // Unreal 的 Y 轴处理
    dp_r.location.y *= -1;  // 反转右侧点的 Y 坐标
    dp_l.location.y *= -1;  // 反转左侧点的 Y 坐标

    // 对人行道应用偏移
    if (GetType() == LaneType::Sidewalk) {  // 如果车道类型为人行道
        // RoadRunner 当前不导出该信息，作为临时解决方案，15.24 cm 是大多数 RoadRunner 人行道匹配的高度
        dp_r.location.z += 0.1524f;  // 右侧点的 Z 坐标增加 0.1524 米
        dp_l.location.z += 0.1524f;  // 左侧点的 Z 坐标增加 0.1524 米
        /// @TODO: 使用 OpenDRIVE 5.3.7.2.1.1.9 车道高度记录
    }

    return std::make_pair(dp_r.location, dp_l.location);  // 返回右侧和左侧点的位置
}