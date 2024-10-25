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
  /// @brief 用于跟踪注册参与者状态变化的计数器
  int registered_vehicles_state;
  /// @brief 当前更新周期中注册到交通管理器的车辆列表  
  /// 使用ActorId类型存储车辆的唯一标识符
  std::vector<ActorId> vehicle_id_list;
  /// @brief 指向本地地图缓存的指针  
  /// 使用智能指针管理InMemoryMap对象，用于存储和访问地图数据
  LocalMapPtr local_map;
  /// @brief 存储所有车辆路径点的缓冲区结构映射  
  /// 使用BufferMap类型（可能是自定义的映射类型）来存储每个车辆的路径点缓冲区
  BufferMap buffer_map;
  /// @brief 用于跟踪交通车辆路径的对象 
  /// TrackTraffic对象可能包含有关交通车辆当前位置和预期路径的信息
  TrackTraffic track_traffic;
  /// @brief 包含模拟中所有参与者当前状态的类型  
  /// 用于表示模拟中所有参与者的状态信息
  SimulationState simulation_state;
  /// @brief 在异步模式下用于计算时间差(dt)的时间实例  
  /// 使用TimePoint类型表示一个具体的时间点，用于与上一个更新时间点进行比较，以计算时间差
  TimePoint previous_update_instance;
  /// @brief 参数化对象  
  /// 包含交通管理器运行所需的各种配置参数
  Parameters parameters;
  /// @brief 存储定位阶段输出数据的数组  
  /// 用于存储定位阶段产生的数据
  LocalizationFrame localization_frame;
  /// @brief 存储碰撞避免阶段输出数据的数组  
  /// 用于存储碰撞避免阶段产生的数据
  CollisionFrame collision_frame;
  /// @brief 存储交通灯响应阶段输出数据的数组  
  /// 用于存储交通灯响应阶段产生的数据
  TLFrame tl_frame;
  /// @brief 存储运动规划阶段输出数据的数组  
  /// 用于存储运动规划阶段产生的控制指令
  ControlFrame control_frame;
  /// @brief 用于跟踪当前为帧保留的数组空间的变量 
  /// 这是一个无符号64位整数，用于记录为各个帧数组预留的空间大小
  uint64_t current_reserved_capacity {0u};
  /// @brief 表示交通管理器核心操作的各种阶段  
  /// 这些阶段包括定位、碰撞避免、交通灯响应、运动规划和车辆灯光控制等
  LocalizationStage localization_stage;
  CollisionStage collision_stage;
  TrafficLightStage traffic_light_stage;
  MotionPlanStage motion_plan_stage;
  VehicleLightStage vehicle_light_stage;
  /// @brief 自动驾驶局部路径规划模块（ALSM）  
  /// ALSM可能是一个用于生成局部路径规划算法的模块或对象
  ALSM alsm;
  /// @brief 交通管理器服务器实例。  
  /// 负责处理来自客户端的请求，并管理交通管理器的运行
  TrafficManagerServer server;
  /// @brief 用于打开/关闭交通管理器的开关  
  /// 这是一个原子布尔变量，用于线程安全地控制交通管理器的运行状态
  std::atomic<bool> run_traffic_manger{true};
  /// @brief 用于标记步骤开始和结束的标志 
  /// 使用std::atomic<bool>确保跨线程的原子操作，避免数据竞争
  std::atomic<bool> step_begin{false};
  std::atomic<bool> step_end{false};
  /// @brief 用于同步执行进度的互斥锁 
  /// std::mutex用于保护共享资源，确保同一时间只有一个线程可以访问
  std::mutex step_execution_mutex;
  /// @brief 用于同步执行进度的条件变量
  /// std::condition_variable用于线程间的同步，当一个线程需要等待某个条件成立时，可以阻塞在该条件变量上，直到另一个线程通知条件已成立
  std::condition_variable step_begin_trigger;
  std::condition_variable step_end_trigger;
  /// @brief 用于顺序执行子组件的单个工作线程  
  /// 使用std::unique_ptr<std::thread>管理线程的生命周期，确保线程在不再需要时能够被正确销毁
  std::unique_ptr<std::thread> worker_thread;
  /// @brief 随机化种子  
  /// 使用当前时间作为随机化种子，确保每次程序运行时都能产生不同的随机序列
  uint64_t seed {static_cast<uint64_t>(time(NULL))};
  /// @brief 持有每辆车的随机设备的结构体  
  /// RandomGenerator是一个用于生成随机数的类或结构体，使用seed进行初始化
  RandomGenerator random_device = RandomGenerator(seed);
  /// @brief 标记为删除的参与者ID列表 
  /// 用于存储需要在某个时间点被移除的参与者ID
  std::vector<ActorId> marked_for_removal;
  /// @brief 防止在帧数组重新分配期间注册车辆的互斥锁  
  /// 用于保护车辆注册操作，确保在帧数组重新分配时不会有新的车辆被注册
  std::mutex registration_mutex;
  /// @brief 检查一个交通灯组中的所有交通灯是否都被冻结的方法
  ///   
  /// @param tl_to_freeze 要检查的交通灯组 
  /// @return 如果所有交通灯都被冻结，则返回true；否则返回false
  bool CheckAllFrozen(TLGroup tl_to_freeze);

