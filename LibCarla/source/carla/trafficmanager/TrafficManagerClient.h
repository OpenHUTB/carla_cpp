// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/trafficmanager/Constants.h"// 引入常量定义
#include "carla/rpc/Actor.h"// 引入Actor类的定义

#include <rpc/client.h>// 引入RPC客户端库

namespace carla {
namespace traffic_manager {
    /**
     * 使用常量命名空间中的TM_TIMEOUT和TM_DEFAULT_PORT。
     */
using constants::Networking::TM_TIMEOUT;
using constants::Networking::TM_DEFAULT_PORT;

/**
 * @class TrafficManagerClient
 * @brief 提供与TrafficManagerServer的RPC通信功能。
 */
class TrafficManagerClient {

public:
    /**
       * @brief 拷贝构造函数，默认实现。
       */
  TrafficManagerClient(const TrafficManagerClient &) = default;
  /**
   * @brief 移动构造函数，默认实现。
   */
  TrafficManagerClient(TrafficManagerClient &&) = default;
  /**
   * @brief 拷贝赋值运算符，默认实现。
   */
  TrafficManagerClient &operator=(const TrafficManagerClient &) = default;
  /**
   * @brief 移动赋值运算符，默认实现。
   */
  TrafficManagerClient &operator=(TrafficManagerClient &&) = default;

  /**
   * @brief 参数化构造函数，用于初始化连接参数。
   * @param _host 服务器主机名或IP地址。
   * @param _port 服务器端口号。
   */
  TrafficManagerClient(
      const std::string &_host,// 服务器主机名或IP地址 
      const uint16_t &_port)// 服务器端口号  
    : tmhost(_host),// 初始化成员变量tmhost 
      tmport(_port) {// 初始化成员变量tmport

      /**
       * 创建RPC客户端实例。
       * 如果_client尚未创建，则创建一个新的rpc::client实例，并设置超时时间。
       */
      if(!_client) {
        _client = new ::rpc::client(tmhost, tmport);// 创建RPC客户端实例
        _client->set_timeout(TM_TIMEOUT);// 设置超时时间为TM_TIMEOUT
      }
  }

  /// 析构函数，用于清理资源
  ~TrafficManagerClient() {
    if(_client) {
      delete _client;// 如果_client存在，则删除它
      _client = nullptr;// 将_client指针置为空，避免野指针  
    }
  };

  /// 设置服务器详细信息。  
/// @param _host 服务器主机名或IP地址。  
/// @param _port 服务器端口号
  void setServerDetails(const std::string &_host, const uint16_t &_port) {
    tmhost = _host;// 设置主机名
    tmport = _port;// 设置端口号 
  }

  /// 获取服务器详细信息。  
/// @param _host 用于存储服务器主机名的字符串引用。  
/// @param _port 用于存储服务器端口号的无符号短整型引用
  void getServerDetails(std::string &_host, uint16_t &_port) {
    _host = tmhost;// 返回主机名
    _port = tmport;// 返回端口号 
  }

  /// 通过RPC客户端向远程交通管理服务器注册车辆。  
/// @param actor_list 要注册的车辆列表。
  void RegisterVehicle(const std::vector<carla::rpc::Actor> &actor_list) {
    DEBUG_ASSERT(_client != nullptr);// 断言_client不为空 
    _client->call("register_vehicle", std::move(actor_list));// 调用RPC方法注册车辆  
  }

  /// 通过RPC客户端从远程交通管理服务器注销车辆。  
/// @param actor_list 要注销的车辆列表。 
  void UnregisterVehicle(const std::vector<carla::rpc::Actor> &actor_list) {
    DEBUG_ASSERT(_client != nullptr);// 断言_client不为空
    _client->call("unregister_vehicle", std::move(actor_list));// 调用RPC方法注销车辆
  }

  /// 设置车辆相对于限速的速度百分比差异。  
/// 如果小于0，则表示百分比增加。  
/// @param _actor 要设置速度差异的车辆。  
/// @param percentage 速度百分比差异。
  void SetPercentageSpeedDifference(const carla::rpc::Actor &_actor, const float percentage) {
    DEBUG_ASSERT(_client != nullptr);// 断言_client不为空
    _client->call("set_percentage_speed_difference", std::move(_actor), percentage);// 调用RPC方法设置速度差异
  }

