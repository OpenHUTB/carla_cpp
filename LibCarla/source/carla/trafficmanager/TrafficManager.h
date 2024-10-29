// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <map>///< 引入map容器，用于存储键值对
#include <mutex>///< 引入互斥锁，用于线程间的同步
#include <vector>///< 引入动态数组容器，用于存储可变大小的数组

#include "carla/client/Actor.h"///< 引入CARLA客户端的Actor类，代表场景中的一个实体
#include "carla/trafficmanager/Constants.h"///< 引入交通管理器常量定义
#include "carla/trafficmanager/TrafficManagerBase.h"///< 引入交通管理器基类

namespace carla {
namespace traffic_manager {

using constants::Networking::TM_DEFAULT_PORT;///< 使用常量TM_DEFAULT_PORT，表示交通管理器的默认端口号

using ActorPtr = carla::SharedPtr<carla::client::Actor>;///< 定义Actor的智能指针类型，用于管理Actor对象的生命周期

/// \class TrafficManager  
/// \brief 该类通过使用消息传递机制，将交通管理器的各个阶段恰当地整合在一起
class TrafficManager {

public:
    /// \brief 公有构造函数，用于单例生命周期管理。  
    /// \param episode_proxy CARLA客户端的EpisodeProxy对象，代表一个场景会话。  
    /// \param port 交通管理器的端口号，默认为TM_DEFAULT_PORT
  explicit TrafficManager(
    carla::client::detail::EpisodeProxy episode_proxy,
    uint16_t port = TM_DEFAULT_PORT);
  /// \brief 拷贝构造函数，用于复制另一个TrafficManager对象。  
  /// \param other 要复制的TrafficManager对象。
  TrafficManager(const TrafficManager& other) {
    _port = other._port;
  }
  /// \brief 默认构造函数，不执行任何操作。
  TrafficManager() {};
  /// \brief 移动构造函数，用于移动另一个TrafficManager对象
  TrafficManager(TrafficManager &&) = default;
  /// \brief 拷贝赋值运算符，用于将另一个TrafficManager对象赋值给当前对象
  TrafficManager &operator=(const TrafficManager &) = default;
  /// \brief 移动赋值运算符，用于将另一个TrafficManager对象移动赋值给当前对象
  TrafficManager &operator=(TrafficManager &&) = default;
  /// \brief 释放TrafficManager对象，用于单例模式的清理工作
  static void Release();
  /// \brief 重置TrafficManager对象，用于单例模式的重置操作
  static void Reset();
  /// \brief 执行TrafficManager的Tick操作，通常用于周期性地更新状态
  static void Tick();

  uint16_t Port() const {
    return _port;
  }

  bool IsValidPort() const {
    // The first 1024 ports are reserved by the OS
    return (_port > 1023);
  }

  /// Method to set Open Street Map mode.
  void SetOSMMode(const bool mode_switch) {
    TrafficManagerBase* tm_ptr = GetTM(_port);
    if (tm_ptr != nullptr) {
      tm_ptr->SetOSMMode(mode_switch);
    }
  }

  /// Method to set our own imported path.
  void SetCustomPath(const ActorPtr &actor, const Path path, const bool empty_buffer) {
    TrafficManagerBase* tm_ptr = GetTM(_port);
    if (tm_ptr != nullptr) {
      tm_ptr->SetCustomPath(actor, path, empty_buffer);
    }
  }

  /// Method to remove a path.
  void RemoveUploadPath(const ActorId &actor_id, const bool remove_path) {
    TrafficManagerBase* tm_ptr = GetTM(_port);
    if (tm_ptr != nullptr) {
      tm_ptr->RemoveUploadPath(actor_id, remove_path);
    }
  }

  /// Method to update an already set path.
  void UpdateUploadPath(const ActorId &actor_id, const Path path) {
    TrafficManagerBase* tm_ptr = GetTM(_port);
    if (tm_ptr != nullptr) {
      tm_ptr->UpdateUploadPath(actor_id, path);
    }
  }

