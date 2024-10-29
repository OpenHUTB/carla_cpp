// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <memory>
#include "carla/client/Actor.h"/// @brief 包含CARLA客户端中Actor类的定义
#include "carla/trafficmanager/SimpleWaypoint.h"/// @brief 包含CARLA交通管理器中SimpleWaypoint类的定义
/**
 * @namespace carla::traffic_manager
 * @brief CARLA交通管理器的命名空间。
 */
namespace carla {
namespace traffic_manager {
	/**
	 * @brief Actor的智能指针类型别名。
	 * @details 这是一个指向carla::client::Actor对象的共享指针。
	 */
using ActorPtr = carla::SharedPtr<carla::client::Actor>;
/**
 * @brief 路径类型别名。
 * @details 这是一个包含cg::Location对象的向量，用于表示一条路径。
 */
using Path = std::vector<cg::Location>;
/**
 * @brief 路线类型别名。
 * @details 这是一个包含uint8_t的向量，用于表示一条路线（可能是一个编码的路径或导航指令）。
 */
using Route = std::vector<uint8_t>;
/**
 * @brief Waypoint的智能指针类型别名。
 * @details 这是一个指向carla::client::Waypoint对象的共享指针。
 */
using WaypointPtr = carla::SharedPtr<carla::client::Waypoint>;
/**
 * @brief 动作类型别名。
 * @details 这是一个包含RoadOption和WaypointPtr的pair，表示一个动作，其中RoadOption表示行驶选项，WaypointPtr表示该动作的目标点。
 */
using Action = std::pair<RoadOption, WaypointPtr>;
/**
 * @brief 动作缓冲区类型别名。
 * @details 这是一个包含Action的向量，用于存储一系列的动作。
 */
using ActionBuffer = std::vector<Action>;


/**
///  * @class TrafficManagerBase
///  * @brief 此类的功能是通过消息传递机制适当地整合交通管理器的各个阶段。
///  */
class TrafficManagerBase {

public:
	/**
   * @brief 启动交通管理器。
   */
  virtual void Start() = 0;

  /**
 * @brief 停止交通管理器。
 */
  virtual void Stop() = 0;

  /**
 * @brief 释放交通管理器。
 */
  virtual void Release() = 0;

  /**
 * @brief 重置交通管理器。
 */
  virtual void Reset() = 0;

  /**
  * @brief 单例生命周期管理的受保护构造函数。
  */
  TrafficManagerBase() {};

  /**
  * @brief 析构函数。
  */
  virtual ~TrafficManagerBase() {};

  /**
 * @brief 此方法向交通管理器注册车辆。
 * @param actor_list 车辆列表。
 */
  virtual void RegisterVehicles(const std::vector<ActorPtr> &actor_list) = 0;

  /**
 * @brief 此方法从交通管理器注销车辆。
 * @param actor_list 车辆列表。
 */
  virtual void UnregisterVehicles(const std::vector<ActorPtr> &actor_list) = 0;

  /**
   * @brief 设置车辆相对于限速的速度百分比降低。
   * 如果小于0，则表示百分比增加。
   * @param actor 车辆指针。
   * @param percentage 百分比值。
   */
  virtual void SetPercentageSpeedDifference(const ActorPtr &actor, const float percentage) = 0;

  /**
 * @brief 设置车辆相对于中心线的车道偏移量。
 * 正值表示向右偏移，负值表示向左偏移。
 * @param actor 车辆指针。
 * @param offset 偏移量。
 */
  virtual void SetLaneOffset(const ActorPtr &actor, const float offset) = 0;

  /**
 * @brief 设置车辆的精确期望速度。
 * @param actor 车辆指针。
 * @param value 速度值。
 */
  virtual void SetDesiredSpeed(const ActorPtr &actor, const float value) = 0;

  /**
 * @brief 设置全局相对于限速的速度百分比降低。
 * 如果小于0，则表示百分比增加。
 * @param percentage 百分比值。
 */
  virtual void SetGlobalPercentageSpeedDifference(float const percentage) = 0;

  /**
 * @brief 设置全局相对于中心线的车道偏移量。
 * 正值表示向右偏移，负值表示向左偏移。
 * @param offset 偏移量。
 */
  virtual void SetGlobalLaneOffset(float const offset) = 0;

  /// Method to set the automatic management of the vehicle lights
  virtual void SetUpdateVehicleLights(const ActorPtr &actor, const bool do_update) = 0;