  /// 设置车辆相对于中心线的车道偏移量。  
/// 正值表示向右偏移，负值表示向左偏移。  
/// @param _actor 要设置车道偏移量的车辆。  
/// @param offset 车道偏移量。
  void SetLaneOffset(const carla::rpc::Actor &_actor, const float offset) {
    DEBUG_ASSERT(_client != nullptr);// 断言_client不为空
    _client->call("set_lane_offset", std::move(_actor), offset);// 调用RPC方法设置车道偏移量
  }

  /// 设置车辆的精确期望速度。  
/// @param _actor 要设置期望速度的车辆。  
/// @param value 期望速度值。
  void SetDesiredSpeed(const carla::rpc::Actor &_actor, const float value) {
    DEBUG_ASSERT(_client != nullptr);// 断言_client不为空
    _client->call("set_desired_speed", std::move(_actor), value);// 调用RPC方法设置期望速度
  }

  /// 设置全局相对于限速的速度百分比差异。  
/// 如果小于0，则表示百分比增加。  
/// @param percentage 全局速度百分比差异。
  void SetGlobalPercentageSpeedDifference(const float percentage) {
    DEBUG_ASSERT(_client != nullptr);// 断言_client不为空
    _client->call("set_global_percentage_speed_difference", percentage);// 调用RPC方法设置全局速度差异
  }

  /// 设置全局车道偏移量，相对于中心线。  
/// 正值表示向右偏移，负值表示向左偏移。  
/// @param offset 车道偏移量。
  void SetGlobalLaneOffset(const float offset) {
    DEBUG_ASSERT(_client != nullptr);// 断言_client指针不为空
    _client->call("set_global_lane_offset", offset);// 调用RPC方法设置全局车道偏移量  
  }

  /// 设置车辆灯光的自动管理。  
/// @param _actor 要设置灯光管理的车辆。  
/// @param do_update 是否启用灯光自动管理。
  void SetUpdateVehicleLights(const carla::rpc::Actor &_actor, const bool do_update) {
    DEBUG_ASSERT(_client != nullptr);// 断言_client指针不为空
    _client->call("update_vehicle_lights", std::move(_actor), do_update);// 调用RPC方法设置车辆灯光自动管理
  }

  /// 设置车辆间的碰撞检测规则。  
/// @param reference_actor 参考车辆。  
/// @param other_actor 另一辆参与碰撞检测的车辆。  
/// @param detect_collision 是否启用碰撞检测。
  void SetCollisionDetection(const carla::rpc::Actor &reference_actor, const carla::rpc::Actor &other_actor, const bool detect_collision) {
    DEBUG_ASSERT(_client != nullptr);// 断言_client指针不为空  
    _client->call("set_collision_detection", reference_actor, other_actor, detect_collision);// 调用RPC方法设置碰撞检测规则 
  }

  /// 强制车辆换道。  
/// @param actor 要强制换道的车辆。  
/// @param direction 换道方向，true表示向左，false表示向右。
  void SetForceLaneChange(const carla::rpc::Actor &actor, const bool direction) {
    DEBUG_ASSERT(_client != nullptr);// 断言_client指针不为空
    _client->call("set_force_lane_change", actor, direction);// 调用RPC方法强制车辆换道
  }

  /// 启用/禁用车辆的自动换道功能。  
/// @param actor 要设置自动换道功能的车辆。  
/// @param enable 是否启用自动换道。
  void SetAutoLaneChange(const carla::rpc::Actor &actor, const bool enable) {
    DEBUG_ASSERT(_client != nullptr);// 断言_client指针不为空 
    _client->call("set_auto_lane_change", actor, enable);// 调用RPC方法设置自动换道功能
  }

  /// 设置车辆与前车应保持的距离。  
/// @param actor 要设置距离的车辆。  
/// @param distance 应保持的距离。
  void SetDistanceToLeadingVehicle(const carla::rpc::Actor &actor, const float distance) {
    DEBUG_ASSERT(_client != nullptr);// 断言_client指针不为空
    _client->call("set_distance_to_leading_vehicle", actor, distance);// 调用RPC方法设置与前车的距离
  }

