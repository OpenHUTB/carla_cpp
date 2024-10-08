
#pragma once

#include <memory>

#include "carla/client/ActorList.h"
#include "carla/client/Timestamp.h"
#include "carla/client/World.h"
#include "carla/Memory.h"

#include "carla/trafficmanager/AtomicActorSet.h"
#include "carla/trafficmanager/CollisionStage.h"
#include "carla/trafficmanager/DataStructures.h"
#include "carla/trafficmanager/InMemoryMap.h"
#include "carla/trafficmanager/LocalizationStage.h"
#include "carla/trafficmanager/MotionPlanStage.h"
#include "carla/trafficmanager/Parameters.h"
#include "carla/trafficmanager/RandomGenerator.h"
#include "carla/trafficmanager/SimulationState.h"
#include "carla/trafficmanager/TrafficLightStage.h"
#include "carla/trafficmanager/VehicleLightStage.h"

namespace carla {
namespace traffic_manager {

using namespace constants::HybridMode;  // 引入混合模式常量
using namespace constants::VehicleRemoval; // 引入车辆移除常量

namespace chr = std::chrono;  // 引用时间相关的命名空间
namespace cg = carla::geom;   // 引用几何相关的命名空间
namespace cc = carla::client;  // 引用客户端相关的命名空间

using ParticipantList = carla::SharedPtr<cc::ActorList>; // 定义参与者列表共享指针类型
using ParticipantMap = std::unordered_map<ActorId, ActorPtr>; // 定义参与者映射表类型
using IdleTimeMap = std::unordered_map<ActorId, double>; // 定义闲置时间映射表类型
using LocalMapPtr = std::shared_ptr<InMemoryMap>; // 定义本地地图共享指针类型

/// ALSM: 代理生命周期和状态管理
/// 此类具有更新运动状态本地缓存的功能
/// 并管理模拟中车辆数量变化的内存和清理。
class ALSM {

private:
  AtomicActorSet &registered_vehicles; // 引用已注册参与者的原子集合
  ParticipantMap unregistered_participants; // 存储未注册参与者的结构
  BufferMap &buffer_map; // 引用缓冲区映射
  IdleTimeMap idle_time; // 存储参与者在位置上停留时间的结构
  ParticipantMap hero_participants; // 存储角色名称为"hero"的参与者
  TrackTraffic &track_traffic; // 引用交通跟踪对象
  std::vector<ActorId>& marked_for_removal; // 标记待移除参与者的数组
  const Parameters &parameters; // 引用参数对象
  const cc::World &world; // 引用世界对象
  const LocalMapPtr &local_map; // 引用本地地图指针
  SimulationState &simulation_state; // 引用仿真状态对象
  LocalizationStage &localization_stage; // 引用定位阶段对象
  CollisionStage &collision_stage; // 引用碰撞阶段对象
  TrafficLightStage &traffic_light_stage; // 引用交通灯阶段对象
  MotionPlanStage &motion_plan_stage; // 引用运动规划阶段对象
  VehicleLightStage &vehicle_light_stage; // 引用车辆灯光阶段对象
  double elapsed_last_participant_destruction {0.0}; // 记录自上次因闲置过久而销毁参与者的时间
  cc::Timestamp current_timestamp; // 当前时间戳
  std::unordered_map<ActorId, bool> has_physics_enabled; // 存储每个参与者是否启用物理的映射

  // 更新已注册参与者在某位置上停留的时间
  void UpdateIdleTime(std::pair<ActorId, double>& max_idle_time, const ActorId& actor_id);

  // 判断一辆车是否长时间停滞不前
  bool IsVehicleStuck(const ActorId& actor_id);

  // 确定自上次更新以来在仿真中新生成的参与者
  void IdentifyNewParticipants(const ParticipantList &participant_list);

  using DestroyeddParticipants = std::pair<ActorIdSet, ActorIdSet>; // 定义删除参与者的数据类型
  // 确定在上一帧中删除的参与者
  // 返回已注册和未注册参与者的数组
  DestroyeddParticipants IdentifyDestroyedParticipants(const ParticipantList &participant_list);

  using IdleInfo = std::pair<ActorId, double>; // 定义闲置信息的数据类型
  void UpdateRegisteredParticipantsData(const bool hybrid_physics_mode, IdleInfo &max_idle_time);

  // 更新参与者数据
  void UpdateData(const bool hybrid_physics_mode, const Actor &vehicle,
                  const bool hero_actor_present, const float physics_radius_square);

  // 更新未注册参与者的数据
  void UpdateUnregisteredParticipantsData();

public:
  // 构造函数
  ALSM(AtomicActorSet &registered_vehicles,
       BufferMap &buffer_map,
       TrackTraffic &track_traffic,
       std::vector<ActorId>& marked_for_removal,
       const Parameters &parameters,
       const cc::World &world,
       const LocalMapPtr &local_map,
       SimulationState &simulation_state,
       LocalizationStage &localization_stage,
       CollisionStage &collision_stage,
       TrafficLightStage &traffic_light_stage,
       MotionPlanStage &motion_plan_stage,
       VehicleLightStage &vehicle_light_stage);

  // 更新方法
  void Update();

  // 从交通管理中移除参与者，并清理与该车辆相关的各种数据
  void RemoveParticipant(const ActorId actor_id, const bool registered_actor);

  // 重置方法
  void Reset();
};