  /// Method to set collision detection rules between vehicles.
  virtual void SetCollisionDetection(const ActorPtr &reference_actor, const ActorPtr &other_actor, const bool detect_collision) = 0;

  /// Method to force lane change on a vehicle.
  /// Direction flag can be set to true for left and false for right.
  virtual void SetForceLaneChange(const ActorPtr &actor, const bool direction) = 0;

  /// Enable/disable automatic lane change on a vehicle.
  virtual void SetAutoLaneChange(const ActorPtr &actor, const bool enable) = 0;

  /// Method to specify how much distance a vehicle should maintain to
  /// the leading vehicle.
  virtual void SetDistanceToLeadingVehicle(const ActorPtr &actor, const float distance) = 0;

  /// Method to specify the % chance of ignoring collisions with any walker.
  virtual void SetPercentageIgnoreWalkers(const ActorPtr &actor, const float perc) = 0;

  /// Method to specify the % chance of ignoring collisions with any vehicle.
  virtual void SetPercentageIgnoreVehicles(const ActorPtr &actor, const float perc) = 0;

  /// Method to specify the % chance of running any traffic light.
  virtual void SetPercentageRunningLight(const ActorPtr &actor, const float perc) = 0;

  /// Method to specify the % chance of running any traffic sign.
  virtual void SetPercentageRunningSign(const ActorPtr &actor, const float perc) = 0;

  /// Method to switch traffic manager into synchronous execution.
  virtual void SetSynchronousMode(bool mode) = 0;

  /// Method to set Tick timeout for synchronous execution.
  virtual void SetSynchronousModeTimeOutInMiliSecond(double time) = 0;

  /// Method to provide synchronous tick
  virtual bool SynchronousTick() = 0;

  /// Get carla episode information
  virtual  carla::client::detail::EpisodeProxy& GetEpisodeProxy() = 0;

  /// Method to set Global Distance to Leading Vehicle.
  virtual void SetGlobalDistanceToLeadingVehicle(const float dist) = 0;

  /// Method to set % to keep on the right lane.
  virtual void SetKeepRightPercentage(const ActorPtr &actor,const float percentage) = 0;

  /// Method to set % to randomly do a left lane change.
  virtual void SetRandomLeftLaneChangePercentage(const ActorPtr &actor, const float percentage) = 0;

  /// Method to set % to randomly do a right lane change.
  virtual void SetRandomRightLaneChangePercentage(const ActorPtr &actor, const float percentage) = 0;

  /// Method to set hybrid physics mode.
  virtual void SetHybridPhysicsMode(const bool mode_switch) = 0;

  /// Method to set hybrid physics radius.
  virtual void SetHybridPhysicsRadius(const float radius) = 0;

  /// Method to set randomization seed.
  virtual void SetRandomDeviceSeed(const uint64_t seed) = 0;

  /// Method to set Open Street Map mode.
  virtual void SetOSMMode(const bool mode_switch) = 0;

  /// Method to set our own imported path.
  virtual void SetCustomPath(const ActorPtr &actor, const Path path, const bool empty_buffer) = 0;

  /// Method to remove a path.
  virtual void RemoveUploadPath(const ActorId &actor_id, const bool remove_path) = 0;

  /// Method to update an already set path.
  virtual void UpdateUploadPath(const ActorId &actor_id, const Path path) = 0;

  /// Method to set our own imported route.
  virtual void SetImportedRoute(const ActorPtr &actor, const Route route, const bool empty_buffer) = 0;

  /// Method to remove a route.
  virtual void RemoveImportedRoute(const ActorId &actor_id, const bool remove_path) = 0;

  /// Method to update an already set route.
  virtual void UpdateImportedRoute(const ActorId &actor_id, const Route route) = 0;

  /// Method to set automatic respawn of dormant vehicles.
  virtual void SetRespawnDormantVehicles(const bool mode_switch) = 0;

  /// Method to set boundaries for respawning vehicles.
  virtual void SetBoundariesRespawnDormantVehicles(const float lower_bound, const float upper_bound) = 0;

  /// Method to set limits for boundaries when respawning vehicles.
  virtual void SetMaxBoundaries(const float lower, const float upper) = 0;

  /// Method to get the vehicle's next action.
  virtual Action GetNextAction(const ActorId &actor_id) = 0;

  /// Method to get the vehicle's action buffer.
  virtual ActionBuffer GetActionBuffer(const ActorId &actor_id) = 0;

  virtual void ShutDown() = 0;

protected:

};

} // namespace traffic_manager
} // namespace carla
