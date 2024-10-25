// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <atomic>///@brief 包含C++原子操作库，用于线程安全的计数器和标志位
#include <chrono>///@brief 包含C++时间库，用于时间测量和延迟
#include <mutex>///@brief 包含C++互斥锁库，用于线程同步
#include <thread>///@brief 包含C++线程库，用于多线程编程
#include <vector>///@brief 包含C++动态数组库，用于存储和管理序列化的数据

#include "carla/client/detail/EpisodeProxy.h"///@brief 包含CARLA客户端的Episode代理类，用于管理仿真场景的一个回合
#include "carla/client/TrafficLight.h"///@brief 包含CARLA客户端的交通灯控制类
#include "carla/client/World.h"///@brief 包含CARLA客户端的世界管理类，用于访问和修改仿真世界
#include "carla/Memory.h"///@brief 包含CARLA的内存管理类，用于管理内存分配和释放
#include "carla/rpc/Command.h"///@brief 包含CARLA的RPC命令处理类，用于远程过程调用

#include "carla/trafficmanager/AtomicActorSet.h"///@brief 包含交通管理器中的原子参与者集合类，用于管理仿真中的参与者（如车辆、行人）
#include "carla/trafficmanager/InMemoryMap.h"///@brief 包含交通管理器的内存地图类，用于在内存中存储地图数据
#include "carla/trafficmanager/Parameters.h"///@brief 包含交通管理器的参数配置类，用于配置交通管理器的各种参数
#include "carla/trafficmanager/RandomGenerator.h"///@brief 包含交通管理器的随机数生成器类，用于生成随机数或随机序列
#include "carla/trafficmanager/SimulationState.h"///@brief 包含交通管理器的仿真状态类，用于管理仿真的全局状态
#include "carla/trafficmanager/TrackTraffic.h"///@brief 包含交通管理器的流量跟踪类，用于跟踪和管理仿真中的交通流量
#include "carla/trafficmanager/TrafficManagerBase.h"///@brief 包含交通管理器的基类，定义了交通管理器的基本接口和功能
#include "carla/trafficmanager/TrafficManagerServer.h"///@brief 包含交通管理器的服务器类，用于管理交通管理器的网络通信

