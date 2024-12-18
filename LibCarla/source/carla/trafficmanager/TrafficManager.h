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
  /// \brief 获取当前端口号。  
/// \return 返回当前设置的端口号
  uint16_t Port() const {
    return _port;
  }
  /// \brief 检查端口号是否有效。  
/// \details 通常，操作系统会保留前1024个端口，因此有效端口号应大于1023。  
/// \return 如果端口号有效，则返回true；否则返回false。
  bool IsValidPort() const {
    return (_port > 1023);
  }

  /// \brief 设置Open Street Map模式。  
  /// \param mode_switch 如果为true，则启用OSM模式；如果为false，则禁用
  void SetOSMMode(const bool mode_switch) {
    TrafficManagerBase* tm_ptr = GetTM(_port);
    if (tm_ptr != nullptr) {
      tm_ptr->SetOSMMode(mode_switch);
    }
  }

  /// \brief 设置自定义路径。  
/// \param actor 对应的Actor指针。  
/// \param path 要设置的路径。  
/// \param empty_buffer 如果为true，则在设置新路径前清空缓冲区
  void SetCustomPath(const ActorPtr &actor, const Path path, const bool empty_buffer) {
    TrafficManagerBase* tm_ptr = GetTM(_port);
    if (tm_ptr != nullptr) {
      tm_ptr->SetCustomPath(actor, path, empty_buffer);
    }
  }

  /// \brief 移除路径。  
 /// \param actor_id 要移除路径的Actor的ID。  
 /// \param remove_path 如果为true，则移除路径；如果为false，则不执行操作（具体行为可能依赖于实现）
  void RemoveUploadPath(const ActorId &actor_id, const bool remove_path) {
    TrafficManagerBase* tm_ptr = GetTM(_port);
    if (tm_ptr != nullptr) {
      tm_ptr->RemoveUploadPath(actor_id, remove_path);
    }
  }

  /// \brief 更新已设置的路径。  
/// \param actor_id 要更新路径的Actor的ID。  
/// \param path 新的路径。
  void UpdateUploadPath(const ActorId &actor_id, const Path path) {
    TrafficManagerBase* tm_ptr = GetTM(_port);
    if (tm_ptr != nullptr) {
      tm_ptr->UpdateUploadPath(actor_id, path);
    }
  }

  /// \brief 设置导入的路线。  
 /// \param actor 对应的Actor指针。  
 /// \param route 要设置的路线。  
 /// \param empty_buffer 如果为true，则在设置新路线前清空缓冲区。
  void SetImportedRoute(const ActorPtr &actor, const Route route, const bool empty_buffer) {
    TrafficManagerBase* tm_ptr = GetTM(_port);
    if (tm_ptr != nullptr) {
      tm_ptr->SetImportedRoute(actor, route, empty_buffer);
    }
  }

  /// \brief 移除路线。  
/// \param actor_id 要移除路线的Actor的ID。  
/// \param remove_path 如果为true，则移除路线；如果为false，则不执行操作（具体行为可能依赖于实现）
  void RemoveImportedRoute(const ActorId &actor_id, const bool remove_path) {
    TrafficManagerBase* tm_ptr = GetTM(_port);
    if (tm_ptr != nullptr) {
      tm_ptr->RemoveImportedRoute(actor_id, remove_path);
    }
  }

  /// \brief 更新已设置的路线。  
/// \param actor_id 要更新路线的Actor的ID。  
/// \param route 新的路线。
  void UpdateImportedRoute(const ActorId &actor_id, const Route route) {
    TrafficManagerBase* tm_ptr = GetTM(_port);
    if (tm_ptr != nullptr) {
      tm_ptr->UpdateImportedRoute(actor_id, route);
    }
  }

  /// \brief 设置是否自动重生车辆。  