  /// 设置车辆忽略行人的碰撞概率。  
/// @param actor 要设置碰撞概率的车辆。  
/// @param percentage 忽略行人的碰撞概率（百分比）。
  void SetPercentageIgnoreWalkers(const carla::rpc::Actor &actor, const float percentage) {
    DEBUG_ASSERT(_client != nullptr);// 断言_client指针不为空
    _client->call("set_percentage_ignore_walkers", actor, percentage); // 调用RPC方法设置忽略行人的碰撞概率
  }

  /// 设置车辆忽略其他车辆的碰撞概率。  
/// @param actor 要设置碰撞概率的车辆。  
/// @param percentage 忽略其他车辆的碰撞概率（百分比）。
  void SetPercentageIgnoreVehicles(const carla::rpc::Actor &actor, const float percentage) {
    DEBUG_ASSERT(_client != nullptr);// 断言_client指针不为空
    _client->call("set_percentage_ignore_vehicles", actor, percentage);// 调用RPC方法设置忽略其他车辆的碰撞概率
  }

  /// 设置车辆闯红灯的概率。  
/// @param actor 要设置闯红灯概率的车辆。  
/// @param percentage 闯红灯的概率（百分比）。
  void SetPercentageRunningLight(const carla::rpc::Actor &actor, const float percentage) {
    DEBUG_ASSERT(_client != nullptr);// 断言_client指针不为空
    _client->call("set_percentage_running_light", actor, percentage);// 调用RPC方法设置闯红灯的概率
  }

  /// 设置车辆无视任何交通标志的概率。  
/// @param actor 要设置无视交通标志概率的车辆。  
/// @param percentage 无视交通标志的概率（百分比）。 
  void SetPercentageRunningSign(const carla::rpc::Actor &actor, const float percentage) {
    DEBUG_ASSERT(_client != nullptr);// 断言_client指针不为空
    _client->call("set_percentage_running_sign", actor, percentage);// 调用RPC方法设置无视交通标志的概率
  }

  /// 将交通管理器切换为同步执行模式。  
/// @param mode 是否启用同步模式。
  void SetSynchronousMode(const bool mode) {
    DEBUG_ASSERT(_client != nullptr);// 断言_client指针不为空 
    _client->call("set_synchronous_mode", mode);// 调用RPC方法设置同步执行模式
  }

  /// Method to set tick timeout for synchronous execution.
  void SetSynchronousModeTimeOutInMiliSecond(const double time) {
    DEBUG_ASSERT(_client != nullptr);
    _client->call("set_synchronous_mode_timeout_in_milisecond", time);
  }

  /// Method to provide synchronous tick.
  bool SynchronousTick() {
    DEBUG_ASSERT(_client != nullptr);
    return _client->call("synchronous_tick").as<bool>();
  }

  /// Check if remote traffic manager is alive
  void HealthCheckRemoteTM() {
    DEBUG_ASSERT(_client != nullptr);
    _client->call("health_check_remote_TM");
  }

  /// Method to specify how much distance a vehicle should maintain to
  /// the Global leading vehicle.
  void SetGlobalDistanceToLeadingVehicle(const float distance) {
    DEBUG_ASSERT(_client != nullptr);
    _client->call("set_global_distance_to_leading_vehicle",distance);
  }

  /// Method to set % to keep on the right lane.
  void SetKeepRightPercentage(const carla::rpc::Actor &actor, const float percentage) {
    DEBUG_ASSERT(_client != nullptr);
    _client->call("keep_right_rule_percentage", actor, percentage);
  }

  /// Method to set % to randomly do a left lane change.
  void SetRandomLeftLaneChangePercentage(const carla::rpc::Actor &actor, const float percentage) {
    DEBUG_ASSERT(_client != nullptr);
    _client->call("random_left_lanechange_percentage", actor, percentage);
  }