#include "carla/trafficmanager/ALSM.h"///@brief 包含交通管理器的ALSM（高级状态机）类，用于管理交通参与者的状态转换
#include "carla/trafficmanager/LocalizationStage.h"///@brief 包含交通管理器的定位阶段类，用于处理交通参与者的定位信息
#include "carla/trafficmanager/CollisionStage.h"///@brief 包含交通管理器的碰撞检测阶段类，用于检测和处理交通参与者之间的碰撞
#include "carla/trafficmanager/TrafficLightStage.h"///@brief 包含交通管理器的交通灯阶段类，用于处理交通灯的控制和同步
#include "carla/trafficmanager/MotionPlanStage.h"///@brief 包含交通管理器的运动规划阶段类，用于规划和执行交通参与者的运动轨迹
///@brief CARLA 交通管理器命名空间，包含交通管理相关的类和函数
namespace carla {
namespace traffic_manager {
///@brief 命名空间别名，简化std::chrono的使用
namespace chr = std::chrono;
///@brief 使用命名空间别名，简化std::chrono_literals的使用，以便于定义时间字面量
using namespace std::chrono_literals;
///@brief 时间点类型，使用系统时钟和纳秒精度
using TimePoint = chr::time_point<chr::system_clock, chr::nanoseconds>;
///@brief 交通灯组类型，使用CARLA的智能指针管理交通灯对象的集合
using TLGroup = std::vector<carla::SharedPtr<carla::client::TrafficLight>>;
///@brief 本地地图指针类型，使用智能指针管理InMemoryMap对象
using LocalMapPtr = std::shared_ptr<InMemoryMap>;
///@brief 引入HYBRID_MODE_DT常量，用于定义混合模式下的时间间隔
using constants::HybridMode::HYBRID_MODE_DT;

/**
 * @class TrafficManagerLocal
 * @brief 交通管理器本地类，通过消息传递机制整合交通管理器的各个阶段。
 * @inherits TrafficManagerBase
 *
 * 该类继承自TrafficManagerBase，并实现了通过消息传递机制整合交通管理器的各个阶段的功能。
 */
class TrafficManagerLocal : public TrafficManagerBase {

private:
    /**
       * @brief 纵向PID控制器参数集合。
       *
       * 包含用于纵向（前进方向）PID（比例-积分-微分）控制器的参数集合。
    */
  std::vector<float> longitudinal_PID_parameters;
  /**
     * @brief 高速公路纵向PID控制器参数集合。
     *
     * 包含用于高速公路场景下纵向PID控制器的参数集合。
     */
  std::vector<float> longitudinal_highway_PID_parameters;
  /**
    * @brief 横向PID控制器参数集合。
    *
    * 包含用于横向（侧向）PID控制器的参数集合。
    */
  std::vector<float> lateral_PID_parameters;
  /**
     * @brief 高速公路横向PID控制器参数集合。
     *
     * 包含用于高速公路场景下横向PID控制器的参数集合。
     */
  std::vector<float> lateral_highway_PID_parameters;
  /**
       * @brief CARLA 客户端连接对象。
       *
       * 用于与CARLA仿真环境进行通信的客户端连接对象。
       */
  carla::client::detail::EpisodeProxy episode_proxy;
  /**
     * @brief CARLA 世界对象。
     *
     * 表示CARLA仿真世界的一个对象，用于访问和操作仿真世界中的元素。
     */
  cc::World world;
  /**
     * @brief 注册到交通管理器的所有参与者集合。
     *
     * 使用AtomicActorSet管理所有已注册到交通管理器的参与者（如车辆、行人等）。
     */
  AtomicActorSet registered_vehicles;
  /// State counter to track changes in registered actors.
  int registered_vehicles_state;
  /// List of vehicles registered with the traffic manager in
  /// current update cycle.
  std::vector<ActorId> vehicle_id_list;
  /// Pointer to local map cache.
  LocalMapPtr local_map;
  /// Structures to hold waypoint buffers for all vehicles.
  BufferMap buffer_map;
  /// Object for tracking paths of the traffic vehicles.
  TrackTraffic track_traffic;
  /// Type containing the current state of all actors involved in the simulation.
  SimulationState simulation_state;
  /// Time instance used to calculate dt in asynchronous mode.
  TimePoint previous_update_instance;
  /// Parameterization object.
  Parameters parameters;
  /// Array to hold output data of localization stage.
  LocalizationFrame localization_frame;
  /// Array to hold output data of collision avoidance.
  CollisionFrame collision_frame;
  /// Array to hold output data of traffic light response.
  TLFrame tl_frame;
  /// Array to hold output data of motion planning.
  ControlFrame control_frame;
  /// Variable to keep track of currently reserved array space for frames.
  uint64_t current_reserved_capacity {0u};
  /// Various stages representing core operations of traffic manager.
  LocalizationStage localization_stage;
  CollisionStage collision_stage;
  TrafficLightStage traffic_light_stage;
  MotionPlanStage motion_plan_stage;
  VehicleLightStage vehicle_light_stage;
  ALSM alsm;
  /// Traffic manager server instance.
  TrafficManagerServer server;
  /// Switch to turn on / turn off traffic manager.
  std::atomic<bool> run_traffic_manger{true};
  /// Flags to signal step begin and end.
  std::atomic<bool> step_begin{false};
  std::atomic<bool> step_end{false};
  /// Mutex for progressing synchronous execution.
  std::mutex step_execution_mutex;
  /// Condition variables for progressing synchronous execution.
  std::condition_variable step_begin_trigger;
  std::condition_variable step_end_trigger;
  /// Single worker thread for sequential execution of sub-components.
  std::unique_ptr<std::thread> worker_thread;
  /// Randomization seed.
  uint64_t seed {static_cast<uint64_t>(time(NULL))};
  /// Structure holding random devices per vehicle.
  RandomGenerator random_device = RandomGenerator(seed);
  std::vector<ActorId> marked_for_removal;
  /// Mutex to prevent vehicle registration during frame array re-allocation.
  std::mutex registration_mutex;

  /// Method to check if all traffic lights are frozen in a group.
  bool CheckAllFrozen(TLGroup tl_to_freeze);

public:
  /// Private constructor for singleton lifecycle management.
  TrafficManagerLocal(std::vector<float> longitudinal_PID_parameters,
                      std::vector<float> longitudinal_highway_PID_parameters,
                      std::vector<float> lateral_PID_parameters,
                      std::vector<float> lateral_highway_PID_parameters,
                      float perc_decrease_from_limit,
                      cc::detail::EpisodeProxy &episode_proxy,
                      uint16_t &RPCportTM);