/// \param mode_switch 如果为true，则启用自动重生；如果为false，则禁用。
  void SetRespawnDormantVehicles(const bool mode_switch) {
    TrafficManagerBase* tm_ptr = GetTM(_port);
    if (tm_ptr != nullptr) {
      tm_ptr->SetRespawnDormantVehicles(mode_switch);
    }
  }
  /// \brief 设置重生车辆的范围边界。  
  /// \param lower_bound 范围的下界。  
  /// \param upper_bound 范围的上界。 
  void SetBoundariesRespawnDormantVehicles(const float lower_bound, const float upper_bound) {
    TrafficManagerBase* tm_ptr = GetTM(_port);
    if (tm_ptr != nullptr) {
      tm_ptr->SetBoundariesRespawnDormantVehicles(lower_bound, upper_bound);
    }
  }

  /// @brief 设置重生车辆的边界。   
/// 此方法用于为TrafficManager设置车辆重生时的上下边界。    
/// @param lower 下边界值。  
/// @param upper 上边界值。 
  void SetMaxBoundaries(const float lower, const float upper) {
    TrafficManagerBase* tm_ptr = GetTM(_port);
    if (tm_ptr != nullptr) {
      tm_ptr->SetMaxBoundaries(lower, upper);
    }
  }

  /// @brief 设置混合物理模式。  
 /// 此方法用于启用或禁用TrafficManager的混合物理模式。  
 /// @param mode_switch 启用（true）或禁用（false）混合物理模式。  
  void SetHybridPhysicsMode(const bool mode_switch) {
    TrafficManagerBase* tm_ptr = GetTM(_port);
    if(tm_ptr != nullptr){
      tm_ptr->SetHybridPhysicsMode(mode_switch);
    }
  }

  /// @brief 设置混合物理半径。  
/// 此方法用于设置TrafficManager的混合物理模式的触发半径。     
/// @param radius 混合物理模式的触发半径。
  void SetHybridPhysicsRadius(const float radius) {
    TrafficManagerBase* tm_ptr = GetTM(_port);
    if(tm_ptr != nullptr){
      tm_ptr->SetHybridPhysicsRadius(radius);
    }
  }

  /// @brief 向交通管理器注册车辆。  
/// 此方法用于将一组车辆注册到TrafficManager中。  
/// @param actor_list 要注册的车辆列表。
  void RegisterVehicles(const std::vector<ActorPtr> &actor_list) {
    TrafficManagerBase* tm_ptr = GetTM(_port);
    if(tm_ptr != nullptr){
      tm_ptr->RegisterVehicles(actor_list);
    }
  }

  /// @brief 从交通管理器注销车辆。  
 /// 此方法用于从TrafficManager中注销一组车辆。 
 /// @param actor_list 要注销的车辆列表。
  void UnregisterVehicles(const std::vector<ActorPtr> &actor_list) {
    TrafficManagerBase* tm_ptr = GetTM(_port);
    if(tm_ptr != nullptr){
      tm_ptr->UnregisterVehicles(actor_list);
    }
  }

  /// @brief 设置车辆相对于限速的速度百分比差异。  
/// 此方法用于设置车辆相对于道路限速的速度百分比差异。如果百分比小于0，则表示速度增加。    
/// @param actor 要设置速度差异的车辆。  
/// @param percentage 速度百分比差异。 
  void SetPercentageSpeedDifference(const ActorPtr &actor, const float percentage) {
    TrafficManagerBase* tm_ptr = GetTM(_port);
    if(tm_ptr != nullptr){
      tm_ptr->SetPercentageSpeedDifference(actor, percentage);
    }
  }

  /// @brief 设置车辆相对于车道中心线的偏移量。  
/// 此方法用于设置车辆相对于车道中心线的偏移量。正值表示向右偏移，负值表示向左偏移。
/// @param actor 要设置车道偏移的车辆。  
/// @param offset 车道偏移量。 
  void SetLaneOffset(const ActorPtr &actor, const float offset) {
    TrafficManagerBase* tm_ptr = GetTM(_port);
    if(tm_ptr != nullptr){
      tm_ptr->SetLaneOffset(actor, offset);
    }
  }

  /// @brief 设置车辆的期望速度。  
 /// 此方法用于设置车辆的精确期望速度。  
 /// @param actor 要设置期望速度的车辆。  
 /// @param value 车辆的期望速度。
  void SetDesiredSpeed(const ActorPtr &actor, const float value) {
    TrafficManagerBase* tm_ptr = GetTM(_port);
    if(tm_ptr != nullptr){
      tm_ptr->SetDesiredSpeed(actor, value);
    }
  }

  /// @brief 设置全局速度百分比差异。  
