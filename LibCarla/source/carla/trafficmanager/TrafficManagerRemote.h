// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <condition_variable>/// @brief 引入条件变量类，用于线程间的同步
#include <mutex>/// @brief 引入互斥锁类，用于保护共享数据的访问
#include <vector>/// @brief 引入动态数组类，用于储存多个元素

#include "carla/client/Actor.h"/// @brief CARLA客户端Actor类的头文件，包含了Actor（参与者）的定义和接口
#include "carla/client/detail/EpisodeProxy.h"/// @brief CARLA客户端EpisodeProxy类的头文件，包含了与仿真片段（Episode）相关的操作和接口
#include "carla/trafficmanager/TrafficManagerBase.h"/// @brief CARLA交通管理器基础类的头文件，提供了交通管理器的基本功能和接口
#include "carla/trafficmanager/TrafficManagerClient.h"/// @brief CARLA交通管理器客户端类的头文件，用于与交通管理器进行通信和控制
/**
 * @namespace carla::traffic_manager
 * @brief CARLA仿真环境中的交通管理器命名空间。
 *
 * 该命名空间包含了与交通管理相关的类、函数和类型定义，用于实现交通流的控制和管理。
 */
namespace carla {
namespace traffic_manager {
	/**
	 * @brief 定义一个智能指针类型，指向carla::client::Actor类。
	 *
	 * 用于在交通管理器中方便地管理Actor（如车辆、行人等）的引用。
	 */
using ActorPtr = carla::SharedPtr<carla::client::Actor>;
/**
 * @brief 定义一个路径类型，由多个cg::Location组成。
 *
 * 用于表示一个Actor在地图上的移动路径。
 */
using Path = std::vector<cg::Location>;
/**
 * @brief 定义一个路由类型，由多个uint8_t组成。
 *
 * 用于表示一个Actor在交通网络中的路由信息。
 */
using Route = std::vector<uint8_t>;
/**
 * @class TrafficManagerRemote
 * @brief 远程交通管理器类，通过消息传递机制整合交通管理的各个阶段。
 *
 * 该类继承自TrafficManagerBase，提供了远程交通管理的功能，包括启动、停止、重置以及车辆的注册、注销和速度控制等。
 */
class TrafficManagerRemote : public TrafficManagerBase {

public:
	/**
	   * @brief 启动交通管理器。
	   *
	   * 初始化并启动交通管理器的各个组件，准备进行交通管理。
	   */
  void Start();
  /**
	 * @brief 停止交通管理器。
	 *
	 * 停止交通管理器的运行，释放相关资源。
	 */
  void Stop();
  /**
	 * @brief 释放交通管理器。
	 *
	 * 彻底释放交通管理器占用的资源，确保不再使用。
	 */
  void Release();
  /**
	 * @brief 重置交通管理器。
	 *
	 * 将交通管理器重置到初始状态，准备进行新的交通管理任务。
	 */
  void Reset();

  /// Constructor store remote location information.
  TrafficManagerRemote(const std::pair<std::string, uint16_t> &_serverTM, carla::client::detail::EpisodeProxy &episodeProxy);

  /// Destructor.
  virtual ~TrafficManagerRemote();

  /// This method registers a vehicle with the traffic manager.
  void RegisterVehicles(const std::vector<ActorPtr> &actor_list);

  /// This method unregisters a vehicle from traffic manager.
  void UnregisterVehicles(const std::vector<ActorPtr> &actor_list);

  /// Method to set a vehicle's % decrease in velocity with respect to the speed limit.
  /// If less than 0, it's a % increase.
  void SetPercentageSpeedDifference(const ActorPtr &actor, const float percentage);

  /// Method to set a lane offset displacement from the center line.
  /// Positive values imply a right offset while negative ones mean a left one.
  void SetLaneOffset(const ActorPtr &actor, const float offset);

  /// Set a vehicle's exact desired velocity.
  void SetDesiredSpeed(const ActorPtr &actor, const float value);

  /// Method to set a global % decrease in velocity with respect to the speed limit.
  /// If less than 0, it's a % increase.
  void SetGlobalPercentageSpeedDifference(float const percentage);

  /// Method to set a global lane offset displacement from the center line.
  /// Positive values imply a right offset while negative ones mean a left one.
  void SetGlobalLaneOffset(float const offset);

  /// Method to set the automatic management of the vehicle lights
  void SetUpdateVehicleLights(const ActorPtr &actor, const bool do_update);