  /// Method to set our own imported route.
  void SetImportedRoute(const ActorPtr &actor, const Route route, const bool empty_buffer) {
    TrafficManagerBase* tm_ptr = GetTM(_port);
    if (tm_ptr != nullptr) {
      tm_ptr->SetImportedRoute(actor, route, empty_buffer);
    }
  }

  /// Method to remove a route.
  void RemoveImportedRoute(const ActorId &actor_id, const bool remove_path) {
    TrafficManagerBase* tm_ptr = GetTM(_port);
    if (tm_ptr != nullptr) {
      tm_ptr->RemoveImportedRoute(actor_id, remove_path);
    }
  }

  /// Method to update an already set route.
  void UpdateImportedRoute(const ActorId &actor_id, const Route route) {
    TrafficManagerBase* tm_ptr = GetTM(_port);
    if (tm_ptr != nullptr) {
      tm_ptr->UpdateImportedRoute(actor_id, route);
    }
  }

  /// Method to set if we are automatically respawning vehicles.
  void SetRespawnDormantVehicles(const bool mode_switch) {
    TrafficManagerBase* tm_ptr = GetTM(_port);
    if (tm_ptr != nullptr) {
      tm_ptr->SetRespawnDormantVehicles(mode_switch);
    }
  }
  /// Method to set boundaries for respawning vehicles.
  void SetBoundariesRespawnDormantVehicles(const float lower_bound, const float upper_bound) {
    TrafficManagerBase* tm_ptr = GetTM(_port);
    if (tm_ptr != nullptr) {
      tm_ptr->SetBoundariesRespawnDormantVehicles(lower_bound, upper_bound);
    }
  }

  /// Method to set boundaries for respawning vehicles.
  void SetMaxBoundaries(const float lower, const float upper) {
    TrafficManagerBase* tm_ptr = GetTM(_port);
    if (tm_ptr != nullptr) {
      tm_ptr->SetMaxBoundaries(lower, upper);
    }
  }

  /// This method sets the hybrid physics mode.
  void SetHybridPhysicsMode(const bool mode_switch) {
    TrafficManagerBase* tm_ptr = GetTM(_port);
    if(tm_ptr != nullptr){
      tm_ptr->SetHybridPhysicsMode(mode_switch);
    }
  }

  /// This method sets the hybrid physics radius.
  void SetHybridPhysicsRadius(const float radius) {
    TrafficManagerBase* tm_ptr = GetTM(_port);
    if(tm_ptr != nullptr){
      tm_ptr->SetHybridPhysicsRadius(radius);
    }
  }

  /// This method registers a vehicle with the traffic manager.
  void RegisterVehicles(const std::vector<ActorPtr> &actor_list) {
    TrafficManagerBase* tm_ptr = GetTM(_port);
    if(tm_ptr != nullptr){
      tm_ptr->RegisterVehicles(actor_list);
    }
  }

  /// This method unregisters a vehicle from traffic manager.
  void UnregisterVehicles(const std::vector<ActorPtr> &actor_list) {
    TrafficManagerBase* tm_ptr = GetTM(_port);
    if(tm_ptr != nullptr){
      tm_ptr->UnregisterVehicles(actor_list);
    }
  }

  /// Set a vehicle's % decrease in velocity with respect to the speed limit.
  /// If less than 0, it's a % increase.
  void SetPercentageSpeedDifference(const ActorPtr &actor, const float percentage) {
    TrafficManagerBase* tm_ptr = GetTM(_port);
    if(tm_ptr != nullptr){
      tm_ptr->SetPercentageSpeedDifference(actor, percentage);
    }
  }

  /// Method to set a lane offset displacement from the center line.
  /// Positive values imply a right offset while negative ones mean a left one.
  void SetLaneOffset(const ActorPtr &actor, const float offset) {
    TrafficManagerBase* tm_ptr = GetTM(_port);
    if(tm_ptr != nullptr){
      tm_ptr->SetLaneOffset(actor, offset);
    }
  }