/// 此方法用于设置所有车辆相对于道路限速的全局速度百分比差异。如果百分比小于0，则表示速度增加。  
/// @param percentage 全局速度百分比差异。
  void SetGlobalPercentageSpeedDifference(float const percentage){
    TrafficManagerBase* tm_ptr = GetTM(_port);
    if(tm_ptr != nullptr){
      tm_ptr->SetGlobalPercentageSpeedDifference(percentage);
    }
  }

  /// 设置全局车道偏移量，相对于中心线的位移。  
/// 正值表示向右偏移，负值表示向左偏移。 
  void SetGlobalLaneOffset(float const offset){
    TrafficManagerBase* tm_ptr = GetTM(_port); // 获取交通管理器实例 
    if(tm_ptr != nullptr){// 检查实例是否有效 
      tm_ptr->SetGlobalLaneOffset(offset);// 调用设置全局车道偏移量的方法 
    }
  }

  /// 设置车辆灯光的自动管理
  void SetUpdateVehicleLights(const ActorPtr &actor, const bool do_update){
    TrafficManagerBase* tm_ptr = GetTM(_port);// 获取交通管理器实例 
    if(tm_ptr != nullptr){// 检查实例是否有效 
      tm_ptr->SetUpdateVehicleLights(actor, do_update);// 调用设置车辆灯光自动管理的方法 
    }
  }

  /// 设置车辆之间的碰撞检测规则
  void SetCollisionDetection(const ActorPtr &reference_actor, const ActorPtr &other_actor, const bool detect_collision) {
    TrafficManagerBase* tm_ptr = GetTM(_port);// 获取交通管理器实例 
    if(tm_ptr != nullptr){// 检查实例是否有效 
      tm_ptr->SetCollisionDetection(reference_actor, other_actor, detect_collision);// 调用设置碰撞检测规则的方法
    }
  }

  /// 强制车辆换道。  
/// 方向标志设置为true表示向左换道，false表示向右换道。
  void SetForceLaneChange(const ActorPtr &actor, const bool direction) {
    TrafficManagerBase* tm_ptr = GetTM(_port);// 获取交通管理器实例
    if(tm_ptr != nullptr){// 检查实例是否有效 
      tm_ptr->SetForceLaneChange(actor, direction);// 调用强制换道的方法
    }
  }

  /// 启用/禁用车辆的自动换道功能。
  void SetAutoLaneChange(const ActorPtr &actor, const bool enable) {
    TrafficManagerBase* tm_ptr = GetTM(_port);// 获取交通管理器实例 
    if(tm_ptr != nullptr){// 检查实例是否有效 
      tm_ptr->SetAutoLaneChange(actor, enable);// 调用设置自动换道功能的方法
    }
  }

  /// 设置车辆与前车应保持的距离。
  void SetDistanceToLeadingVehicle(const ActorPtr &actor, const float distance) {
    TrafficManagerBase* tm_ptr = GetTM(_port); // 获取交通管理器实例
    if(tm_ptr != nullptr){// 检查实例是否有效
      tm_ptr->SetDistanceToLeadingVehicle(actor, distance); // 调用设置与前车距离的方法
    }
  }

  /// 设置忽略行人的碰撞概率。  
/// @param actor 车辆指针。  
/// @param perc 忽略碰撞的概率（百分比）。
  void SetPercentageIgnoreWalkers(const ActorPtr &actor, const float perc) {
    TrafficManagerBase* tm_ptr = GetTM(_port);// 获取交通管理器实例 
    if(tm_ptr != nullptr){// 检查实例是否有效  
      tm_ptr->SetPercentageIgnoreWalkers(actor, perc); // 调用设置忽略行人碰撞概率的方法
    }
  }

  /// 设置忽略车辆的碰撞概率。  
