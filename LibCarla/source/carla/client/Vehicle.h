// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/client/Actor.h"
#include "carla/rpc/AckermannControllerSettings.h"
#include "carla/rpc/TrafficLightState.h"
#include "carla/rpc/VehicleAckermannControl.h"
#include "carla/rpc/VehicleControl.h"
#include "carla/rpc/VehicleDoor.h"
#include "carla/rpc/VehicleLightState.h"
#include "carla/rpc/VehiclePhysicsControl.h"
#include "carla/rpc/VehicleTelemetryData.h"
#include "carla/rpc/VehicleWheels.h"
#include "carla/trafficmanager/TrafficManager.h"

using carla::traffic_manager::constants::Networking::TM_DEFAULT_PORT;

namespace carla {

namespace traffic_manager {
  class TrafficManager;
}

namespace client {

  class TrafficLight;

  class Vehicle : public Actor {
  public:

    using Control = rpc::VehicleControl;
    using AckermannControl = rpc::VehicleAckermannControl;
    using PhysicsControl = rpc::VehiclePhysicsControl;
    using TelemetryData = rpc::VehicleTelemetryData;
    using LightState = rpc::VehicleLightState::LightState;
    using TM = traffic_manager::TrafficManager;
    using VehicleDoor = rpc::VehicleDoor;
    using WheelLocation = carla::rpc::VehicleWheelLocation;


    explicit Vehicle(ActorInitializer init);

    /// 开关车辆的自动驾驶仪.
    void SetAutopilot(bool enabled = true, uint16_t tm_port = TM_DEFAULT_PORT);

    /// 返回车辆的遥测数据.
    ///
    /// @warning 此函数调用模拟器.
    TelemetryData GetTelemetryData() const;

    /// 开关车辆的自动驾驶仪.
    void ShowDebugTelemetry(bool enabled = true);

    /// 应用 @a 控制此车辆.
    void ApplyControl(const Control &control);

    /// 应用 @a 控制此车辆.
    void ApplyAckermannControl(const AckermannControl &control);

    /// 返回最后应用于车辆的 Ackermann controller 设置.
    ///
    /// @warning 此函数调用模拟器.
    rpc::AckermannControllerSettings GetAckermannControllerSettings() const;

    /// 应用 Ackermann 控制设置给此车辆
    void ApplyAckermannControllerSettings(const rpc::AckermannControllerSettings &settings);

    /// 给此车辆应用物理控制.
    void ApplyPhysicsControl(const PhysicsControl &physics_control);

    /// 在车中开门
    void OpenDoor(const VehicleDoor door_idx);

    /// 在车中关门
    void CloseDoor(const VehicleDoor door_idx);

    /// 给车辆设置一个 @a LightState.
    void SetLightState(const LightState &light_state);

    /// 给车辆轮子一个 @a 旋转 (影响汽车的骨骼框架,而不是物理)
    void SetWheelSteerDirection(WheelLocation wheel_location, float angle_in_deg);

    /// 从车辆轮子返回一个 @a 旋转
    ///
    /// @note 基于轮子的物理，此函数返回它的旋转
    float GetWheelSteerAngle(WheelLocation wheel_location);

    /// 返回最后应用于车辆的控制.
    ///
    /// @note 此函数不调用模拟器,它返回数据
    /// 接收最后一个节拍.
    Control GetControl() const;

    /// 返回车辆最后应用的物理控制.
    ///
    /// @warning 此函数调用模拟器.
    PhysicsControl GetPhysicsControl() const;

    /// 返回当前车辆的打开灯(LightState).
    ///
    /// @note 此函数不调用模拟器,它返回数据
    /// 接收最后一个节拍.
    LightState GetLightState() const;

    /// 返回当前影响该车辆的速度限制.
    ///
    /// @note 此函数不调用模拟器,它返回数据
    /// 接收最后一个节拍.
    float GetSpeedLimit() const;

    /// 返回当前影响该车辆的交通灯的状态.
    ///
    /// @return 绿灯表示车辆没有受到交通信号灯的影响.
    ///
    /// @note 此函数不调用模拟器,它返回数据
    /// 接收最后一个节拍.
    rpc::TrafficLightState GetTrafficLightState() const;

    /// 返回交通灯是否正在影响该车辆.
    ///
    /// @note 此函数不调用模拟器,它返回数据
    /// 接收最后一个节拍.
    bool IsAtTrafficLight();

    /// 检索当前影响该车辆的交通灯.
    SharedPtr<TrafficLight> GetTrafficLight() const;

    /// 如果可用，启用CarSim模拟
    void EnableCarSim(std::string simfile_path);

    /// 允许使用CarSim内部道路定义，而不是虚幻引擎的
    void UseCarSimRoad(bool enabled);

    void EnableChronoPhysics(
        uint64_t MaxSubsteps,
        float MaxSubstepDeltaTime,
        std::string VehicleJSON = "",
        std::string PowertrainJSON = "",
        std::string TireJSON = "",
        std::string BaseJSONPath = "");

    void RestorePhysXPhysics();

    /// 返回车辆的故障状态
    rpc::VehicleFailureState GetFailureState() const;

  private:

    const bool _is_control_sticky;

    Control _control;
  };

} // namespace client
} // namespace carla