  /// Set a vehicle's exact desired velocity.
  void SetDesiredSpeed(const ActorPtr &actor, const float value) {
    TrafficManagerBase* tm_ptr = GetTM(_port);
    if(tm_ptr != nullptr){
      tm_ptr->SetDesiredSpeed(actor, value);
    }
  }

  /// Set a global % decrease in velocity with respect to the speed limit.
  /// If less than 0, it's a % increase.
  void SetGlobalPercentageSpeedDifference(float const percentage){
    TrafficManagerBase* tm_ptr = GetTM(_port);
    if(tm_ptr != nullptr){
      tm_ptr->SetGlobalPercentageSpeedDifference(percentage);
    }
  }

  /// Method to set a global lane offset displacement from the center line.
  /// Positive values imply a right offset while negative ones mean a left one.
  void SetGlobalLaneOffset(float const offset){
    TrafficManagerBase* tm_ptr = GetTM(_port);
    if(tm_ptr != nullptr){
      tm_ptr->SetGlobalLaneOffset(offset);
    }
  }

  /// Set the automatic management of the vehicle lights
  void SetUpdateVehicleLights(const ActorPtr &actor, const bool do_update){
    TrafficManagerBase* tm_ptr = GetTM(_port);
    if(tm_ptr != nullptr){
      tm_ptr->SetUpdateVehicleLights(actor, do_update);
    }
  }

  /// Method to set collision detection rules between vehicles.
  void SetCollisionDetection(const ActorPtr &reference_actor, const ActorPtr &other_actor, const bool detect_collision) {
    TrafficManagerBase* tm_ptr = GetTM(_port);
    if(tm_ptr != nullptr){
      tm_ptr->SetCollisionDetection(reference_actor, other_actor, detect_collision);
    }
  }

  /// Method to force lane change on a vehicle.
  /// Direction flag can be set to true for left and false for right.
  void SetForceLaneChange(const ActorPtr &actor, const bool direction) {
    TrafficManagerBase* tm_ptr = GetTM(_port);
    if(tm_ptr != nullptr){
      tm_ptr->SetForceLaneChange(actor, direction);
    }
  }

  /// Enable/disable automatic lane change on a vehicle.
  void SetAutoLaneChange(const ActorPtr &actor, const bool enable) {
    TrafficManagerBase* tm_ptr = GetTM(_port);
    if(tm_ptr != nullptr){
      tm_ptr->SetAutoLaneChange(actor, enable);
    }
  }

  /// Method to specify how much distance a vehicle should maintain to
  /// the leading vehicle.
  void SetDistanceToLeadingVehicle(const ActorPtr &actor, const float distance) {
    TrafficManagerBase* tm_ptr = GetTM(_port);
    if(tm_ptr != nullptr){
      tm_ptr->SetDistanceToLeadingVehicle(actor, distance);
    }
  }

  /// Method to specify the % chance of ignoring collisions with any walker.
  void SetPercentageIgnoreWalkers(const ActorPtr &actor, const float perc) {
    TrafficManagerBase* tm_ptr = GetTM(_port);
    if(tm_ptr != nullptr){
      tm_ptr->SetPercentageIgnoreWalkers(actor, perc);
    }
  }

  /// Method to specify the % chance of ignoring collisions with any vehicle.
  void SetPercentageIgnoreVehicles(const ActorPtr &actor, const float perc) {
    TrafficManagerBase* tm_ptr = GetTM(_port);
    if(tm_ptr != nullptr){
      tm_ptr->SetPercentageIgnoreVehicles(actor, perc);
    }
  }

  /// Method to specify the % chance of running a sign.
  void SetPercentageRunningSign(const ActorPtr &actor, const float perc) {
    TrafficManagerBase* tm_ptr = GetTM(_port);
    if(tm_ptr != nullptr){
      tm_ptr->SetPercentageRunningSign(actor, perc);
    }
  }

  /// Method to specify the % chance of running a light.
  void SetPercentageRunningLight(const ActorPtr &actor, const float perc){
    TrafficManagerBase* tm_ptr = GetTM(_port);
    if(tm_ptr != nullptr){
      tm_ptr->SetPercentageRunningLight(actor, perc);
    }
  }

