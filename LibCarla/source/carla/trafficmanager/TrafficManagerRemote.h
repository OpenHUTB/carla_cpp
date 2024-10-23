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
 
  /**
   * @brief 构造函数，存储远程服务器的位置信息。
   *
   * @param _serverTM 远程交通管理器的地址和端口号。
   * @param episodeProxy 与当前仿真片段相关联的代理对象。
   */
  TrafficManagerRemote(const std::pair<std::string, uint16_t> &_serverTM, carla::client::detail::EpisodeProxy &episodeProxy);
  
  /**
   * @brief 析构函数。
   *
   * 释放TrafficManagerRemote对象占用的资源。
   */
  virtual ~TrafficManagerRemote();

  /**
   * @brief 注册车辆到交通管理器。
   *
   * @param actor_list 需要注册的车辆列表。
   */
  void RegisterVehicles(const std::vector<ActorPtr> &actor_list);

  /**
   * @brief 从交通管理器注销车辆。
   *
   * @param actor_list 需要注销的车辆列表。
   */
  void UnregisterVehicles(const std::vector<ActorPtr> &actor_list);

  /**
  * @brief 设置车辆相对于限速的速度差异百分比。
  *
  * @param actor 需要设置速度差异的车辆。
  * @param percentage 速度差异的百分比。如果小于0，则表示速度增加。
  */
  void SetPercentageSpeedDifference(const ActorPtr &actor, const float percentage);

  /**
  * @brief 设置车辆相对于车道中心线的偏移量。
  *
  * @param actor 需要设置车道偏移的车辆。
  * @param offset 车道偏移量。正值表示向右偏移，负值表示向左偏移。
  */
  void SetLaneOffset(const ActorPtr &actor, const float offset);

  /**
	* @brief 设置车辆的精确期望速度。
	*
	* @param actor 需要设置期望速度的车辆。
	* @param value 车辆的期望速度。
	*/
  void SetDesiredSpeed(const ActorPtr &actor, const float value);

  /**
   * @brief 设置全局速度相对于限速的百分比减少量。
   *
   * 如果传入的值小于0，则表示速度百分比增加。
   *
   * @param percentage 速度变化的百分比值。
   */
  void SetGlobalPercentageSpeedDifference(float const percentage);

  /**
 * @brief 设置全局车道偏移量，相对于车道中心线。
 *
 * 正值表示向右偏移，负值表示向左偏移。
 *
 * @param offset 车道偏移量的具体值。
 */
  void SetGlobalLaneOffset(float const offset);

  /**
 * @brief 设置车辆灯光的自动管理功能。
 *
 * @param actor 需要设置灯光自动管理的车辆对象。
 * @param do_update 是否启用灯光自动管理功能。
 */
  void SetUpdateVehicleLights(const ActorPtr &actor, const bool do_update);

  /**
   * @brief 设置车辆之间的碰撞检测规则。
   *
   * @param reference_actor 参考车辆对象，即碰撞检测的一方。
   * @param other_actor 另一车辆对象，即与参考车辆进行碰撞检测的对象。
   * @param detect_collision 是否启用碰撞检测功能。
   */
  void SetCollisionDetection(const ActorPtr &reference_actor, const ActorPtr &other_actor, const bool detect_collision);

  /**
 * @brief 强制车辆进行换道。
 *
 * 方向标志设置为true表示向左换道，设置为false表示向右换道。
 *
 * @param actor 需要进行换道的车辆对象。
 * @param direction 换道方向标志。
 */
  void SetForceLaneChange(const ActorPtr &actor, const bool direction);

  /**
 * @brief 启用/禁用车辆的自动换道功能。
 *
 * @param actor 需要设置自动换道功能的车辆对象。
 * @param enable 是否启用自动换道功能。
 */
  void SetAutoLaneChange(const ActorPtr &actor, const bool enable);

  /**
 * @brief 设置车辆与前车应保持的距离。
 *
 * @param actor 需要设置距离的车辆对象。
 * @param distance 车辆与前车应保持的具体距离值。
 */
  void SetDistanceToLeadingVehicle(const ActorPtr &actor, const float distance);

  /**
 * @brief 设置全局前车距离。
 *
 * @param distance 全局前车距离的具体值。
 */
  void SetGlobalDistanceToLeadingVehicle(const float distance);

  /**
 * @brief 设置车辆忽略行人碰撞的百分比概率。
 *
 * @param actor 需要设置碰撞忽略概率的车辆对象。
 * @param perc 忽略行人碰撞的百分比概率值。
 */
  void SetPercentageIgnoreWalkers(const ActorPtr &actor, const float perc);

  /**
 * @brief 设置车辆忽略其他车辆碰撞的百分比概率。
 *
 * @param actor 需要设置碰撞忽略概率的车辆对象。
 * @param perc 忽略其他车辆碰撞的百分比概率值。
 */
  void SetPercentageIgnoreVehicles(const ActorPtr &actor, const float perc);

  /**
 * @brief 设置车辆无视交通信号灯的概率百分比。
 *
 * @param actor 需要设置无视信号灯概率的车辆对象。
 * @param perc 无视交通信号灯的概率百分比值。
 */
  void SetPercentageRunningLight(const ActorPtr &actor, const float perc);

  /**
  * @brief 设置车辆无视交通标志的概率百分比。
  *
  * @param actor 需要设置无视交通标志概率的车辆对象。
  * @param perc 无视交通标志的概率百分比值。
  */
  void SetPercentageRunningSign(const ActorPtr &actor, const float perc);

  /**
 * @brief 切换交通管理器为同步执行模式。
 *
 * @param mode 是否启用同步执行模式。
 */
  void SetSynchronousMode(bool mode);

  /**
 * @brief 设置同步执行模式的Tick超时时间（毫秒）。
 *
 * @param time 同步执行模式的Tick超时时间值（毫秒）。
 */
  void SetSynchronousModeTimeOutInMiliSecond(double time);

  /**
  * @brief 设置车辆保持在右侧车道的百分比概率。
  *
  * @param actor 需要设置保持在右侧车道概率的车辆对象。
  * @param percentage 保持在右侧车道的百分比概率值。
  */
  void SetKeepRightPercentage(const ActorPtr &actor, const float percentage);

  /**
 * @brief 设置车辆随机进行左车道变换的百分比概率。
 *
 * @param actor 需要设置随机左车道变换概率的车辆对象。
 * @param percentage 随机进行左车道变换的百分比概率值。
 */
  void SetRandomLeftLaneChangePercentage(const ActorPtr &actor, const float percentage);

  /**
 * @brief 设置车辆随机进行右车道变换的百分比概率。
 *
 * @param actor 需要设置随机右车道变换概率的车辆对象。
 * @param percentage 随机进行右车道变换的百分比概率值。
 */
  void SetRandomRightLaneChangePercentage(const ActorPtr &actor, const float percentage);

  /**
 * @brief 设置混合物理模式。
 *
 * @param mode_switch 是否启用混合物理模式。
 */
  void SetHybridPhysicsMode(const bool mode_switch);

  /**
 * @brief 设置混合物理模式的半径。
 *
 * @param radius 混合物理模式的半径值。
 */
  void SetHybridPhysicsRadius(const float radius);

  /**
 * @brief 设置Open Street Map（OSM）模式。
 *
 * @param mode_switch 是否启用OSM模式。
 */
  void SetOSMMode(const bool mode_switch);

  /**
 * @brief 设置自定义路径。
 *
 * @param actor 需要设置自定义路径的车辆对象。
 * @param path 自定义路径对象。
 * @param empty_buffer 是否清空动作缓冲区。
 */
  void SetCustomPath(const ActorPtr &actor, const Path path, const bool empty_buffer);

  /**
  * @brief 移除路径。
  *
  * @param actor_id 需要移除路径的车辆ID。
  * @param remove_path 是否移除路径。
  */
  void RemoveUploadPath(const ActorId &actor_id, const bool remove_path);

  /**
 * @brief 更新已设置的路径。
 *
 * @param actor_id 需要更新路径的车辆ID。
 * @param path 新的路径对象。
 */
  void UpdateUploadPath(const ActorId &actor_id, const Path path);

  /**
 * @brief 设置自定义路线。
 *
 * @param actor 需要设置自定义路线的车辆对象。
 * @param route 自定义路线对象。
 * @param empty_buffer 是否清空动作缓冲区。
 */
  void SetImportedRoute(const ActorPtr &actor, const Route route, const bool empty_buffer);

  /**
 * @brief 移除路线。
 *
 * @param actor_id 需要移除路线的车辆ID。
 * @param remove_path 是否移除路线。
 */
  void RemoveImportedRoute(const ActorId &actor_id, const bool remove_path);

  /**
 * @brief 更新已设置的路线。
 *
 * @param actor_id 需要更新路线的车辆ID。
 * @param route 新的路线对象。
 */
  void UpdateImportedRoute(const ActorId &actor_id, const Route route);

  /**
  * @brief 设置自动重生休眠车辆的模式。
  *
  * @param mode_switch 是否启用自动重生休眠车辆模式。
  */
  void SetRespawnDormantVehicles(const bool mode_switch);

  /**
 * @brief 设置休眠车辆重生的边界。
 *
 * @param lower_bound 重生的下边界值。
 * @param upper_bound 重生的上边界值。
 */
  void SetBoundariesRespawnDormantVehicles(const float lower_bound, const float upper_bound);

  /**
 * @brief 设置最大的重生边界。
 *
 * @param lower 最小的重生边界值。
 * @param upper 最大的重生边界值。
 */
  void SetMaxBoundaries(const float lower, const float upper);
  /**
 * @brief 关闭交通管理器。
 */
  virtual void ShutDown();

  /**
  * @brief 获取车辆的下一个动作。
  *
  * @param actor_id 需要获取动作的车辆ID。
  * @return 车辆的下一个动作。
  */
  Action GetNextAction(const ActorId &actor_id);

  /**
  * @brief 获取车辆的动作缓冲区。
  *
  * @param actor_id 需要获取动作缓冲区的车辆ID。
  * @return 车辆的动作缓冲区。
  */
  ActionBuffer GetActionBuffer(const ActorId &actor_id);

  /**
 * @brief 提供同步Tick。
 *
 * @return 是否成功执行同步Tick。
 */
  bool SynchronousTick();

  /**
  * @brief 获取CARLA回合信息。
  *
  * @return CARLA回合信息的代理对象。
  */
  carla::client::detail::EpisodeProxy& GetEpisodeProxy();

  /**
 * @brief 检查服务器是否存活。
 */
  void HealthCheckRemoteTM();

  /**
 * @brief 设置随机化种子。
 *
 * @param seed 随机化种子值。
 */
  void SetRandomDeviceSeed(const uint64_t seed);

private:

	/**
   * @brief 远程客户端，使用IP和端口信息连接到远程RPC交通管理器服务器。
   */
  TrafficManagerClient client;

  /**
  * @brief CARLA客户端连接对象。
  */
  carla::client::detail::EpisodeProxy episodeProxyTM;
  /**
 * @brief 条件变量，用于线程同步。
 */
  std::condition_variable _cv;
  /**
 * @brief 互斥锁，用于保护共享资源。
 */
  std::mutex _mutex;
  /**
 * @brief 保持活动状态标志。
 */
  bool _keep_alive = true;
};

} // namespace traffic_manager
} // namespace carla