/// @param actor 车辆指针。  
/// @param perc 忽略碰撞的概率（百分比）。
  void SetPercentageIgnoreVehicles(const ActorPtr &actor, const float perc) {
    TrafficManagerBase* tm_ptr = GetTM(_port);// 获取交通管理器实例 
    if(tm_ptr != nullptr){// 检查实例是否有效 
      tm_ptr->SetPercentageIgnoreVehicles(actor, perc);// 调用设置忽略车辆碰撞概率的方法
    }
  }

  /// 设置无视交通标志的概率。  
/// @param actor 车辆指针。  
/// @param perc 无视交通标志的概率（百分比）。 
  void SetPercentageRunningSign(const ActorPtr &actor, const float perc) {
    TrafficManagerBase* tm_ptr = GetTM(_port);// 获取交通管理器实例 
    if(tm_ptr != nullptr){// 检查实例是否有效 
      tm_ptr->SetPercentageRunningSign(actor, perc);// 调用设置无视交通标志概率的方法
    }
  }

  /// 设置无视交通信号灯的概率。  
/// @param actor 车辆指针。  
/// @param perc 无视交通信号灯的概率（百分比）。  
  void SetPercentageRunningLight(const ActorPtr &actor, const float perc){
    TrafficManagerBase* tm_ptr = GetTM(_port);// 获取交通管理器实例
    if(tm_ptr != nullptr){// 检查实例是否有效
      tm_ptr->SetPercentageRunningLight(actor, perc); // 调用设置无视交通信号灯概率的方法
    }
  }

  /// 将交通管理器切换为同步执行模式。  
 /// @param mode 是否启用同步模式（true为启用，false为禁用）。
  void SetSynchronousMode(bool mode) {// 获取交通管理器实例 
    TrafficManagerBase* tm_ptr = GetTM(_port);// 获取交通管理器实例
    if(tm_ptr != nullptr){// 检查实例是否有效
      tm_ptr->SetSynchronousMode(mode);// 调用设置同步执行模式的方法  
    }
  }

  /// 设置同步执行模式的滴答超时时间（以毫秒为单位）。  
/// @param time 超时时间，单位为毫秒。
  void SetSynchronousModeTimeOutInMiliSecond(double time) {
    TrafficManagerBase* tm_ptr = GetTM(_port);// 获取交通管理器实例 
    if(tm_ptr != nullptr){// 检查实例是否有效 
      tm_ptr->SetSynchronousModeTimeOutInMiliSecond(time);// 调用设置超时时间的方法
    }
  }

  /// 执行同步滴答。  
/// @return 如果成功执行同步滴答，则返回true；否则返回false。 
  bool SynchronousTick() {
    TrafficManagerBase* tm_ptr = GetTM(_port);// 获取交通管理器实例 
    if(tm_ptr != nullptr){// 检查实例是否有效 
      return tm_ptr->SynchronousTick();// 调用执行同步滴答的方法，并返回结果
    }
    return false;// 如果实例无效，则返回false
  }

  /// 设置全局的前车距离。  
/// @param distance 前车距离。
  void SetGlobalDistanceToLeadingVehicle(const float distance) {
    TrafficManagerBase* tm_ptr = GetTM(_port);// 获取交通管理器实例
    if(tm_ptr != nullptr){// 检查实例是否有效
      tm_ptr->SetGlobalDistanceToLeadingVehicle(distance);// 调用设置前车距离的方法
    }
  }

  /// 设置车辆保持在右侧车道的百分比。  
 /// @param actor 车辆指针。  
 /// @param percentage 保持在右侧车道的百分比。 
  void SetKeepRightPercentage(const ActorPtr &actor, const float percentage) {
    TrafficManagerBase* tm_ptr = GetTM(_port);// 获取交通管理器实例  
    if(tm_ptr != nullptr){// 检查实例是否有效
      tm_ptr->SetKeepRightPercentage(actor, percentage);// 调用设置保持在右侧车道百分比的方法
    }
  }

  /// 设置车辆随机进行左侧车道变换的百分比。  