  /// Method to switch traffic manager into synchronous execution.
  void SetSynchronousMode(bool mode) {
    TrafficManagerBase* tm_ptr = GetTM(_port);
    if(tm_ptr != nullptr){
      tm_ptr->SetSynchronousMode(mode);
    }
  }

  /// Method to set tick timeout for synchronous execution.
  void SetSynchronousModeTimeOutInMiliSecond(double time) {
    TrafficManagerBase* tm_ptr = GetTM(_port);
    if(tm_ptr != nullptr){
      tm_ptr->SetSynchronousModeTimeOutInMiliSecond(time);
    }
  }

  /// Method to provide synchronous tick.
  bool SynchronousTick() {
    TrafficManagerBase* tm_ptr = GetTM(_port);
    if(tm_ptr != nullptr){
      return tm_ptr->SynchronousTick();
    }
    return false;
  }

  /// Method to Set Global distance to Leading vehicle
  void SetGlobalDistanceToLeadingVehicle(const float distance) {
    TrafficManagerBase* tm_ptr = GetTM(_port);
    if(tm_ptr != nullptr){
      tm_ptr->SetGlobalDistanceToLeadingVehicle(distance);
    }
  }

  /// Method to set % to keep on the right lane.
  void SetKeepRightPercentage(const ActorPtr &actor, const float percentage) {
    TrafficManagerBase* tm_ptr = GetTM(_port);
    if(tm_ptr != nullptr){
      tm_ptr->SetKeepRightPercentage(actor, percentage);
    }
  }

  /// Method to set % to randomly do a left lane change.
  void SetRandomLeftLaneChangePercentage(const ActorPtr &actor, const float percentage) {
    TrafficManagerBase* tm_ptr = GetTM(_port);
    if(tm_ptr != nullptr){
      tm_ptr->SetRandomLeftLaneChangePercentage(actor, percentage);
    }
  }

  /// Method to set % to randomly do a right lane change.
  void SetRandomRightLaneChangePercentage(const ActorPtr &actor, const float percentage) {
    TrafficManagerBase* tm_ptr = GetTM(_port);
    if(tm_ptr != nullptr){
      tm_ptr->SetRandomRightLaneChangePercentage(actor, percentage);
    }
  }

  /// Method to set randomization seed.
  void SetRandomDeviceSeed(const uint64_t seed) {
    TrafficManagerBase* tm_ptr = GetTM(_port);
    if(tm_ptr != nullptr){
      tm_ptr->SetRandomDeviceSeed(seed);
    }
  }

  void ShutDown();

  /// Method to get the next action.
  Action GetNextAction(const ActorId &actor_id) {
    Action next_action;
    TrafficManagerBase* tm_ptr = GetTM(_port);
    if (tm_ptr != nullptr) {
      next_action = tm_ptr->GetNextAction(actor_id);
      return next_action;
    }
    return next_action;
  }

  /// Method to get the action buffer.
  ActionBuffer GetActionBuffer(const ActorId &actor_id) {
    ActionBuffer action_buffer;
    TrafficManagerBase* tm_ptr = GetTM(_port);
    if (tm_ptr != nullptr) {
      action_buffer = tm_ptr->GetActionBuffer(actor_id);
      return action_buffer;
    }
    return action_buffer;
  }

private:

  void CreateTrafficManagerServer(
    carla::client::detail::EpisodeProxy episode_proxy,
    uint16_t port);


  bool CreateTrafficManagerClient(
    carla::client::detail::EpisodeProxy episode_proxy,
    uint16_t port);

  TrafficManagerBase* GetTM(uint16_t port) const {
    std::lock_guard<std::mutex> lock(_mutex);
    auto it = _tm_map.find(port);
    if (it != _tm_map.end()) {
      return it->second;
    }
    return nullptr;
  }

  static std::map<uint16_t, TrafficManagerBase*> _tm_map;
  static std::mutex _mutex;

  uint16_t _port = 0;

};

} // namespace traffic_manager
} // namespace carla
