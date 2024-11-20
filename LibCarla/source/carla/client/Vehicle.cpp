// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/client/Vehicle.h"

#include "carla/client/ActorList.h"
#include "carla/client/detail/Simulator.h"
#include "carla/client/TrafficLight.h"
#include "carla/Memory.h"
#include "carla/rpc/TrafficLightState.h"

#include "carla/trafficmanager/TrafficManager.h"

namespace carla {

using TM = traffic_manager::TrafficManager;

namespace client {

// 模板函数，用于从属性列表中获取"sticky_control"属性的值
  template <typename AttributesT>
  static bool GetControlIsSticky(const AttributesT &attributes) {
    for (auto &&attribute : attributes) {
    	// 检查属性 ID 是否为"sticky_control"
      if (attribute.GetId() == "sticky_control") {
        return attribute.template As<bool>();
      }
    }
    // 如果没有找到"sticky_control"属性，默认返回 true
    return true;
  }
// 车辆类的构造函数
  Vehicle::Vehicle(ActorInitializer init)
    : Actor(std::move(init)),
      _is_control_sticky(GetControlIsSticky(GetAttributes())) {}
// 设置车辆的自动驾驶状态
  void Vehicle::SetAutopilot(bool enabled, uint16_t tm_port) {
    TM tm(GetEpisode(), tm_port);
    if (enabled) {
    	// 如果启用自动驾驶，将车辆注册到交通管理器
      tm.RegisterVehicles({shared_from_this()});
    } else {
    	// 如果禁用自动驾驶，将车辆从交通管理器中注销
      tm.UnregisterVehicles({shared_from_this()});
    }
  }
// 获取车辆的遥测数据
  Vehicle::TelemetryData Vehicle::GetTelemetryData() const {
    return GetEpisode().Lock()->GetVehicleTelemetryData(*this);
  }
// 显示或隐藏车辆的调试遥测信息
  void Vehicle::ShowDebugTelemetry(bool enabled) {
    GetEpisode().Lock()->ShowVehicleDebugTelemetry(*this, enabled);
  }
// 应用车辆控制指令
  void Vehicle::ApplyControl(const Control &control) {
  	// 如果控制指令不是粘性的或者与当前控制指令不同，则应用新的控制指令
    if (!_is_control_sticky || (control != _control)) {
      GetEpisode().Lock()->ApplyControlToVehicle(*this, control);
      _control = control;
    }
  }
// 应用阿克曼控制指令
  void Vehicle::ApplyAckermannControl(const AckermannControl &control) {
    GetEpisode().Lock()->ApplyAckermannControlToVehicle(*this, control);
  }
// 获取阿克曼控制器设置
  rpc::AckermannControllerSettings Vehicle::GetAckermannControllerSettings() const {
    return GetEpisode().Lock()->GetAckermannControllerSettings(*this);
  }
// 应用阿克曼控制器设置
  void Vehicle::ApplyAckermannControllerSettings(const rpc::AckermannControllerSettings &settings) {
    GetEpisode().Lock()->ApplyAckermannControllerSettings(*this, settings);
  }
// 应用车辆物理控制指令
  void Vehicle::ApplyPhysicsControl(const PhysicsControl &physics_control) {
    GetEpisode().Lock()->ApplyPhysicsControlToVehicle(*this, physics_control);
  }
// 打开车辆的指定门
  void Vehicle::OpenDoor(const VehicleDoor door_idx) {
    GetEpisode().Lock()->OpenVehicleDoor(*this, rpc::VehicleDoor(door_idx));
  }
// 关闭车辆的指定门
  void Vehicle::CloseDoor(const VehicleDoor door_idx) {
    GetEpisode().Lock()->CloseVehicleDoor(*this, rpc::VehicleDoor(door_idx));
  }
// 设置车辆的灯光状态
  void Vehicle::SetLightState(const LightState &light_state) {
    GetEpisode().Lock()->SetLightStateToVehicle(*this, rpc::VehicleLightState(light_state));
  }
// 设置车辆指定轮子的转向角度
  void Vehicle::SetWheelSteerDirection(WheelLocation wheel_location, float angle_in_deg) {
    GetEpisode().Lock()->SetWheelSteerDirection(*this, wheel_location, angle_in_deg);
  }
// 获取车辆指定轮子的转向角度
  float Vehicle::GetWheelSteerAngle(WheelLocation wheel_location) {
    return GetEpisode().Lock()->GetWheelSteerAngle(*this, wheel_location);
  }
// 获取车辆当前的控制指令
  Vehicle::Control Vehicle::GetControl() const {
    return GetEpisode().Lock()->GetActorSnapshot(*this).state.vehicle_data.control;
  }
// 获取车辆当前的物理控制指令
  Vehicle::PhysicsControl Vehicle::GetPhysicsControl() const {
    return GetEpisode().Lock()->GetVehiclePhysicsControl(*this);
  }
// 获取车辆当前的灯光状态
  Vehicle::LightState Vehicle::GetLightState() const {
    return GetEpisode().Lock()->GetVehicleLightState(*this).GetLightStateEnum();
  }
// 获取车辆当前的速度限制
  float Vehicle::GetSpeedLimit() const {
    return GetEpisode().Lock()->GetActorSnapshot(*this).state.vehicle_data.speed_limit;
  }
// 获取车辆当前所处交通灯的状态
  rpc::TrafficLightState Vehicle::GetTrafficLightState() const {
    return GetEpisode().Lock()->GetActorSnapshot(*this).state.vehicle_data.traffic_light_state;
  }
// 判断车辆是否在交通灯下
  bool Vehicle::IsAtTrafficLight() {
    return GetEpisode().Lock()->GetActorSnapshot(*this).state.vehicle_data.has_traffic_light;
  }
// 获取车辆当前所处的交通灯
  SharedPtr<TrafficLight> Vehicle::GetTrafficLight() const {
    auto id = GetEpisode().Lock()->GetActorSnapshot(*this).state.vehicle_data.traffic_light_id;
    return boost::static_pointer_cast<TrafficLight>(GetWorld().GetActor(id));
  }
// 启用车辆模拟（CarSim）
  void Vehicle::EnableCarSim(std::string simfile_path) {
    GetEpisode().Lock()->EnableCarSim(*this, simfile_path);
  }
// 设置是否使用车辆模拟的道路
  void Vehicle::UseCarSimRoad(bool enabled) {
    GetEpisode().Lock()->UseCarSimRoad(*this, enabled);
  }
// 启用 Chrono 物理引擎
  void Vehicle::EnableChronoPhysics(
      uint64_t MaxSubsteps,
      float MaxSubstepDeltaTime,
      std::string VehicleJSON,
      std::string PowertrainJSON,
      std::string TireJSON,
      std::string BaseJSONPath) {
    GetEpisode().Lock()->EnableChronoPhysics(*this,
        MaxSubsteps,
        MaxSubstepDeltaTime,
        VehicleJSON,
        PowertrainJSON,
        TireJSON,
        BaseJSONPath);
  }
// 恢复 PhysX 物理引擎
  void Vehicle::RestorePhysXPhysics() {
    GetEpisode().Lock()->RestorePhysXPhysics(*this);
  }
// 获取车辆的故障状态
  rpc::VehicleFailureState Vehicle::GetFailureState() const {
    return GetEpisode().Lock()->GetActorSnapshot(*this).state.vehicle_data.failure_state;
  }

} // namespace client
} // namespace carla