  /// Method to set collision detection rules between vehicles.
  void SetCollisionDetection(const ActorPtr &reference_actor, const ActorPtr &other_actor, const bool detect_collision);

  /// Method to force lane change on a vehicle.
  /// Direction flag can be set to true for left and false for right.
  void SetForceLaneChange(const ActorPtr &actor, const bool direction);

  /// Enable/disable automatic lane change on a vehicle.
  void SetAutoLaneChange(const ActorPtr &actor, const bool enable);

  /// Method to specify how much distance a vehicle should maintain to
  /// the leading vehicle.
  void SetDistanceToLeadingVehicle(const ActorPtr &actor, const float distance);

  /// Method to specify Global Distance
  void SetGlobalDistanceToLeadingVehicle(const float distance);

  /// Method to specify the % chance of ignoring collisions with any walker.
  void SetPercentageIgnoreWalkers(const ActorPtr &actor, const float perc);

  /// Method to specify the % chance of ignoring collisions with any vehicle.
  void SetPercentageIgnoreVehicles(const ActorPtr &actor, const float perc);

  /// Method to specify the % chance of running any traffic light
  void SetPercentageRunningLight(const ActorPtr &actor, const float perc);

  /// Method to specify the % chance of running any traffic sign
  void SetPercentageRunningSign(const ActorPtr &actor, const float perc);

  /// Method to switch traffic manager into synchronous execution.
  void SetSynchronousMode(bool mode);

  /// Method to set Tick timeout for synchronous execution.
  void SetSynchronousModeTimeOutInMiliSecond(double time);

  /// Method to set % to keep on the right lane.
  void SetKeepRightPercentage(const ActorPtr &actor, const float percentage);

  /// Method to set % to randomly do a left lane change.
  void SetRandomLeftLaneChangePercentage(const ActorPtr &actor, const float percentage);

  /// Method to set % to randomly do a right lane change.
  void SetRandomRightLaneChangePercentage(const ActorPtr &actor, const float percentage);

  /// Method to set hybrid physics mode.
  void SetHybridPhysicsMode(const bool mode_switch);

  /// Method to set hybrid physics radius.
  void SetHybridPhysicsRadius(const float radius);

  /// Method to set Open Street Map mode.
  void SetOSMMode(const bool mode_switch);

  /// Method to set our own imported path.
  void SetCustomPath(const ActorPtr &actor, const Path path, const bool empty_buffer);

  /// Method to remove a path.
  void RemoveUploadPath(const ActorId &actor_id, const bool remove_path);

  /// Method to update an already set path.
  void UpdateUploadPath(const ActorId &actor_id, const Path path);

  /// Method to set our own imported route.
  void SetImportedRoute(const ActorPtr &actor, const Route route, const bool empty_buffer);

  /// Method to remove a route.
  void RemoveImportedRoute(const ActorId &actor_id, const bool remove_path);

  /// Method to update an already set route.
  void UpdateImportedRoute(const ActorId &actor_id, const Route route);

  /// Method to set automatic respawn of dormant vehicles.
  void SetRespawnDormantVehicles(const bool mode_switch);

  // Method to set boundaries to respawn of dormant vehicles.
  void SetBoundariesRespawnDormantVehicles(const float lower_bound, const float upper_bound);

  // Method to set boundaries to respawn of dormant vehicles.
  void SetMaxBoundaries(const float lower, const float upper);

  virtual void ShutDown();

  /// Method to get the vehicle's next action.
  Action GetNextAction(const ActorId &actor_id);

  /// Method to get the vehicle's action buffer.
  ActionBuffer GetActionBuffer(const ActorId &actor_id);

  /// Method to provide synchronous tick
  bool SynchronousTick();

  /// Get CARLA episode information.
  carla::client::detail::EpisodeProxy& GetEpisodeProxy();

  /// Method to check server is alive or not.
  void HealthCheckRemoteTM();

  /// Method to set randomization seed.
  void SetRandomDeviceSeed(const uint64_t seed);

private:

  /// Remote client using the IP and port information it connects to
  /// as remote RPC traffic manager server.
  TrafficManagerClient client;

  /// CARLA client connection object.
  carla::client::detail::EpisodeProxy episodeProxyTM;

  std::condition_variable _cv;

  std::mutex _mutex;

  bool _keep_alive = true;
};

} // namespace traffic_manager
} // namespace carla
