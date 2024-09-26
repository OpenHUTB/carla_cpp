
#pragma once // 防止头文件重复包含

#include <memory> // 引入智能指针的支持

#if defined(__clang__) // 如果使用 clang 编译器
#  pragma clang diagnostic push // 保存当前警告状态
#  pragma clang diagnostic ignored "-Wshadow" // 忽略变量遮蔽的警告
#endif
#include "boost/geometry.hpp" // 引入 Boost.Geometry 库
#include "boost/geometry/geometries/geometries.hpp" // 包含几何体的定义
#include "boost/geometry/geometries/point_xy.hpp" // 包含二维点的定义
#include "boost/geometry/geometries/polygon.hpp" // 包含多边形的定义
#if defined(__clang__) // 如果使用 clang 编译器
#  pragma clang diagnostic pop // 恢复之前的警告状态
#endif

#include "carla/trafficmanager/DataStructures.h" // 引入数据结构的定义
#include "carla/trafficmanager/Parameters.h" // 引入参数的定义
#include "carla/trafficmanager/RandomGenerator.h" // 引入随机数生成器的定义
#include "carla/trafficmanager/SimulationState.h" // 引入仿真状态的定义
#include "carla/trafficmanager/Stage.h" // 引入阶段的定义

namespace carla { // 定义 carla 命名空间
namespace traffic_manager { // 定义 traffic_manager 命名空间

struct GeometryComparison { // 定义几何比较结构
  double reference_vehicle_to_other_geodesic; // 参考车辆到其他车辆的测地线距离
  double other_vehicle_to_reference_geodesic; // 其他车辆到参考车辆的测地线距离
  double inter_geodesic_distance; // 车辆间的测地线距离
  double inter_bbox_distance; // 车辆间的边界框距离
};

struct CollisionLock { // 定义碰撞锁结构
  double distance_to_lead_vehicle; // 到前方车辆的距离
  double initial_lock_distance; // 初始锁定距离
  ActorId lead_vehicle_id; // 前方车辆的 ID
};
using CollisionLockMap = std::unordered_map<ActorId, CollisionLock>; // 定义碰撞锁映射表

namespace cc = carla::client; // 简化 carla::client 的命名空间
namespace bg = boost::geometry; // 简化 boost::geometry 的命名空间

using Buffer = std::deque<std::shared_ptr<SimpleWaypoint>>; // 定义 waypoint 缓冲区
using BufferMap = std::unordered_map<carla::ActorId, Buffer>; // 定义缓冲区映射表
using LocationVector = std::vector<cg::Location>; // 定义位置向量
using GeodesicBoundaryMap = std::unordered_map<ActorId, LocationVector>; // 定义测地边界映射表
using GeometryComparisonMap = std::unordered_map<uint64_t, GeometryComparison>; // 定义几何比较映射表
using Polygon = bg::model::polygon<bg::model::d2::point_xy<double>>; // 定义多边形类型

/// 该类具有检测与附近演员潜在碰撞的功能。
class CollisionStage : Stage { // 定义 CollisionStage 类，继承自 Stage
private:
  const std::vector<ActorId> &vehicle_id_list; // 车辆 ID 列表
  const SimulationState &simulation_state; // 仿真状态
  const BufferMap &buffer_map; // 缓冲区映射表
  const TrackTraffic &track_traffic; // 跟踪交通
  const Parameters &parameters; // 参数
  CollisionFrame &output_array; // 输出数组
  CollisionLockMap collision_locks; // 存储阻塞的前方车辆信息
  GeometryComparisonMap geometry_cache; // 存储车辆边界的几何比较结果
  GeodesicBoundaryMap geodesic_boundary_map; // 存储车辆的测地边界
  RandomGenerator &random_device; // 随机数生成器

  // 方法：确定车辆是否与另一辆车处于碰撞路径
  std::pair<bool, float> NegotiateCollision(const ActorId reference_vehicle_id,
                                            const ActorId other_actor_id,
                                            const uint64_t reference_junction_look_ahead_index);

  // 方法：计算车辆前方的边界框扩展长度
  float GetBoundingBoxExtention(const ActorId actor_id);

  // 方法：计算车辆边界的多边形点
  LocationVector GetBoundary(const ActorId actor_id);

  // 方法：构造车辆路径边界的多边形点
  LocationVector GetGeodesicBoundary(const ActorId actor_id);

  Polygon GetPolygon(const LocationVector &boundary); // 获取多边形对象

  // 方法：比较路径边界、车辆的边界框，并缓存当前更新周期的结果
  GeometryComparison GetGeometryBetweenActors(const ActorId reference_vehicle_id,
                                              const ActorId other_actor_id);

  // 方法：绘制路径边界
  void DrawBoundary(const LocationVector &boundary);

public:
  // 构造函数：初始化 CollisionStage 对象
  CollisionStage(const std::vector<ActorId> &vehicle_id_list,
                 const SimulationState &simulation_state,
                 const BufferMap &buffer_map,
                 const TrackTraffic &track_traffic,
                 const Parameters &parameters,
                 CollisionFrame &output_array,
                 RandomGenerator &random_device);

  void Update (const unsigned long index) override; // 更新方法

  void RemoveActor(const ActorId actor_id) override; // 移除参与者方法

  void Reset() override; // 重置方法

  // 方法：清除当前更新周期的缓存
  void ClearCycleCache();
};

} // namespace traffic_manager
} // namespace carla
