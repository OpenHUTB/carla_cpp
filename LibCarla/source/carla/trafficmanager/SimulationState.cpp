#include "carla/trafficmanager/SimulationState.h"
// carla 命名空间
namespace carla {
// traffic_manager 命名空间
namespace traffic_manager {
// 构造函数，初始化 SimulationState 对象
SimulationState::SimulationState() {}
// 向模拟状态中添加一个actor
void SimulationState::AddActor(ActorId actor_id,
                               KinematicState kinematic_state,
                               StaticAttributes attributes,
                               TrafficLightState tl_state) {
  actor_set.insert(actor_id);// 将actor的ID 插入到 actor_set 中
  kinematic_state_map.insert({actor_id, kinematic_state});// 将actor的ID和运动状态插入到 kinematic_state_map 中
  static_attribute_map.insert({actor_id, attributes});// 将actor的ID和静态属性插入到 static_attribute_map 中
  tl_state_map.insert({actor_id, tl_state});// 将actor的ID和交通灯状态插入到 tl_state_map 中
}
// 检查模拟状态中是否包含特定的actor的ID
bool SimulationState::ContainsActor(ActorId actor_id) const {
// 如果在 actor_set 中找到该actor的ID，则返回 true，否则返回 false
  return actor_set.find(actor_id) != actor_set.end();
}
// 从模拟状态中移除一个actor
void SimulationState::RemoveActor(ActorId actor_id) {
  actor_set.erase(actor_id); // 从 actor_set 中删除该actor的ID
  kinematic_state_map.erase(actor_id);// 从 kinematic_state_map 中删除该actor的运动状态
  static_attribute_map.erase(actor_id);// 从 static_attribute_map 中删除该actor的静态属性
  tl_state_map.erase(actor_id);// 从 tl_state_map 中删除该actor的交通灯状态
}
// 重置模拟状态，清空所有数据结构
void SimulationState::Reset() {
  actor_set.clear();// 清空 actor_set
  kinematic_state_map.clear();// 清空 kinematic_state_map
  static_attribute_map.clear();// 清空 static_attribute_map
  tl_state_map.clear(); // 清空 tl_state_map
}
// 更新特定actor的运动状态
void SimulationState::UpdateKinematicState(ActorId actor_id, KinematicState state) {
  kinematic_state_map.at(actor_id) = state;// 使用 at 方法访问并更新特定actor的运动状态
}
// 更新特定actor的混合结束位置
void SimulationState::UpdateKinematicHybridEndLocation(ActorId actor_id, cg::Location location) {
  kinematic_state_map.at(actor_id).hybrid_end_location = location;// 使用 at 方法访问并更新特定actor的混合结束位置
}
// 更新特定actor的交通灯状态，注意特殊的绿色-黄色状态过渡处理
void SimulationState::UpdateTrafficLightState(ActorId actor_id, TrafficLightState state) {
  // The green-yellow state transition is not notified to the vehicle. This is done to avoid
  // having vehicles stopped very near the intersection when only the rear part of the vehicle
  // is colliding with the trigger volume of the traffic light.
  auto previous_tl_state = GetTLS(actor_id);
  if (previous_tl_state.at_traffic_light && previous_tl_state.tl_state == TLS::Green) {
    state.tl_state = TLS::Green;
  }
// 使用 at 方法访问并更新特定actor的交通灯状态
  tl_state_map.at(actor_id) = state;
}
// 获取特定actor的位置
cg::Location SimulationState::GetLocation(ActorId actor_id) const {
// 使用 at 方法访问并返回特定actor的位置	
  return kinematic_state_map.at(actor_id).location;
}
// 获取特定actor的混合结束位置
cg::Location SimulationState::GetHybridEndLocation(ActorId actor_id) const {
  return kinematic_state_map.at(actor_id).hybrid_end_location;// 使用 at 方法访问并返回特定actor的混合结束位置
}
// 获取特定actor的旋转状态
cg::Rotation SimulationState::GetRotation(ActorId actor_id) const {
  return kinematic_state_map.at(actor_id).rotation;// 使用 at 方法访问并返回特定actor的旋转状态
}
// 获取特定actor的前进方向向量
cg::Vector3D SimulationState::GetHeading(ActorId actor_id) const {
  return kinematic_state_map.at(actor_id).rotation.GetForwardVector(); // 使用 at 方法访问特定actor的旋转状态，然后获取其前进方向向量并返回
}
// 获取特定actor的速度向量
cg::Vector3D SimulationState::GetVelocity(ActorId actor_id) const {
  return kinematic_state_map.at(actor_id).velocity;// 使用 at 方法访问并返回特定actor的速度向量
}
// 获取特定actor的速度限制
float SimulationState::GetSpeedLimit(ActorId actor_id) const {
  return kinematic_state_map.at(actor_id).speed_limit; // 使用 at 方法访问并返回特定actor的速度限制
}
// 检查特定actor的物理模拟是否启用
bool SimulationState::IsPhysicsEnabled(ActorId actor_id) const {
  return kinematic_state_map.at(actor_id).physics_enabled; // 使用 at 方法访问并返回特定actor的物理模拟启用状态
}
// 检查特定actor是否处于休眠状态
bool SimulationState::IsDormant(ActorId actor_id) const {
  return kinematic_state_map.at(actor_id).is_dormant;// 使用 at 方法访问并返回特定actor的休眠状态
}
// 获取特定actor的交通灯状态
TrafficLightState SimulationState::GetTLS(ActorId actor_id) const {
  return tl_state_map.at(actor_id);// 使用 at 方法访问并返回特定actor的交通灯状态
}
// 获取特定actor的类型
ActorType SimulationState::GetType(ActorId actor_id) const {
  return static_attribute_map.at(actor_id).actor_type; // 使用 at 方法访问特定actor的静态属性，然后返回actor类型
}
// 获取特定actor的尺寸
cg::Vector3D SimulationState::GetDimensions(ActorId actor_id) const {
// 使用 at 方法访问特定actor的静态属性，然后构建并返回actor的尺寸向量	
  const StaticAttributes &attributes = static_attribute_map.at(actor_id);
  return cg::Vector3D(attributes.half_length, attributes.half_width, attributes.half_height);
}

} // namespace  traffic_manager
} // namespace carla