public:
    /// @brief 私有构造函数，用于单例生命周期管理  
      ///   
      /// 通过私有构造函数确保TrafficManagerLocal类只能有一个实例被创建，并通过其他静态方法访问该实例 
      ///   
      /// @param longitudinal_PID_parameters 纵向PID控制参数 
      /// @param longitudinal_highway_PID_parameters 高速公路纵向PID控制参数  
      /// @param lateral_PID_parameters 横向PID控制参数  
      /// @param lateral_highway_PID_parameters 高速公路横向PID控制参数  
      /// @param perc_decrease_from_limit 从限速降低的百分比 
      /// @param episode_proxy 模拟器中的episode代理对象  
      /// @param RPCportTM 交通管理器使用的RPC端口号
  TrafficManagerLocal(std::vector<float> longitudinal_PID_parameters,
                      std::vector<float> longitudinal_highway_PID_parameters,
                      std::vector<float> lateral_PID_parameters,
                      std::vector<float> lateral_highway_PID_parameters,
                      float perc_decrease_from_limit,
                      cc::detail::EpisodeProxy &episode_proxy,
                      uint16_t &RPCportTM);

  /// @brief 析构函数 
  /// 虚拟析构函数，用于确保派生类能够正确地被销毁
  virtual ~TrafficManagerLocal();

  /// @brief 设置内存中的地图 
  /// 此方法用于初始化或设置TrafficManagerLocal所使用的内存地图
  void SetupLocalMap();

  /// @brief 启动交通管理器  
  /// 此方法用于启动TrafficManagerLocal，开始其管理交通的功能
  void Start();

  /// @brief 启动线程以顺序运行交通管理器  
  /// 此方法将创建一个新线程（如果尚未创建），并在该线程中顺序运行交通管理器的逻辑
  void Run();

  /// @brief 停止交通管理器  
  /// 此方法用于停止TrafficManagerLocal的运行，并可能进行必要的清理工作
  void Stop();

  /// @brief 释放交通管理器 
  /// 此方法用于释放TrafficManagerLocal所占用的资源，例如线程、内存等
  void Release();

  /// @brief 重置交通管理器  
  /// 此方法将TrafficManagerLocal重置为其初始状态，清除所有已注册的车辆和设置
  void Reset();

  /// @brief 向交通管理器注册车辆  
  /// @param actor_list 要注册的车辆列表，每个元素都是一个指向Actor的指针
  void RegisterVehicles(const std::vector<ActorPtr> &actor_list);

  /// @brief 从交通管理器注销车辆  
  /// @param actor_list 要注销的车辆列表，每个元素都是一个指向Actor的指针
  void UnregisterVehicles(const std::vector<ActorPtr> &actor_list);

  /// @brief 设置车辆相对于限速的速度百分比差异  
  /// @param actor 要设置速度差异的车辆指针  
  /// @param percentage 速度百分比差异，如果小于0，则表示速度百分比增加
  void SetPercentageSpeedDifference(const ActorPtr &actor, const float percentage);

  /// @brief 设置车辆的精确期望速度  
  /// @param actor 要设置期望速度的车辆指针  
  /// @param value 车辆的期望速度值
  void SetDesiredSpeed(const ActorPtr &actor, const float value);

  /// @brief 设置全局相对于限速的速度百分比差异 
  /// @param percentage 全局速度百分比差异，如果小于0，则表示速度百分比增加  
  /// 此设置将影响所有已注册的车辆
  void SetGlobalPercentageSpeedDifference(float const percentage);

  /// @brief 设置车辆相对于中心线的车道偏移量  
  /// @param actor 要设置车道偏移量的车辆指针  
  /// @param offset 车道偏移量，正值表示向右偏移，负值表示向左偏移
  void SetLaneOffset(const ActorPtr &actor, const float offset);

  /// @brief 设置全局车道相对于中心线的偏移量。  
  ///   
  /// @param offset 车道偏移量。正值表示向右偏移，负值表示向左偏移。  
  /// 此设置将影响所有由交通管理器管理的车辆
  void SetGlobalLaneOffset(float const offset);

  /// @brief 设置车辆的灯光是否自动管理。  
  /// @param actor 要设置灯光管理状态的车辆指针。  
  /// @param do_update 是否启用自动灯光管理。如果为true，则启用；如果为false，则禁用
  void SetUpdateVehicleLights(const ActorPtr &actor, const bool do_update);

  /// @brief 设置两辆车之间的碰撞检测规则。  
  /// @param reference_actor 参考车辆指针，即碰撞检测中的一方。  
  /// @param other_actor 另一车辆指针，即与参考车辆进行碰撞检测的另一方。  
  /// @param detect_collision 是否启用两车之间的碰撞检测。如果为true，则启用；如果为false，则禁用
  void SetCollisionDetection(const ActorPtr &reference_actor, const ActorPtr &other_actor, const bool detect_collision);

  /// @brief 强制车辆进行换道。  
  /// @param actor 要强制换道的车辆指针。  
  /// @param direction 换道方向。如果为true，则向左换道；如果为false，则向右换道
  void SetForceLaneChange(const ActorPtr &actor, const bool direction);

  /// @brief 启用或禁用车辆的自动换道功能。  
  /// @param actor 要设置自动换道状态的车辆指针。  
  /// @param enable 是否启用自动换道。如果为true，则启用；如果为false，则禁用
  void SetAutoLaneChange(const ActorPtr &actor, const bool enable);

  /// @brief 设置车辆与前车应保持的距离。  
  /// @param actor 要设置距离的车辆指针。  
  /// @param distance 车辆与前车应保持的距离值
  void SetDistanceToLeadingVehicle(const ActorPtr &actor, const float distance);

  /// @brief 设置车辆忽略行人碰撞的百分比概率。  
  /// @param actor 要设置忽略行人碰撞概率的车辆指针。  
  /// @param perc 忽略行人碰撞的百分比概率。值范围应在0到100之间。  
  void SetPercentageIgnoreWalkers(const ActorPtr &actor, const float perc);

  /// @brief 设置车辆忽略其他车辆碰撞的百分比概率。  
  /// @param actor 要设置忽略其他车辆碰撞概率的车辆指针。  
  /// @param perc 忽略其他车辆碰撞的百分比概率。值范围应在0到100之间
  void SetPercentageIgnoreVehicles(const ActorPtr &actor, const float perc);

  /// @brief 设置车辆闯红灯的百分比概率。  
  /// @param actor 要设置闯红灯概率的车辆指针。  
  /// @param perc 闯红灯的百分比概率。值范围应在0到100之间
  void SetPercentageRunningLight(const ActorPtr &actor, const float perc);

  /// @brief 设置车辆无视交通标志的百分比概率。  
  /// @param actor 要设置无视交通标志概率的车辆指针。  
  /// @param perc 无视交通标志的百分比概率。值范围应在0到100之间。
  void SetPercentageRunningSign(const ActorPtr &actor, const float perc);

  /// @brief 将交通管理器切换到同步执行模式。  
  /// @param mode 是否启用同步执行模式。如果为true，则启用；如果为false，则禁用。  
  /// 在同步执行模式下，交通管理器的所有操作将按顺序执行，而不是并行执行
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