/// @param actor 车辆指针。  
/// @param percentage 随机进行左侧车道变换的百分比。
  void SetRandomLeftLaneChangePercentage(const ActorPtr &actor, const float percentage) {
    TrafficManagerBase* tm_ptr = GetTM(_port);// 获取交通管理器实例 
    if(tm_ptr != nullptr){// 检查实例是否有效 
      tm_ptr->SetRandomLeftLaneChangePercentage(actor, percentage);// 调用设置随机左侧车道变换百分比的方法
    }
  }

  /// 设置车辆随机进行右侧车道变换的百分比。  
 /// @param actor 车辆指针。  
 /// @param percentage 随机进行右侧车道变换的百分比。
  void SetRandomRightLaneChangePercentage(const ActorPtr &actor, const float percentage) {
    TrafficManagerBase* tm_ptr = GetTM(_port);// 获取交通管理器实例  
    if(tm_ptr != nullptr){// 检查实例是否有效
      tm_ptr->SetRandomRightLaneChangePercentage(actor, percentage);// 调用设置随机右侧车道变换百分比的方法
    }
  }

  /// 设置随机数生成器的种子。  
 /// @param seed 随机数生成器的种子。
  void SetRandomDeviceSeed(const uint64_t seed) {
    TrafficManagerBase* tm_ptr = GetTM(_port);// 获取交通管理器实例 
    if(tm_ptr != nullptr){// 检查实例是否有效
      tm_ptr->SetRandomDeviceSeed(seed);// 调用设置随机数生成器种子的方法
    }
  }
  /// 关闭交通管理器服务或客户端。
  void ShutDown();

  /// 获取指定参与者的下一个动作。  
  /// @param actor_id 参与者的唯一标识符。  
  /// @return 指定参与者的下一个动作。
  Action GetNextAction(const ActorId &actor_id) {
    Action next_action;
    TrafficManagerBase* tm_ptr = GetTM(_port);
    if (tm_ptr != nullptr) {
      next_action = tm_ptr->GetNextAction(actor_id);
      return next_action;
    }
    return next_action;
  }

  /// 获取指定参与者的动作缓冲区。  
   /// @param actor_id 参与者的唯一标识符。  
   /// @return 指定参与者的动作缓冲区。 
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
    /// 创建交通管理器服务器。  
  /// @param episode_proxy 与当前场景相关联的代理对象。  
  /// @param port 服务器监听的端口号。
  void CreateTrafficManagerServer(
    carla::client::detail::EpisodeProxy episode_proxy,
    uint16_t port);

  /// 创建交通管理器客户端。  
  /// @param episode_proxy 与当前场景相关联的代理对象。  
  /// @param port 客户端连接的端口号。  
  /// @return 如果创建成功，则返回true；否则返回false。
  bool CreateTrafficManagerClient(
    carla::client::detail::EpisodeProxy episode_proxy,
    uint16_t port);
  /// 根据端口号获取交通管理器实例。  
  /// @param port 端口号。  
  /// @return 对应的交通管理器实例指针；如果未找到，则返回nullptr。
  TrafficManagerBase* GetTM(uint16_t port) const {
    std::lock_guard<std::mutex> lock(_mutex);
    auto it = _tm_map.find(port);
    if (it != _tm_map.end()) {
      return it->second;
    }
    return nullptr;
  }
  /// 存储交通管理器实例的静态映射表。  
  /// @details 键为端口号，值为对应的交通管理器实例指针
  static std::map<uint16_t, TrafficManagerBase*> _tm_map;
  /// 用于保护_tm_map的静态互斥锁。
  static std::mutex _mutex;
  /// 当前交通管理器实例使用的端口号。
  uint16_t _port = 0;

};

} // namespace traffic_manager
} // namespace carla
