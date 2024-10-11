// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/geom/Math.h"

#include "carla/trafficmanager/SimpleWaypoint.h"

namespace carla {
namespace traffic_manager {

  using SimpleWaypointPtr = std::shared_ptr<SimpleWaypoint>; // 定义一个智能指针类型，指向SimpleWaypoint类的实例

  SimpleWaypoint::SimpleWaypoint(WaypointPtr _waypoint) { // 构造函数，接受一个WaypointPtr类型的参数
    waypoint = _waypoint; // 初始化成员waypoint
    next_left_waypoint = nullptr; // 初始化左侧下一个路点为空
    next_right_waypoint = nullptr; // 初始化右侧下一个路点为空
  }
  SimpleWaypoint::~SimpleWaypoint() {} // 析构函数

  std::vector<SimpleWaypointPtr> SimpleWaypoint::GetNextWaypoint() const { // 获取下一个路点
    return next_waypoints; // 返回下一个路点的向量
  }

  std::vector<SimpleWaypointPtr> SimpleWaypoint::GetPreviousWaypoint() const { // 获取上一个路点
    return previous_waypoints; // 返回上一个路点的向量
  }

  WaypointPtr SimpleWaypoint::GetWaypoint() const { // 获取当前的Waypoint
    return waypoint; // 返回waypoint
  }

  uint64_t SimpleWaypoint::GetId() const { // 获取当前路点的ID
    return waypoint->GetId(); // 返回waypoint的ID
  }

  SimpleWaypointPtr SimpleWaypoint::GetLeftWaypoint() { // 获取左侧下一个路点
    return next_left_waypoint; // 返回左侧下一个路点
  }

  SimpleWaypointPtr SimpleWaypoint::GetRightWaypoint() { // 获取右侧下一个路点
    return next_right_waypoint; // 返回右侧下一个路点
  }

  cg::Location SimpleWaypoint::GetLocation() const { // 获取当前路点的位置
    return waypoint->GetTransform().location; // 返回位置
  }

  cg::Vector3D SimpleWaypoint::GetForwardVector() const { // 获取当前路点的前进方向向量
    return waypoint->GetTransform().rotation.GetForwardVector(); // 返回前进方向向量
  }

  uint64_t SimpleWaypoint::SetNextWaypoint(const std::vector<SimpleWaypointPtr> &waypoints) { // 设置下一个路点
    for (auto &simple_waypoint: waypoints) { // 遍历给定的路点向量
      next_waypoints.push_back(simple_waypoint); // 将每个路点添加到next_waypoints中
    }
    return static_cast<uint64_t>(waypoints.size()); // 返回设置的路点数量
  }

  uint64_t SimpleWaypoint::SetPreviousWaypoint(const std::vector<SimpleWaypointPtr> &waypoints) { // 设置上一个路点
    for (auto &simple_waypoint: waypoints) { // 遍历给定的路点向量
      previous_waypoints.push_back(simple_waypoint); // 将每个路点添加到previous_waypoints中
    }
    return static_cast<uint64_t>(waypoints.size()); // 返回设置的路点数量
  }

  void SimpleWaypoint::SetLeftWaypoint(SimpleWaypointPtr &_waypoint) { // 设置左侧下一个路点
    const cg::Vector3D heading_vector = waypoint->GetTransform().GetForwardVector(); // 获取前进方向向量
    const cg::Vector3D relative_vector = GetLocation() - _waypoint->GetLocation(); // 计算相对位置向量
    if ((heading_vector.x * relative_vector.y - heading_vector.y * relative_vector.x) > 0.0f) { // 判断是否为左侧
      next_left_waypoint = _waypoint; // 设置左侧下一个路点
    }
  }

  void SimpleWaypoint::SetRightWaypoint(SimpleWaypointPtr &_waypoint) { // 设置右侧下一个路点
    const cg::Vector3D heading_vector = waypoint->GetTransform().GetForwardVector(); // 获取前进方向向量
    const cg::Vector3D relative_vector = GetLocation() - _waypoint->GetLocation(); // 计算相对位置向量
    if ((heading_vector.x * relative_vector.y - heading_vector.y * relative_vector.x) < 0.0f) { // 判断是否为右侧
      next_right_waypoint = _waypoint; // 设置右侧下一个路点
    }
  }

  float SimpleWaypoint::Distance(const cg::Location &location) const { // 计算与给定位置的距离
    return GetLocation().Distance(location); // 返回距离
  }

  float SimpleWaypoint::Distance(const SimpleWaypointPtr &other) const { // 计算与另一个路点的距离
    return GetLocation().Distance(other->GetLocation()); // 返回距离
  }

  float SimpleWaypoint::DistanceSquared(const cg::Location &location) const { // 计算与给定位置的平方距离
    return cg::Math::DistanceSquared(GetLocation(), location); // 返回平方距离
  }

  float SimpleWaypoint::DistanceSquared(const SimpleWaypointPtr &other) const { // 计算与另一个路点的平方距离
    return cg::Math::DistanceSquared(GetLocation(), other->GetLocation()); // 返回平方距离
  }

  bool SimpleWaypoint::CheckJunction() const { // 检查当前路点是否为交叉口
    return _is_junction; // 返回交叉口状态
  }

  void SimpleWaypoint::SetIsJunction(bool value) { // 设置当前路点为交叉口
    _is_junction = value; // 更新交叉口状态
  }

  bool SimpleWaypoint::CheckIntersection() const { // 检查当前路点是否有多个下一个路点
    return (next_waypoints.size() > 1); // 如果下一个路点数量大于1，则返回true
  }

  void SimpleWaypoint::SetGeodesicGridId(GeoGridId _geodesic_grid_id) { // 设置地理网格ID
    geodesic_grid_id = _geodesic_grid_id; // 更新地理网格ID
  }

  GeoGridId SimpleWaypoint::GetGeodesicGridId() { // 获取地理网格ID
    GeoGridId grid_id; // 声明变量存储网格ID
    if (waypoint->IsJunction()) { // 如果当前路点是交叉口
      grid_id = waypoint->GetJunctionId(); // 获取交叉口ID
    } else {
      grid_id = geodesic_grid_id; // 否则获取地理网格ID
    }
    return grid_id; // 返回网格ID
  }

  GeoGridId SimpleWaypoint::GetJunctionId() const { // 获取交叉口ID
    return waypoint->GetJunctionId(); // 返回交叉口ID
  }

  cg::Transform SimpleWaypoint::GetTransform() const { // 获取当前路点的变换信息
    return waypoint->GetTransform(); // 返回变换信息
  }

  void SimpleWaypoint::SetRoadOption(RoadOption _road_option) { // 设置道路选项
    road_option = _road_option; // 更新道路选项
  }

  RoadOption SimpleWaypoint::GetRoadOption() { // 获取道路选项
    return road_option; // 返回道路选项
  }

} // namespace traffic_manager
} // namespace carla