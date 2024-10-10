// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <memory.h> // 引入内存操作相关的头文件

#include "carla/client/Waypoint.h" // 引入Carla客户端的Waypoint类
#include "carla/geom/Location.h" // 引入Carla几何位置类
#include "carla/geom/Transform.h" // 引入Carla变换类
#include "carla/geom/Vector3D.h" // 引入Carla三维向量类
#include "carla/Memory.h" // 引入Carla内存管理相关的头文件
#include "carla/road/RoadTypes.h" // 引入Carla道路类型相关的头文件

namespace carla {
namespace traffic_manager {

  namespace cc = carla::client; // 简化命名空间cc为carla::client
  namespace cg = carla::geom; // 简化命名空间cg为carla::geom
  using WaypointPtr = carla::SharedPtr<cc::Waypoint>; // 定义WaypointPtr为Waypoint的智能指针类型
  using GeoGridId = carla::road::JuncId; // 定义GeoGridId为交叉口ID类型
  enum class RoadOption : uint8_t { // 定义道路选项的枚举类
    Void = 0, // 无效选项
    Left = 1, // 向左
    Right = 2, // 向右
    Straight = 3, // 直行
    LaneFollow = 4, // 跟随车道
    ChangeLaneLeft = 5, // 向左变道
    ChangeLaneRight = 6, // 向右变道
    RoadEnd = 7 // 道路结束
  };

  /// 该类是Carla的Waypoint对象的简单封装。
  /// 该类用于表示世界地图的离散样本。
  class SimpleWaypoint {

    using SimpleWaypointPtr = std::shared_ptr<SimpleWaypoint>; // 定义SimpleWaypointPtr为SimpleWaypoint的共享指针

  private:

    /// 指向Carla的Waypoint对象的指针，作为此类的封装对象。
    WaypointPtr waypoint;
    /// 指向下一个连接waypoint的指针列表。
    std::vector<SimpleWaypointPtr> next_waypoints;
    /// 指向前一个连接waypoint的指针列表。
    std::vector<SimpleWaypointPtr> previous_waypoints;
    /// 指向左侧变道waypoint的指针。
    SimpleWaypointPtr next_left_waypoint;
    /// 指向右侧变道waypoint的指针。
    SimpleWaypointPtr next_right_waypoint;
    /// 当前waypoint的RoadOption。
    RoadOption road_option = RoadOption::Void; // 默认设置为无效选项
    /// 整数，用于将waypoint放置到地理网格中。
    GeoGridId geodesic_grid_id = 0; // 初始化为0
    // 布尔值，表示waypoint是否属于交叉口。
    bool _is_junction = false; // 默认设置为false

  public:

    SimpleWaypoint(WaypointPtr _waypoint); // 构造函数，初始化waypoint
    ~SimpleWaypoint(); // 析构函数

    /// 返回此waypoint的位置信息。
    cg::Location GetLocation() const;

    /// 返回指向carla::waypoint的共享指针。
    WaypointPtr GetWaypoint() const;

    /// 返回下一个waypoint的列表。
    std::vector<SimpleWaypointPtr> GetNextWaypoint() const;

    /// 返回前一个waypoint的列表。
    std::vector<SimpleWaypointPtr> GetPreviousWaypoint() const;

    /// 返回沿waypoint方向的向量。
    cg::Vector3D GetForwardVector() const;

    /// 返回waypoint的唯一ID。
    uint64_t GetId() const;

    /// 此方法用于设置下一个waypoint。
    uint64_t SetNextWaypoint(const std::vector<SimpleWaypointPtr> &next_waypoints);

    /// 此方法用于设置前一个waypoint。
    uint64_t SetPreviousWaypoint(const std::vector<SimpleWaypointPtr> &next_waypoints);
   /// 此方法用于设置最近的左侧waypoint，以便进行变道。
    void SetLeftWaypoint(SimpleWaypointPtr &waypoint);

    /// 此方法用于设置最近的右侧waypoint，以便进行变道。
    void SetRightWaypoint(SimpleWaypointPtr &waypoint);

    /// 此方法用于获取最近的左侧waypoint。
    SimpleWaypointPtr GetLeftWaypoint();

    /// 此方法用于获取最近的右侧waypoint。
    SimpleWaypointPtr GetRightWaypoint();

    /// 访问器方法，用于设置地理网格ID。
    void SetGeodesicGridId(GeoGridId _geodesic_grid_id);
    
    /// 访问器方法，用于获取地理网格ID。
    GeoGridId GetGeodesicGridId();

    /// 方法用于获取waypoint的交叉口ID。
    GeoGridId GetJunctionId() const;

    /// 计算对象的waypoint到给定位置的距离。
    float Distance(const cg::Location &location) const;

    /// 计算当前SimpleWaypoint对象与另一个SimpleWaypoint对象的距离。
    float Distance(const SimpleWaypointPtr &other) const;

    /// 计算到给定位置的距离的平方。
    float DistanceSquared(const cg::Location &location) const;

    /// 计算到其他waypoints的距离的平方。
    float DistanceSquared(const SimpleWaypointPtr &other) const;

    /// 如果对象的waypoint属于交叉口，则返回true。
    bool CheckJunction() const;

    /// 此方法用于设置waypoint是否属于交叉口。
    void SetIsJunction(bool value);

    /// 如果对象的waypoint属于交叉口（不使用OpenDrive），则返回true。
    bool CheckIntersection() const;

    /// 返回当前waypoint的变换对象。
    cg::Transform GetTransform() const;

    // 访问器方法，用于设置道路选项。
    void SetRoadOption(RoadOption _road_option);
    
    // 访问器方法，用于获取道路选项。
    RoadOption GetRoadOption();
  };

} // namespace traffic_manager
} // namespace carla
