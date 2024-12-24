// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

// 包含Carla客户端中Actor相关的头文件
// Actor在Carla模拟环境里代表各种参与模拟的实体对象，例如车辆、行人、交通标志等
// 该头文件提供了创建、操作以及获取Actor相关属性的类、接口等相关定义
#include "carla/client/Actor.h" 

// 包含用于定义阿克曼转向控制器（Ackermann Controller）相关设置的头文件
// 阿克曼转向常用于车辆等的转向控制模型，此头文件中可能包含如转向角度限制、控制增益等相关设置的定义
// 方便配置相应的控制逻辑，以实现基于阿克曼转向原理的车辆操控
#include "carla/rpc/AckermannControllerSettings.h" 

// 定义交通信号灯状态相关的数据结构、枚举等内容的头文件
// 例如包含了红灯、绿灯、黄灯等不同状态的表示方式
// 也可能提供一些用于判断、转换交通灯状态的辅助函数或方法，有助于在模拟交通场景中处理与交通灯相关的交互逻辑
#include "carla/rpc/TrafficLightState.h" 

// 与基于阿克曼转向模型对车辆进行控制相关的头文件
// 声明了发送给车辆使其按照阿克曼转向原理进行运动控制（像控制车辆的转向、速度等）的结构体、类或者相关函数接口
// 方便在代码里实现对车辆具体的操控逻辑，基于阿克曼转向来模拟车辆行驶行为
#include "carla/rpc/VehicleAckermannControl.h" 

// 针对车辆控制方面更通用的头文件
// 包含了如车辆的油门、刹车、转向等基础控制操作对应的结构体定义、控制命令的封装等内容
// 通过使用此头文件里的定义，可以实现基本的车辆驾驶行为模拟，对车辆进行常规操作控制
#include "carla/rpc/VehicleControl.h" 

// 涉及车辆门相关操作和属性的定义的头文件
// 例如定义了车门的打开、关闭状态表示，以及可能存在控制车门开关的函数接口声明等
// 在模拟诸如乘车、下车等涉及车辆门状态改变的场景时会用到里面的相关功能
#include "carla/rpc/VehicleDoor.h" 

// 用来定义车辆灯光状态的头文件
// 像车辆的大灯（远光、近光）、转向灯、刹车灯等不同灯光的亮灭状态、组合状态等相关的数据结构都在这里定义
// 同时可能提供用于设置、查询车辆灯光状态的接口声明，有助于模拟车辆在不同行驶场景下的灯光使用情况
#include "carla/rpc/VehicleLightState.h" 

// 侧重于车辆物理特性控制方面的头文件
// 包含对车辆的质量、惯性、摩擦力等物理参数进行设置和获取的相关类、结构体定义以及对应的操作函数接口声明等内容
// 借助这些定义能够更精细地模拟车辆在物理环境中的运动和行为表现，实现更逼真的车辆物理模拟
#include "carla/rpc/VehiclePhysicsControl.h" 

// 定义了用于获取和表示车辆各种遥测数据的头文件（车辆遥测数据）
// 例如车辆的实时速度、位置坐标、加速度、航向角等信息的结构体或类在此定义
// 对于分析车辆的行驶状态、记录行驶轨迹以及基于车辆实时状态进行更复杂的控制决策等方面非常有用
#include "carla/rpc/VehicleTelemetryData.h"  // 车辆遥测数据

// 主要涉及车辆车轮相关内容的头文件
// 像车轮的转速、转向角度、轮胎压力等属性的定义会在这里出现
// 可能还有针对车轮单独控制或者获取其状态的函数接口声明，在模拟车辆复杂的运动、处理车轮相关故障等场景下能发挥作用
#include "carla/rpc/VehicleWheels.h" 

// 包含和Carla中交通管理相关的头文件
// 可能包含用于管理交通流量、设置交通规则、协调不同车辆和交通参与者之间交互关系的类定义和函数接口声明
// 例如可以用于设置车辆速度限制、生成交通流等功能，以构建更逼真的交通模拟场景
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
    /// @note 此函数不调用模拟器.
    /// 在最后一个节拍返回接收到的数据.
    Control GetControl() const;

    /// 返回车辆最后应用的物理控制.
    ///
    /// @warning 此函数调用模拟器.
    PhysicsControl GetPhysicsControl() const;

    /// 返回当前车辆的打开灯(LightState).
    ///
    /// @note 此函数不调用模拟器.
    /// 在最后一个节拍返回接收到的数据.
    LightState GetLightState() const;

    /// 返回当前影响该车辆的速度限制.
    ///
    /// @note 此函数不调用模拟器.
    /// 在最后一个节拍返回接收到的数据.
    float GetSpeedLimit() const;

    /// 返回当前影响该车辆的交通灯的状态.
    ///
    /// @return 绿灯表示车辆没有受到交通信号灯的影响.
    ///
    /// @note 此函数不调用模拟器.
    /// 在最后一个节拍返回接收到的数据.
    rpc::TrafficLightState GetTrafficLightState() const;

    /// 返回交通灯是否正在影响该车辆.
    ///
    /// @note 此函数不调用模拟器.
    /// 在最后一个节拍返回接收到的数据.
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
