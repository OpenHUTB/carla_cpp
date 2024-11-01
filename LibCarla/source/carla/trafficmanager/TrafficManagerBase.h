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

  /**
 * @brief 设置车辆灯光的自动管理。
 * @param actor 车辆指针。
 * @param do_update 是否更新灯光管理。
 */
  virtual void SetUpdateVehicleLights(const ActorPtr &actor, const bool do_update) = 0;

  /**
  * @brief 设置车辆之间的碰撞检测规则。
  *
  * @param reference_actor 参考车辆指针。
  * @param other_actor 其他车辆指针。
  * @param detect_collision 是否检测碰撞。
  */
  virtual void SetCollisionDetection(const ActorPtr &reference_actor, const ActorPtr &other_actor, const bool detect_collision) = 0;

  /**
 * @brief 强制车辆换道。
 *
 * 方向标志可以设置为true表示向左换道，false表示向右换道。
 *
 * @param actor 车辆指针。
 * @param direction 换道方向。
 */
  virtual void SetForceLaneChange(const ActorPtr &actor, const bool direction) = 0;

  /**
 * @brief 启用/禁用车辆的自动换道。
 *
 * @param actor 车辆指针。
 * @param enable 是否启用自动换道。
 */
  virtual void SetAutoLaneChange(const ActorPtr &actor, const bool enable) = 0;

  /**
 * @brief 设置车辆与前车应保持的距离。
 *
 * @param actor 车辆指针。
 * @param distance 应保持的距离。
 */
  virtual void SetDistanceToLeadingVehicle(const ActorPtr &actor, const float distance) = 0;

  /**
  * @brief 设置车辆忽略行人的碰撞概率。
  *
  * @param actor 车辆指针。
  * @param perc 忽略行人的碰撞概率。
  */
  virtual void SetPercentageIgnoreWalkers(const ActorPtr &actor, const float perc) = 0;

  /**
 * @brief 设置车辆忽略其他车辆的碰撞概率。
 *
 * @param actor 车辆指针。
 * @param perc 忽略其他车辆的碰撞概率。
 */
  virtual void SetPercentageIgnoreVehicles(const ActorPtr &actor, const float perc) = 0;

  /**
 * @brief 设置车辆闯红灯的概率。
 *
 * @param actor 车辆指针。
 * @param perc 闯红灯的概率。
 */
  virtual void SetPercentageRunningLight(const ActorPtr &actor, const float perc) = 0;

  /**
  * @brief 设置车辆无视交通标志的概率。
  *
  * @param actor 车辆指针。
  * @param perc 无视交通标志的概率。
  */
  virtual void SetPercentageRunningSign(const ActorPtr &actor, const float perc) = 0;

  /**
 * @brief 将交通管理器切换到同步执行模式。
 *
 * @param mode 是否启用同步模式。
 */
  virtual void SetSynchronousMode(bool mode) = 0;

  /**
  * @brief 设置同步执行模式的Tick超时时间。
  *
  * @param time Tick超时时间（毫秒）。
  */
  virtual void SetSynchronousModeTimeOutInMiliSecond(double time) = 0;

  /**
 * @brief 提供同步Tick。
 *
 * @return 如果成功提供同步Tick，则返回true；否则返回false。
 */
  virtual bool SynchronousTick() = 0;

  /**
 * @brief 获取CARLA场景信息代理。
 *
 * @return 返回CARLA场景信息代理的引用。
 */
  virtual  carla::client::detail::EpisodeProxy& GetEpisodeProxy() = 0;

  /**
 * @brief 设置全局前车距离。
 *
 * @param dist 前车距离。
 */
  virtual void SetGlobalDistanceToLeadingVehicle(const float dist) = 0;

  /**
 * @brief 设置保持在右车道的百分比。
 *
 * @param actor 车辆指针。
 * @param percentage 保持在右车道的百分比。
 */
  virtual void SetKeepRightPercentage(const ActorPtr &actor,const float percentage) = 0;

  /**
   * @brief 设置随机左变道的百分比。
   *
   * @param actor 车辆指针。
   * @param percentage 随机左变道的百分比。
   */
  virtual void SetRandomLeftLaneChangePercentage(const ActorPtr &actor, const float percentage) = 0;

  /**
  * @brief 设置随机右变道的百分比。
  *
  * @param actor 车辆指针。
  * @param percentage 随机右变道的百分比。
  */
  virtual void SetRandomRightLaneChangePercentage(const ActorPtr &actor, const float percentage) = 0;

  /**
 * @brief 设置混合物理模式。
 *
 * @param mode_switch 是否启用混合物理模式。
 */
  virtual void SetHybridPhysicsMode(const bool mode_switch) = 0;

  /**
 * @brief 设置混合物理半径。
 *
 * @param radius 混合物理半径。
 */
  virtual void SetHybridPhysicsRadius(const float radius) = 0;

  /**
 * @brief 设置随机化种子。
 *
 * @param seed 随机化种子。
 */
  virtual void SetRandomDeviceSeed(const uint64_t seed) = 0;

  /**
 * @brief 设置Open Street Map模式。
 *
 * @param mode_switch 是否启用Open Street Map模式。
 */
  virtual void SetOSMMode(const bool mode_switch) = 0;

  /**
   * @brief 设置自定义导入路径。
   *
   * @param actor 车辆指针。
   * @param path 自定义路径。
   * @param empty_buffer 是否清空缓冲区。
   */
  virtual void SetCustomPath(const ActorPtr &actor, const Path path, const bool empty_buffer) = 0;

  /**
 * @brief 移除路径。
 *
 * @param actor_id 车辆ID。
 * @param remove_path 是否移除路径。
 */
  virtual void RemoveUploadPath(const ActorId &actor_id, const bool remove_path) = 0;

  /**
  * @brief 更新已设置的路径。
  *
  * @param actor_id 车辆ID。
  * @param path 更新后的路径。
  */
  virtual void UpdateUploadPath(const ActorId &actor_id, const Path path) = 0;

  /**
 * @brief 设置自定义导入路线。
 *
 * @param actor 车辆指针。
 * @param route 自定义路线。
 * @param empty_buffer 是否清空缓冲区。
 */
  virtual void SetImportedRoute(const ActorPtr &actor, const Route route, const bool empty_buffer) = 0;

  /**
 * @brief 移除路线。
 *
 * @param actor_id 车辆ID。
 * @param remove_path 是否移除路线。
 */
  virtual void RemoveImportedRoute(const ActorId &actor_id, const bool remove_path) = 0;

  /**
  * @brief 更新已设置的路线。
  *
  * @param actor_id 车辆ID。
  * @param route 更新后的路线。
  */
  virtual void UpdateImportedRoute(const ActorId &actor_id, const Route route) = 0;

  /**
 * @brief 设置休眠车辆的自动重生。
 *
 * @param mode_switch 是否启用休眠车辆的自动重生。
 */
  virtual void SetRespawnDormantVehicles(const bool mode_switch) = 0;

  /**
 * @brief 设置重生车辆的范围。
 *
 * @param lower_bound 范围下限。
 * @param upper_bound 范围上限。
 */
  virtual void SetBoundariesRespawnDormantVehicles(const float lower_bound, const float upper_bound) = 0;

  /**
 * @brief 设置范围限制。
 *
 * @param lower 范围下限。
 * @param upper 范围上限。
 */
  virtual void SetMaxBoundaries(const float lower, const float upper) = 0;

  /**
 * @brief 获取车辆的下一个动作。
 *
 * @param actor_id 车辆ID。
 * @return 车辆的下一个动作。
 */
  virtual Action GetNextAction(const ActorId &actor_id) = 0;

  /**
  * @brief 获取车辆的动作缓冲区。
  *
  * @param actor_id 车辆ID。
  * @return 车辆的动作缓冲区。
  */
  virtual ActionBuffer GetActionBuffer(const ActorId &actor_id) = 0;
  /**
 * @brief 关闭系统。
 */
  virtual void ShutDown() = 0;

protected:

};

} // namespace traffic_manager
} // namespace carla
