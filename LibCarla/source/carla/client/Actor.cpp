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

  std::vector<geom::Transform> Actor::GetBoneWorldTransforms() const {
    return GetEpisode().Lock()->GetActorBoneWorldTransforms(*this);
  }

  std::vector<geom::Transform> Actor::GetBoneRelativeTransforms() const {
    return GetEpisode().Lock()->GetActorBoneRelativeTransforms(*this);
  }

  std::vector<std::string> Actor::GetComponentNames() const {
    return GetEpisode().Lock()->GetActorComponentNames(*this);
  }

  std::vector<std::string> Actor::GetBoneNames() const {
    return GetEpisode().Lock()->GetActorBoneNames(*this);
  } 

  std::vector<geom::Transform> Actor::GetSocketWorldTransforms() const {
    return GetEpisode().Lock()->GetActorSocketWorldTransforms(*this);
  }

  std::vector<geom::Transform> Actor::GetSocketRelativeTransforms() const {
    return GetEpisode().Lock()->GetActorSocketRelativeTransforms(*this);
  }

  std::vector<std::string> Actor::GetSocketNames() const {
    return GetEpisode().Lock()->GetActorSocketNames(*this);
  }  

  void Actor::SetLocation(const geom::Location &location) {
    GetEpisode().Lock()->SetActorLocation(*this, location);
  }

  void Actor::SetTransform(const geom::Transform &transform) {
    GetEpisode().Lock()->SetActorTransform(*this, transform);
  }

  void Actor::SetTargetVelocity(const geom::Vector3D &vector) {
    GetEpisode().Lock()->SetActorTargetVelocity(*this, vector);
  }

  void Actor::SetTargetAngularVelocity(const geom::Vector3D &vector) {
    GetEpisode().Lock()->SetActorTargetAngularVelocity(*this, vector);
  }

  void Actor::EnableConstantVelocity(const geom::Vector3D &vector) {
    GetEpisode().Lock()->EnableActorConstantVelocity(*this, vector);
  }

  void Actor::DisableConstantVelocity() {
    GetEpisode().Lock()->DisableActorConstantVelocity(*this);
  }

  void Actor::AddImpulse(const geom::Vector3D &impulse) {
    GetEpisode().Lock()->AddActorImpulse(*this, impulse);
  }

  void Actor::AddImpulse(const geom::Vector3D &impulse, const geom::Vector3D &location) {
    GetEpisode().Lock()->AddActorImpulse(*this, impulse, location);
  }

  void Actor::AddForce(const geom::Vector3D &force) {
    GetEpisode().Lock()->AddActorForce(*this, force);
  }

  void Actor::AddForce(const geom::Vector3D &force, const geom::Vector3D &location) {
    GetEpisode().Lock()->AddActorForce(*this, force, location);
  }

  void Actor::AddAngularImpulse(const geom::Vector3D &vector) {
    GetEpisode().Lock()->AddActorAngularImpulse(*this, vector);
  }

  void Actor::AddTorque(const geom::Vector3D &torque) {
    GetEpisode().Lock()->AddActorTorque(*this, torque);
  }

  void Actor::SetSimulatePhysics(const bool enabled) {
    GetEpisode().Lock()->SetActorSimulatePhysics(*this, enabled);
  }

  void Actor::SetCollisions(const bool enabled) {
    GetEpisode().Lock()->SetActorCollisions(*this, enabled);
  }

  void Actor::SetActorDead() {
    GetEpisode().Lock()->SetActorDead(*this);
  }

  void Actor::SetEnableGravity(const bool enabled) {
    GetEpisode().Lock()->SetActorEnableGravity(*this, enabled);
  }

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