  /// Destructor.
  virtual ~TrafficManagerLocal();

  /// Method to setup InMemoryMap.
  void SetupLocalMap();

  /// To start the TrafficManager.
  void Start();

  /// Initiates thread to run the TrafficManager sequentially.
  void Run();

  /// To stop the TrafficManager.
  void Stop();

  /// To release the traffic manager.
  void Release();

  /// To reset the traffic manager.
  void Reset();

  /// This method registers a vehicle with the traffic manager.
  void RegisterVehicles(const std::vector<ActorPtr> &actor_list);

  /// This method unregisters a vehicle from traffic manager.
  void UnregisterVehicles(const std::vector<ActorPtr> &actor_list);

  /// Method to set a vehicle's % decrease in velocity with respect to the speed limit.
  /// If less than 0, it's a % increase.
  void SetPercentageSpeedDifference(const ActorPtr &actor, const float percentage);

  /// Set a vehicle's exact desired velocity.
  void SetDesiredSpeed(const ActorPtr &actor, const float value);

  /// Method to set a global % decrease in velocity with respect to the speed limit.
  /// If less than 0, it's a % increase.
  void SetGlobalPercentageSpeedDifference(float const percentage);

  /// Method to set a lane offset displacement from the center line.
  /// Positive values imply a right offset while negative ones mean a left one.
  void SetLaneOffset(const ActorPtr &actor, const float offset);

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

  /// Method to specify the % chance of ignoring collisions with any walker.
  void SetPercentageIgnoreWalkers(const ActorPtr &actor, const float perc);

  /// Method to specify the % chance of ignoring collisions with any vehicle.
  void SetPercentageIgnoreVehicles(const ActorPtr &actor, const float perc);

  /// Method to specify the % chance of running any traffic light.
  void SetPercentageRunningLight(const ActorPtr &actor, const float perc);

  /// Method to specify the % chance of running any traffic sign.
  void SetPercentageRunningSign(const ActorPtr &actor, const float perc);

  /// Method to switch traffic manager into synchronous execution.
  void SetSynchronousMode(bool mode);

  /// Method to set Tick timeout for synchronous execution.
  void SetSynchronousModeTimeOutInMiliSecond(double time);

  /// Method to provide synchronous tick.
  bool SynchronousTick();

  /// Get CARLA episode information.
  carla::client::detail::EpisodeProxy &GetEpisodeProxy();

  /// Get list of all registered vehicles.
  std::vector<ActorId> GetRegisteredVehiclesIDs();

  /// Method to specify how much distance a vehicle should maintain to
  /// the Global leading vehicle.
  void SetGlobalDistanceToLeadingVehicle(const float distance);

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

  /// Method to set randomization seed.
  void SetRandomDeviceSeed(const uint64_t _seed);

  /// Method to set Open Street Map mode.
  void SetOSMMode(const bool mode_switch);

  /// Method to set our own imported path.
  void SetCustomPath(const ActorPtr &actor, const Path path, const bool empty_buffer);

  /// Method to remove a list of points.
  void RemoveUploadPath(const ActorId &actor_id, const bool remove_path);

  /// Method to update an already set list of points.
  void UpdateUploadPath(const ActorId &actor_id, const Path path);

  /// Method to set our own imported route.
  void SetImportedRoute(const ActorPtr &actor, const Route route, const bool empty_buffer);

  /// Method to remove a route.
  void RemoveImportedRoute(const ActorId &actor_id, const bool remove_path);

  /// Method to update an already set route.
  void UpdateImportedRoute(const ActorId &actor_id, const Route route);

  /// Method to set automatic respawn of dormant vehicles.
  void SetRespawnDormantVehicles(const bool mode_switch);

  /// Method to set boundaries to respawn of dormant vehicles.
  void SetBoundariesRespawnDormantVehicles(const float lower_bound, const float upper_bound);

  /// Method to set limits for boundaries when respawning dormant vehicles.
  void SetMaxBoundaries(const float lower, const float upper);

  /// Method to get the vehicle's next action.
  Action GetNextAction(const ActorId &actor_id);

  /// Method to get the vehicle's action buffer.
  ActionBuffer GetActionBuffer(const ActorId &actor_id);

  void ShutDown() {};
};

} // namespace traffic_manager
} // namespace carla