  /// Method to set % to randomly do a right lane change.
  void SetRandomRightLaneChangePercentage(const carla::rpc::Actor &actor, const float percentage) {
    DEBUG_ASSERT(_client != nullptr);
    _client->call("random_right_lanechange_percentage", actor, percentage);
  }

  /// Method to set hybrid physics mode.
  void SetHybridPhysicsMode(const bool mode_switch) {
    DEBUG_ASSERT(_client != nullptr);
    _client->call("set_hybrid_physics_mode", mode_switch);
  }

  /// Method to set hybrid physics mode.
  void SetHybridPhysicsRadius(const float radius) {
    DEBUG_ASSERT(_client != nullptr);
    _client->call("set_hybrid_physics_radius", radius);
  }

  /// Method to set randomization seed.
  void SetRandomDeviceSeed(const uint64_t seed) {
    DEBUG_ASSERT(_client != nullptr);
    _client->call("set_random_device_seed", seed);
  }

  /// Method to set Open Street Map mode.
  void SetOSMMode(const bool mode_switch) {
    DEBUG_ASSERT(_client != nullptr);
    _client->call("set_osm_mode", mode_switch);
  }

  /// Method to set our own imported path.
  void SetCustomPath(const carla::rpc::Actor &actor, const Path path, const bool empty_buffer) {
    DEBUG_ASSERT(_client != nullptr);
    _client->call("set_path", actor, path, empty_buffer);
  }

  /// Method to remove a list of points.
  void RemoveUploadPath(const ActorId &actor_id, const bool remove_path) {
    DEBUG_ASSERT(_client != nullptr);
    _client->call("remove_custom_path", actor_id, remove_path);
  }

  /// Method to update an already set list of points.
  void UpdateUploadPath(const ActorId &actor_id, const Path path) {
    DEBUG_ASSERT(_client != nullptr);
    _client->call("update_custom_path", actor_id, path);
  }

  /// Method to set our own imported route.
  void SetImportedRoute(const carla::rpc::Actor &actor, const Route route, const bool empty_buffer) {
    DEBUG_ASSERT(_client != nullptr);
    _client->call("set_imported_route", actor, route, empty_buffer);
  }

  /// Method to remove a route.
  void RemoveImportedRoute(const ActorId &actor_id, const bool remove_path) {
    DEBUG_ASSERT(_client != nullptr);
    _client->call("remove_imported_route", actor_id, remove_path);
  }

  /// Method to update an already set list of points.
  void UpdateImportedRoute(const ActorId &actor_id, const Route route) {
    DEBUG_ASSERT(_client != nullptr);
    _client->call("update_imported_route", actor_id, route);
  }

  /// Method to set automatic respawn of dormant vehicles.
  void SetRespawnDormantVehicles(const bool mode_switch) {
    DEBUG_ASSERT(_client != nullptr);
    _client->call("set_respawn_dormant_vehicles", mode_switch);
  }

  /// Method to set boundaries for respawning vehicles.
  void SetBoundariesRespawnDormantVehicles(const float lower_bound, const float upper_bound) {
    DEBUG_ASSERT(_client != nullptr);
    _client->call("set_boundaries_respawn_dormant_vehicles", lower_bound, upper_bound);
  }

  /// Method to set boundaries for respawning vehicles.
  void SetMaxBoundaries(const float lower, const float upper) {
    DEBUG_ASSERT(_client != nullptr);
    _client->call("set_max_boundaries", lower, upper);
  }

  /// Method to get the vehicle's next action.
  Action GetNextAction(const ActorId &actor_id) {
    DEBUG_ASSERT(_client != nullptr);
    _client->call("get_next_action", actor_id);
    return Action();
  }

  /// Method to get the vehicle's action buffer.
  ActionBuffer GetActionBuffer(const ActorId &actor_id) {
    DEBUG_ASSERT(_client != nullptr);
    _client->call("get_all_actions", actor_id);
    return ActionBuffer();
  }

  void ShutDown() {
    DEBUG_ASSERT(_client != nullptr);
    _client->call("shut_down");
  }

private:

  /// RPC client.
  ::rpc::client *_client = nullptr;

  /// Server port and host.
  std::string tmhost;
  uint16_t    tmport;
};

} // namespace traffic_manager
} // namespace carla
