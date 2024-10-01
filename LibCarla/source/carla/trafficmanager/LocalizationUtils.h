// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/client/Actor.h"  // 引入演员类
#include "carla/client/ActorList.h"  // 引入演员列表类
#include "carla/client/Vehicle.h"  // 引入车辆类
#include "carla/client/World.h"  // 引入世界类
#include "carla/geom/Location.h"  // 引入位置几何类
#include "carla/road/RoadTypes.h"  // 引入道路类型定义
#include "carla/rpc/ActorId.h"  // 引入演员ID定义

#include "carla/trafficmanager/Constants.h"  // 引入交通管理常量
#include "carla/trafficmanager/SimpleWaypoint.h"  // 引入简单路点类
#include "carla/trafficmanager/TrackTraffic.h"  // 引入交通跟踪类

namespace carla {
namespace traffic_manager {

  namespace cc = carla::client;  // 为 carla::client 起别名 cc
  namespace cg = carla::geom;  // 为 carla::geom 起别名 cg
  using Actor = carla::SharedPtr<cc::Actor>;  // 定义 Actor 类型为共享指针
  using ActorId = carla::ActorId;  // 定义 ActorId 类型
  using ActorIdSet = std::unordered_set<ActorId>;  // 定义 ActorId 集合
  using SimpleWaypointPtr = std::shared_ptr<SimpleWaypoint>;  // 定义简单路点的智能指针类型
  using Buffer = std::deque<SimpleWaypointPtr>;  // 定义缓冲区为简单路点的双端队列
  using GeoGridId = carla::road::JuncId;  // 定义地理网格ID为道路交叉口ID
  using constants::Map::MAP_RESOLUTION;  // 引入地图分辨率常量
  using constants::Map::INV_MAP_RESOLUTION;  // 引入地图反分辨率常量

  /// 返回车辆朝向向量与指向下一个目标路点方向向量之间的叉积（z分量值）
  float DeviationCrossProduct(const cg::Location &reference_location,
                              const cg::Vector3D &heading_vector,
                              const cg::Location &target_location);
  /// 返回车辆朝向向量与指向下一个目标路点方向向量之间的点积
  float DeviationDotProduct(const cg::Location &reference_location,
                            const cg::Vector3D &heading_vector,
                            const cg::Location &target_location);

  // 将一个路点添加到路径缓冲区并更新路点跟踪
  void PushWaypoint(ActorId actor_id, TrackTraffic& track_traffic,
                    Buffer& buffer, SimpleWaypointPtr& waypoint);

  // 从路径缓冲区中移除一个路点并更新路点跟踪
  void PopWaypoint(ActorId actor_id, TrackTraffic& track_traffic,
                   Buffer& buffer, bool front_or_back=true);

  /// 根据目标点距离从路点缓冲区返回路点信息
  using TargetWPInfo = std::pair<SimpleWaypointPtr, uint64_t>;  // 定义目标路点信息为简单路点指针和无符号整数对
  TargetWPInfo GetTargetWaypoint(const Buffer& waypoint_buffer, const float& target_point_distance);  // 获取目标路点函数

} // namespace traffic_manager
} // namespace carla
