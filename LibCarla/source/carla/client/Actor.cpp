// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/client/Actor.h"// 引入Carla模拟器中参与者模块的头文件

#include "carla/Logging.h"// 引入Carla日志模块的头文件
#include "carla/client/detail/Simulator.h"// 引入Carla客户端模拟器实现的细节头文件

namespace carla {
namespace client {
  //这个函数返回Actor当前的位置。
  geom::Location Actor::GetLocation() const {
    return GetEpisode().Lock()->GetActorLocation(*this);
  }
  //返回Actor当前的变换（位置和方向）。
  //实现方式与GetLocation类似，通过GetEpisode().Lock()->GetActorTransform(*this)获取。
  geom::Transform Actor::GetTransform() const {
    return GetEpisode().Lock()->GetActorTransform(*this);
  }
  //返回Actor当前的速度，以三维向量表示。
  //通过GetEpisode().Lock()->GetActorVelocity(*this)获取。
  geom::Vector3D Actor::GetVelocity() const {
    return GetEpisode().Lock()->GetActorVelocity(*this);
  }
  //返回Actor当前的角速度，也是以三维向量表示。
  //实现方式同上，通过GetEpisode().Lock()->GetActorAngularVelocity(*this)获取。

  geom::Vector3D Actor::GetAngularVelocity() const {
    return GetEpisode().Lock()->GetActorAngularVelocity(*this);
  }
  //返回Actor当前的加速度，以三维向量表示。
  //通过GetEpisode().Lock()->GetActorAcceleration(*this)获取。
  geom::Vector3D Actor::GetAcceleration() const {
    return GetEpisode().Lock()->GetActorAcceleration(*this);
  }
  //返回指定组件在世界空间中的变换。
  //这里的componentName是一个字符串，表示Actor上某个特定组件的名称。
  geom::Transform Actor::GetComponentWorldTransform(const std::string componentName) const {
    return GetEpisode().Lock()->GetActorComponentWorldTransform(*this, componentName);
  }
  //返回指定组件相对于Actor自身的变换。
  geom::Transform Actor::GetComponentRelativeTransform(const std::string componentName) const {
    return GetEpisode().Lock()->GetActorComponentRelativeTransform(*this, componentName);
  }
  //这个函数返回一个std::vector<geom::Transform>类型的值，代表Actor中所有骨骼的世界变换（World Transforms）。
  std::vector<geom::Transform> Actor::GetBoneWorldTransforms() const {
    return GetEpisode().Lock()->GetActorBoneWorldTransforms(*this);
  }
  //与GetBoneWorldTransforms类似，这个函数返回Actor中所有骨骼的相对变换（Relative Transforms）
  std::vector<geom::Transform> Actor::GetBoneRelativeTransforms() const {
    return GetEpisode().Lock()->GetActorBoneRelativeTransforms(*this);
  }
  //这个函数返回一个std::vector<std::string>类型的值，包含Actor中所有组件的名称。
  std::vector<std::string> Actor::GetComponentNames() const {
    return GetEpisode().Lock()->GetActorComponentNames(*this);
  }
  //这个函数返回一个std::vector<std::string>类型的值，包含Actor中所有骨骼的名称
  std::vector<std::string> Actor::GetBoneNames() const {
    return GetEpisode().Lock()->GetActorBoneNames(*this);
  } 
  //这个函数返回一个std::vector<geom::Transform>类型的值，代表Actor中所有插槽（Sockets）的世界变换
  std::vector<geom::Transform> Actor::GetSocketWorldTransforms() const {
    return GetEpisode().Lock()->GetActorSocketWorldTransforms(*this);
  }
  //与GetSocketWorldTransforms类似，这个函数返回Actor中所有插槽的相对变换
  std::vector<geom::Transform> Actor::GetSocketRelativeTransforms() const {
    return GetEpisode().Lock()->GetActorSocketRelativeTransforms(*this);
  }
  //这个函数返回一个std::vector<std::string>类型的值，包含Actor中所有插槽的名称。
  std::vector<std::string> Actor::GetSocketNames() const {
    return GetEpisode().Lock()->GetActorSocketNames(*this);
  }  
/**
 * 设置Actor的位置。
 * @param location 新的位置信息。
 */
  void Actor::SetLocation(const geom::Location &location) {
    GetEpisode().Lock()->SetActorLocation(*this, location);
  }
/**
 * 设置Actor的变换，包括位置、旋转和缩放。
 * @param transform 新的变换信息。
 */
  void Actor::SetTransform(const geom::Transform &transform) {
    GetEpisode().Lock()->SetActorTransform(*this, transform);
  }
/**
 * 设置Actor的目标速度。
 * @param vector 目标速度向量。
 */
  void Actor::SetTargetVelocity(const geom::Vector3D &vector) {
    GetEpisode().Lock()->SetActorTargetVelocity(*this, vector);
  }
/**
 * 设置Actor的目标角速度。
 * @param vector 目标角速度向量。
 */
  void Actor::SetTargetAngularVelocity(const geom::Vector3D &vector) {
    GetEpisode().Lock()->SetActorTargetAngularVelocity(*this, vector);
  }
/**
 * 启用Actor的恒定速度。
 * @param vector 恒定速度向量。
 */
  void Actor::EnableConstantVelocity(const geom::Vector3D &vector) {
    GetEpisode().Lock()->EnableActorConstantVelocity(*this, vector);
  }
/**
 * 禁用Actor的恒定速度。
 */
  void Actor::DisableConstantVelocity() {
    GetEpisode().Lock()->DisableActorConstantVelocity(*this);
  }
/**
 * 向Actor添加冲量（瞬间力）。
 * @param impulse 冲量向量。
 */
  void Actor::AddImpulse(const geom::Vector3D &impulse) {
    GetEpisode().Lock()->AddActorImpulse(*this, impulse);
  }
/**
 * 向Actor在特定位置添加冲量（瞬间力）。
 * @param impulse 冲量向量。
 * @param location 冲量作用的位置。
 */
  void Actor::AddImpulse(const geom::Vector3D &impulse, const geom::Vector3D &location) {
    GetEpisode().Lock()->AddActorImpulse(*this, impulse, location);
  }
/**
 * 向Actor在特定位置添加冲量（瞬间力）。
 * @param impulse 冲量向量。
 * @param location 冲量作用的位置。
 */
  void Actor::AddForce(const geom::Vector3D &force) {
    GetEpisode().Lock()->AddActorForce(*this, force);
  }
/**
 * 向Actor在特定位置添加力。
 * @param force 力向量。
 * @param location 力作用的位置。
 */
  void Actor::AddForce(const geom::Vector3D &force, const geom::Vector3D &location) {
    GetEpisode().Lock()->AddActorForce(*this, force, location);
  }
/**
 * 向Actor添加角冲量。
 * @param vector 角冲量向量。
 */
  void Actor::AddAngularImpulse(const geom::Vector3D &vector) {
    GetEpisode().Lock()->AddActorAngularImpulse(*this, vector);
  }
/**
 * 向Actor添加扭矩。
 * @param torque 扭矩向量。
 */
  void Actor::AddTorque(const geom::Vector3D &torque) {
    GetEpisode().Lock()->AddActorTorque(*this, torque);
  }
/**
 * 设置Actor是否模拟物理。
 * @param enabled 是否启用物理模拟。
 */
  void Actor::SetSimulatePhysics(const bool enabled) {
    GetEpisode().Lock()->SetActorSimulatePhysics(*this, enabled);
  }
/**
 * 设置Actor是否启用碰撞。
 * @param enabled 是否启用碰撞。
 */
  void Actor::SetCollisions(const bool enabled) {
    GetEpisode().Lock()->SetActorCollisions(*this, enabled);
  }
/**
 * 标记Actor为死亡状态。
 */
  void Actor::SetActorDead() {
    GetEpisode().Lock()->SetActorDead(*this);
  }
/**
 * 设置Actor是否启用重力。
 * @param enabled 是否启用重力。
 */
  void Actor::SetEnableGravity(const bool enabled) {
    GetEpisode().Lock()->SetActorEnableGravity(*this, enabled);
  }

/**
 * 获取Actor的当前状态。
 * @return Actor的当前状态。
 */
  rpc::ActorState Actor::GetActorState() const {
    return GetEpisode().Lock()->GetActorState(*this);
  }
  // 定义在Actor类中的Destroy成员函数，该函数返回一个布尔值，表示销毁操作是否成功
  bool Actor::Destroy() {
      // 调用GetActorState成员函数获取当前Actor的状态，并将其存储在actor_state变量中
      rpc::ActorState actor_state = GetActorState();
      // 定义一个布尔变量result，初始化为false，用于记录销毁操作的结果
      bool result = false;
      // 判断actor_state是否不等于rpc::ActorState枚举中的Invalid状态。  
      // 如果不等于Invalid，说明该Actor对象是一个有效的对象，可以进行销毁操作
      if (actor_state != rpc::ActorState::Invalid) {
      result = GetEpisode().Lock()->DestroyActor(*this);
    } else {
      log_warning(
          "attempting to destroy an actor that is already dead:",
          GetDisplayId());
    }
    // 返回销毁操作的结果
    return result;
  }

} // 命名空间client方便区分函数
} // 命名空间carla方便区分函数
