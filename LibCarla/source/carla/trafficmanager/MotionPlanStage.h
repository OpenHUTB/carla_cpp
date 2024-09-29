
/// This file has functionality for motion planning based on information
/// from localization, collision avoidance and traffic light response.

#pragma once

#include "carla/trafficmanager/DataStructures.h"
#include "carla/trafficmanager/InMemoryMap.h"
#include "carla/trafficmanager/LocalizationUtils.h"
#include "carla/trafficmanager/Parameters.h"
#include "carla/trafficmanager/RandomGenerator.h"
#include "carla/trafficmanager/SimulationState.h"
#include "carla/trafficmanager/Stage.h"
#include "carla/trafficmanager/TrackTraffic.h"

namespace carla {
namespace traffic_manager {
// 使用 shared_ptr 定义一个指向 InMemoryMap 的别名
using LocalMapPtr = std::shared_ptr<InMemoryMap>;
// 定义一个无序映射，键为字符串，值为指向 client::Actor 的共享指针
using TLMap = std::unordered_map<std::string, SharedPtr<client::Actor>>;
// 定义 MotionPlanStage 类，继承自 Stage 类
class MotionPlanStage: Stage {
private:
  const std::vector<ActorId> &vehicle_id_list;// 引用车辆 ID 列表
  SimulationState &simulation_state;// 引用模拟状态对象
  const Parameters &parameters; // 引用参数对象
  const BufferMap &buffer_map;// 引用缓冲区映射对象
  TrackTraffic &track_traffic;// 引用跟踪交通对象
  // PID paramenters for various road conditions.
  const std::vector<float> urban_longitudinal_parameters;
  const std::vector<float> highway_longitudinal_parameters;
  const std::vector<float> urban_lateral_parameters;
  const std::vector<float> highway_lateral_parameters;
  const LocalizationFrame &localization_frame;// 引用定位帧对象
  const CollisionFrame &collision_frame;// 引用碰撞帧对象
  const TLFrame &tl_frame;
  const cc::World &world;
  // Structure holding the controller state for registered vehicles.
  std::unordered_map<ActorId, StateEntry> pid_state_map;
  // Structure to keep track of duration between teleportation
  // in hybrid physics mode.
  std::unordered_map<ActorId, cc::Timestamp> teleportation_instance;
  ControlFrame &output_array;
  cc::Timestamp current_timestamp;// 当前时间戳
  RandomGenerator &random_device;// 引用随机数生成器对象
  const LocalMapPtr &local_map;// 引用本地地图指针对象
// 处理碰撞的私有方法
  std::pair<bool, float> CollisionHandling(const CollisionHazardData &collision_hazard,
                                           const bool tl_hazard,
                                           const cg::Vector3D ego_velocity,
                                           const cg::Vector3D ego_heading,
                                           const float max_target_velocity);
// 判断在路口后是否安全的私有方法
  bool SafeAfterJunction(const LocalizationData &localization,
                         const bool tl_hazard,
                         const bool collision_emergency_stop);
 // 根据地标获取目标速度的私有方法
  float GetLandmarkTargetVelocity(const SimpleWaypoint& waypoint,
                                  const cg::Location vehicle_location,
                                  const ActorId actor_id,
                                  float max_target_velocity);
// 根据路点缓冲区获取转弯目标速度的私有方法
  float GetTurnTargetVelocity(const Buffer &waypoint_buffer,
                              float max_target_velocity);
// 获取三点圆半径的私有方法
  float GetThreePointCircleRadius(cg::Location first_location,
                                  cg::Location middle_location,
                                  cg::Location last_location);

public:
// 构造函数，初始化成员变量
  MotionPlanStage(const std::vector<ActorId> &vehicle_id_list,
                  SimulationState &simulation_state,
                  const Parameters &parameters,
                  const BufferMap &buffer_map,
                  TrackTraffic &track_traffic,
                  const std::vector<float> &urban_longitudinal_parameters,
                  const std::vector<float> &highway_longitudinal_parameters,
                  const std::vector<float> &urban_lateral_parameters,
                  const std::vector<float> &highway_lateral_parameters,
                  const LocalizationFrame &localization_frame,
                  const CollisionFrame &collision_frame,
                  const TLFrame &tl_frame,
                  const cc::World &world,
                  ControlFrame &output_array,
                  RandomGenerator &random_device,
                  const LocalMapPtr &local_map);
 // 更新方法，根据给定的索引进行更新
  void Update(const unsigned long index);
// 移除指定 actor 的方法
  void RemoveActor(const ActorId actor_id);
// 重置方法
  void Reset();
};

} // namespace traffic_manager
} // namespace carla
