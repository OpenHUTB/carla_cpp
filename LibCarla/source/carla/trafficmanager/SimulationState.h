
#pragma once

#include <unordered_set> // 引入无序集合头文件

#include "carla/trafficmanager/DataStructures.h" // 引入数据结构的头文件

namespace carla {
namespace traffic_manager {

// 定义演员类型的枚举
enum ActorType {
  Vehicle,      // 车辆
  Pedestrian,   // 行人
  Any           // 任意类型
};

// 描述运动状态的结构体
struct KinematicState {
  cg::Location location;         // 位置
  cg::Rotation rotation;         // 旋转
  cg::Vector3D velocity;        // 速度
  float speed_limit;            // 速度限制
  bool physics_enabled;         // 是否启用物理模拟
  bool is_dormant;              // 是否处于休眠状态
  cg::Location hybrid_end_location; // 混合结束位置
};
using KinematicStateMap = std::unordered_map<ActorId, KinematicState>; // 定义运动状态映射

// 描述交通灯状态的结构体
struct TrafficLightState {
  TLS tl_state;                // 交通灯状态
  bool at_traffic_light;       // 是否在交通灯处
};
using TrafficLightStateMap = std::unordered_map<ActorId, TrafficLightState>; // 定义交通灯状态映射

// 描述静态属性的结构体
struct StaticAttributes {
  ActorType actor_type;        // 演员类型
  float half_length;           // 半长
  float half_width;            // 半宽
  float half_height;           // 半高
};
using StaticAttributeMap = std::unordered_map<ActorId, StaticAttributes>; // 定义静态属性映射

/// 该类保持了仿真中所有车辆的状态。
class SimulationState {

private:
  // 存储仿真中所有演员的ID的结构
  std::unordered_set<ActorId> actor_set; 
  // 存储演员动态运动相关状态的结构
  KinematicStateMap kinematic_state_map; 
  // 存储演员静态属性的结构
  StaticAttributeMap static_attribute_map; 
  // 存储演员动态交通灯相关状态的结构
  TrafficLightStateMap tl_state_map; 

public :
  SimulationState(); // 构造函数

  // 将演员添加到仿真状态的方法
  void AddActor(ActorId actor_id,
                KinematicState kinematic_state,
                StaticAttributes attributes,
                TrafficLightState tl_state);

  // 验证演员是否当前存在于仿真状态中的方法
  bool ContainsActor(ActorId actor_id) const;

  // 从仿真状态中移除演员的方法
  void RemoveActor(ActorId actor_id);

  // 清除所有状态和演员的方法
  void Reset();

  // 更新演员运动状态的方法
  void UpdateKinematicState(ActorId actor_id, KinematicState state);

  // 更新演员混合结束位置的方法
  void UpdateKinematicHybridEndLocation(ActorId actor_id, cg::Location location);

  // 更新交通灯状态的方法
  void UpdateTrafficLightState(ActorId actor_id, TrafficLightState state);

  // 获取演员位置的方法
  cg::Location GetLocation(const ActorId actor_id) const;

  // 获取演员混合结束位置的方法
  cg::Location GetHybridEndLocation(const ActorId actor_id) const;

  // 获取演员旋转的方法
  cg::Rotation GetRotation(const ActorId actor_id) const;

  // 获取演员朝向的方法
  cg::Vector3D GetHeading(const ActorId actor_id) const;

  // 获取演员速度的方法
  cg::Vector3D GetVelocity(const ActorId actor_id) const;

  // 获取速度限制的方法
  float GetSpeedLimit(const ActorId actor_id) const;

  // 检查物理是否启用的方法
  bool IsPhysicsEnabled(const ActorId actor_id) const;

  // 检查演员是否处于休眠状态的方法
  bool IsDormant(const ActorId actor_id) const;

  // 获取英雄位置的方法
  cg::Location GetHeroLocation(const ActorId actor_id) const;

  // 获取交通灯状态的方法
  TrafficLightState GetTLS(const ActorId actor_id) const;

  // 获取演员类型的方法
  ActorType GetType(const ActorId actor_id) const;

  // 获取演员尺寸的方法
  cg::Vector3D GetDimensions(const ActorId actor_id) const;

};

} // namespace traffic_manager
} // namespace carla
