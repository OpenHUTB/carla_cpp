
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
// 定义在carla命名空间下的traffic_manager命名空间
namespace carla {
namespace traffic_manager {
// 使用 shared_ptr 定义一个指向 InMemoryMap 的别名。
using LocalMapPtr = std::shared_ptr<InMemoryMap>;
// 定义一个无序映射，键为字符串，值为指向 client::Actor 的共享指针。
using TLMap = std::unordered_map<std::string, SharedPtr<client::Actor>>;
// 定义 MotionPlanStage 类，继承自 Stage 类。
class MotionPlanStage: Stage {
private:
  const std::vector<ActorId> &vehicle_id_list;// 引用车辆 ID 列表。
  SimulationState &simulation_state;// 引用模拟状态对象。
  const Parameters &parameters; // 引用参数对象。
  const BufferMap &buffer_map;// 引用缓冲区映射对象。
  TrackTraffic &track_traffic;// 引用跟踪交通对象。
//PID控制参数等用于车辆纵向运动控制
  const std::vector<float> urban_longitudinal_parameters;
  const std::vector<float> highway_longitudinal_parameters;//纵向控制相关的参数列表
  const std::vector<float> urban_lateral_parameters;//车辆横向运动控制相关操作
  const std::vector<float> highway_lateral_parameters;//横向控制相关的参数列表
  const LocalizationFrame &localization_frame;// 引用定位帧对象。
  const CollisionFrame &collision_frame;// 引用碰撞帧对象。
  const TLFrame &tl_frame;//获取交通信号灯相关状态信息
  const cc::World &world;//获取世界的快照等全局信息
  // Structure holding the controller state for registered vehicles.
  std::unordered_map<ActorId, StateEntry> pid_state_map;
  // Structure to keep track of duration between teleportation
  // in hybrid physics mode.
  std::unordered_map<ActorId, cc::Timestamp> teleportation_instance;
  ControlFrame &output_array;
  cc::Timestamp current_timestamp;// 当前时间戳。
  RandomGenerator &random_device;// 引用随机数生成器对象。
  const LocalMapPtr &local_map;// 引用本地地图指针对象。
// 处理碰撞的私有方法。
  std::pair<bool, float> CollisionHandling(const CollisionHazardData &collision_hazard,
                                           const bool tl_hazard,
                                           const cg::Vector3D ego_velocity,
                                           const cg::Vector3D ego_heading,
                                           const float max_target_velocity);
// 判断在路口后是否安全的私有方法。
  bool SafeAfterJunction(const LocalizationData &localization,
                         const bool tl_hazard,
                         const bool collision_emergency_stop);
 // 根据地标获取目标速度的私有方法。
  float GetLandmarkTargetVelocity(const SimpleWaypoint& waypoint,
                                  const cg::Location vehicle_location,
                                  const ActorId actor_id,
                                  float max_target_velocity);
// 根据路点缓冲区获取转弯目标速度的私有方法。
  float GetTurnTargetVelocity(const Buffer &waypoint_buffer,
                              float max_target_velocity);
// 获取三点圆半径的私有方法。
  float GetThreePointCircleRadius(cg::Location first_location,
                                  cg::Location middle_location,
                                  cg::Location last_location);

public:
// 构造函数，初始化成员变量。
// MotionPlanStage函数，用于进行运动规划阶段相关操作，接收多个参数来综合考虑各种信息并生成控制输出
  MotionPlanStage(const std::vector<ActorId> &vehicle_id_list, // 传入的车辆ID列表，以引用方式传递，包含了需要进行运动规划相关操作的车辆标识信息
                  SimulationState &simulation_state,// 模拟状态的引用，在整个运动规划过程中可能会对其进行更新、查询等操作，用于反映当前模拟环境下的整体状态
                  const Parameters &parameters,// 传入的参数结构体引用，包含了运动规划过程中需要用到的各种配置参数、常量等
                  const BufferMap &buffer_map,// 缓冲区地图的引用，可能存储着一些临时数据、缓冲区域相关的信息用于辅助运动规划判断等操作
                  TrackTraffic &track_traffic,// 轨道交通相关信息的引用，也许涉及到交通流、轨道上的车辆行驶情况等方面内容，供运动规划参考
                  const std::vector<float> &urban_longitudinal_parameters,// 城市纵向相关参数的向量引用，里面存放着适用于城市环境下车辆纵向运动规划（比如加速、减速等方面）的一系列参数
                  const std::vector<float> &highway_longitudinal_parameters, // 高速公路纵向相关参数的向量引用，用于高速公路场景下车辆纵向运动规划所需要的对应参数
                  const std::vector<float> &urban_lateral_parameters,// 城市横向相关参数的向量引用，包含城市环境中车辆横向运动（比如变道等操作）规划用到的参数
                  const std::vector<float> &highway_lateral_parameters,// 高速公路横向相关参数的向量引用，针对高速公路场景下车辆横向运动规划所需要的参数
                  const LocalizationFrame &localization_frame,// 定位帧信息的引用，包含车辆当前的位置、姿态等定位相关的数据，用于确定车辆在环境中的准确位置以辅助运动规划
                  const CollisionFrame &collision_frame, // 碰撞帧信息的引用，提供车辆周围可能存在的碰撞风险、障碍物等相关信息，让运动规划能够避免碰撞情况发生
                  const TLFrame &tl_frame, // 交通信号灯帧信息的引用，包含交通信号灯的状态等内容，运动规划需要根据信号灯情况来合理安排车辆的行驶决策
                  const cc::World &world,// 整个世界相关的信息，可能涵盖了整个模拟环境的全貌，比如地图、所有物体等信息，作为运动规划的宏观背景参考
                  ControlFrame &output_array, // 控制帧的引用，用于存储运动规划最终生成的控制输出信息，比如对车辆的转向、加速等控制指令
                  RandomGenerator &random_device,// 随机数生成器的引用，可能在运动规划的某些随机决策环节（例如随机避让策略等情况，如果有涉及的话）会用到它来生成随机数
                  const LocalMapPtr &local_map);// 局部地图指针的引用，指向局部地图相关的数据结构，用于获取车辆周边更详细的地图环境信息辅助进行运动规划
 // 这里通常会放置函数具体的实现逻辑代码，来根据传入的这些参数进行运动规划计算，生成相应的控制输出存放在output_array中，但目前函数体内部代码缺失
 // 更新方法，根据给定的索引进行更新。
  void Update(const unsigned long index);
// 移除指定 actor 的方法。
  void RemoveActor(const ActorId actor_id);
// 重置方法。
  void Reset();
};

} // namespace traffic_manager
} // namespace carla
