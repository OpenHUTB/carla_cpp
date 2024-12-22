// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/client/Vehicle.h" // 引入控制和管理车辆的相关类定义

#include "carla/client/ActorList.h" // 引入Actor列表相关的类，通常是用于访问仿真中的所有角色（例如车辆、行人等）
#include "carla/client/detail/Simulator.h" // 引入Simulator的实现细节，用于模拟环境的控制
#include "carla/client/TrafficLight.h" // 引入交通信号灯相关的类定义，允许控制和查询交通灯的状态
#include "carla/Memory.h" // 引入内存管理相关的定义，通常用于资源管理和垃圾回收
#include "carla/rpc/TrafficLightState.h" // 引入交通灯状态的定义，以便管理交通信号灯的不同状态（如红灯、绿灯等）

#include "carla/trafficmanager/TrafficManager.h" // 引入交通管理器的定义，允许对整个城市中的交通进行全局管理和控制

namespace carla {

using TM = traffic_manager::TrafficManager;

namespace client {

  // 模板函数：从属性列表中获取“sticky_control”属性的值
  // 该属性指示是否启用了粘性控制（即控制指令是否会持续应用）
  template <typename AttributesT>
  static bool GetControlIsSticky(const AttributesT &attributes) {
    for (auto &&attribute : attributes) {
    	// 查找属性 ID 为"sticky_control"的属性
      if (attribute.GetId() == "sticky_control") {
        return attribute.template As<bool>(); // 返回该属性值（布尔类型）
      }
    }
    // 如果未找到"sticky_control"属性，则默认返回 true
    return true;
  }
  // 车辆类的构造函数
  // 初始化时获取车辆的“sticky_control”属性，决定是否启用粘性控制
  Vehicle::Vehicle(ActorInitializer init)
    : Actor(std::move(init)),
      _is_control_sticky(GetControlIsSticky(GetAttributes())) {}
  // 设置车辆的自动驾驶状态
  // 启用或禁用自动驾驶，并在交通管理器中注册或注销该车辆
  void Vehicle::SetAutopilot(bool enabled, uint16_t tm_port) {
    TM tm(GetEpisode(), tm_port);
    if (enabled) {
    	// 启用自动驾驶，车辆被注册到交通管理器
      tm.RegisterVehicles({shared_from_this()});
    } else {
    	// 禁用自动驾驶，车辆从交通管理器中注销
      tm.UnregisterVehicles({shared_from_this()});
    }
  }
  // 获取车辆的遥测数据
  // 返回车辆的最新遥测数据，包括速度、位置等
  Vehicle::TelemetryData Vehicle::GetTelemetryData() const {
    return GetEpisode().Lock()->GetVehicleTelemetryData(*this);
  }
  // 显示或隐藏车辆的调试遥测信息
  // 根据`enabled`参数来控制遥测信息是否显示
  void Vehicle::ShowDebugTelemetry(bool enabled) {
    GetEpisode().Lock()->ShowVehicleDebugTelemetry(*this, enabled);
  }
  // 应用车辆控制指令
  // 如果控制指令不是粘性的，或者与当前指令不同，才应用新的控制指令
  void Vehicle::ApplyControl(const Control &control) {
  	// 如果控制指令不是粘性的或者与当前控制指令不同，则应用新的控制指令
    if (!_is_control_sticky || (control != _control)) {
      GetEpisode().Lock()->ApplyControlToVehicle(*this, control);
      _control = control;
    }
  }
  // 应用阿克曼控制指令
  // 使用阿克曼转向模型来控制车辆的转向
  void Vehicle::ApplyAckermannControl(const AckermannControl &control) {
    GetEpisode().Lock()->ApplyAckermannControlToVehicle(*this, control);
  }
  // 获取阿克曼控制器的设置
  // 返回当前的阿克曼控制器设置，包括转向角度等
  rpc::AckermannControllerSettings Vehicle::GetAckermannControllerSettings() const {
    return GetEpisode().Lock()->GetAckermannControllerSettings(*this);
  }
  // 应用新的阿克曼控制器设置
  // 设置阿克曼控制器的相关参数（如转向参数）
  void Vehicle::ApplyAckermannControllerSettings(const rpc::AckermannControllerSettings &settings) {
    GetEpisode().Lock()->ApplyAckermannControllerSettings(*this, settings);
  }
  // 应用车辆的物理控制指令
  // 使用物理控制模型调整车辆的物理行为（例如速度、加速度等）
  void Vehicle::ApplyPhysicsControl(const PhysicsControl &physics_control) {
    GetEpisode().Lock()->ApplyPhysicsControlToVehicle(*this, physics_control);
  }
  // 打开指定的车辆门
  // `door_idx`指示需要打开的车辆门（例如：前左门、后右门等）
  void Vehicle::OpenDoor(const VehicleDoor door_idx) {
    GetEpisode().Lock()->OpenVehicleDoor(*this, rpc::VehicleDoor(door_idx));
  }
  // 关闭指定的车辆门
  // `door_idx`指示需要关闭的车辆门
  void Vehicle::CloseDoor(const VehicleDoor door_idx) {
    GetEpisode().Lock()->CloseVehicleDoor(*this, rpc::VehicleDoor(door_idx));
  }
  // 设置车辆的灯光状态
  // 设置车辆的各类灯光（如远光灯、近光灯等）的状态
  void Vehicle::SetLightState(const LightState &light_state) {
    GetEpisode().Lock()->SetLightStateToVehicle(*this, rpc::VehicleLightState(light_state));
  }
  // 设置指定轮子的转向角度
  // `wheel_location`表示轮子的位置，`angle_in_deg`表示转向角度（单位：度）
  void Vehicle::SetWheelSteerDirection(WheelLocation wheel_location, float angle_in_deg) {
    GetEpisode().Lock()->SetWheelSteerDirection(*this, wheel_location, angle_in_deg);
  }
  // 获取指定轮子的转向角度
  // 返回指定轮子的当前转向角度（单位：度）
  float Vehicle::GetWheelSteerAngle(WheelLocation wheel_location) {
    return GetEpisode().Lock()->GetWheelSteerAngle(*this, wheel_location);
  }
  // 获取车辆当前的控制指令
  // 返回当前应用于车辆的控制指令（如加速、刹车、转向等）
  Vehicle::Control Vehicle::GetControl() const {
    return GetEpisode().Lock()->GetActorSnapshot(*this).state.vehicle_data.control;
  }
  // 获取车辆的物理控制指令
  // 返回当前车辆的物理控制状态（如速度、加速度等）
  Vehicle::PhysicsControl Vehicle::GetPhysicsControl() const {
    return GetEpisode().Lock()->GetVehiclePhysicsControl(*this);
  }
  // 获取车辆的灯光状态
  // 返回车辆的灯光状态（如是否开启远光灯、近光灯等）
  Vehicle::LightState Vehicle::GetLightState() const {
    return GetEpisode().Lock()->GetVehicleLightState(*this).GetLightStateEnum();
  }
  // 获取车辆的当前速度限制
  // 返回车辆所受的速度限制（单位：米/秒）
  float Vehicle::GetSpeedLimit() const {
    return GetEpisode().Lock()->GetActorSnapshot(*this).state.vehicle_data.speed_limit;
  }
  // 获取车辆当前所处交通灯的状态
  // 返回当前交通灯的状态（例如：红灯、绿灯等）
  rpc::TrafficLightState Vehicle::GetTrafficLightState() const {
    return GetEpisode().Lock()->GetActorSnapshot(*this).state.vehicle_data.traffic_light_state;
  }
  // 判断车辆是否处于交通灯下
  // 返回 true 如果车辆当前处于交通灯位置
  bool Vehicle::IsAtTrafficLight() {
    return GetEpisode().Lock()->GetActorSnapshot(*this).state.vehicle_data.has_traffic_light;
  }
  // 获取车辆当前所处的交通灯
  // 返回指向交通灯对象的指针，若没有交通灯则返回 nullptr
  SharedPtr<TrafficLight> Vehicle::GetTrafficLight() const {
    auto id = GetEpisode().Lock()->GetActorSnapshot(*this).state.vehicle_data.traffic_light_id;
    return boost::static_pointer_cast<TrafficLight>(GetWorld().GetActor(id));
  }
  // 启用车辆模拟（CarSim）
  // 使用指定的模拟文件路径启用车辆模拟
  void Vehicle::EnableCarSim(std::string simfile_path) {
    GetEpisode().Lock()->EnableCarSim(*this, simfile_path);// 启动车辆模拟功能
  }
  // 设置是否使用车辆模拟的道路
  // 根据 `enabled` 参数启用或禁用模拟的道路
  void Vehicle::UseCarSimRoad(bool enabled) {
    GetEpisode().Lock()->UseCarSimRoad(*this, enabled);
  }
  // 启用 Chrono 物理引擎
  // 设置 Chrono 物理引擎的相关参数（如最大子步数、时间步长等）
// 启用Chrono 物理引擎，并设置相关参数
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
  // 将车辆的物理引擎恢复为默认的 PhysX 引擎
  void Vehicle::RestorePhysXPhysics() {
    GetEpisode().Lock()->RestorePhysXPhysics(*this);
  }
  // 获取车辆的故障状态
  // 返回车辆的故障状态，包括是否存在故障、故障类型等信息
  rpc::VehicleFailureState Vehicle::GetFailureState() const {
    return GetEpisode().Lock()->GetActorSnapshot(*this).state.vehicle_data.failure_state;
  }

} // namespace client
} // namespace carla
