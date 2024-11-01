// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/geom/Transform.h"
#include "carla/geom/Vector3D.h"
#include "carla/rpc/ActorId.h"
#include "carla/rpc/ActorState.h"
#include "carla/rpc/VehicleFailureState.h"
#include "carla/rpc/TrafficLightState.h"
#include "carla/rpc/VehicleControl.h"
#include "carla/rpc/WalkerControl.h"

#include <cstdint>

namespace carla { // 定义命名空间 carla
namespace sensor { // 定义命名空间 sensor
namespace data { // 定义命名空间 data

namespace detail { // 定义命名空间 detail

#pragma pack(push, 1) // 开始 1 字节对齐
  class PackedVehicleControl { // 定义 PackedVehicleControl 类
  public:

    PackedVehicleControl() = default; // 默认构造函数

    PackedVehicleControl(const rpc::VehicleControl &control) // 带参数的构造函数
      : throttle(control.throttle), // 初始化油门
        steer(control.steer), // 初始化转向
        brake(control.brake), // 初始化刹车
        hand_brake(control.hand_brake), // 初始化手刹
        reverse(control.reverse), // 初始化倒车
        manual_gear_shift(control.manual_gear_shift), // 初始化手动换档
        gear(control.gear) {} // 初始化档位

    operator rpc::VehicleControl() const { // 转换操作符，将其转换为 rpc::VehicleControl
      return {throttle, steer, brake, hand_brake, reverse, manual_gear_shift, gear}; // 返回对应的 rpc::VehicleControl 对象
    }

  private:

    float throttle; // 油门
    float steer; // 转向
    float brake; // 刹车
    bool hand_brake; // 手刹
    bool reverse; // 倒车
    bool manual_gear_shift; // 手动换档
    int32_t gear; // 档位
  };

#pragma pack(pop) // 恢复对齐方式

#pragma pack(push, 1) // 开始 1 字节对齐
  struct VehicleData { // 定义 VehicleData 结构体
    VehicleData() = default; // 默认构造函数

    PackedVehicleControl control; // 控制信息
    float speed_limit; // 速度限制
    rpc::TrafficLightState traffic_light_state; // 交通灯状态
    bool has_traffic_light; // 是否有交通灯
    rpc::ActorId traffic_light_id; // 交通灯 ID
    rpc::VehicleFailureState failure_state; // 车辆故障状态
  };
#pragma pack(pop) // 恢复对齐方式

#pragma pack(push, 1) // 开始 1 字节对齐
  class PackedWalkerControl { // 定义 PackedWalkerControl 类
  public:

    PackedWalkerControl() = default; // 默认构造函数

    PackedWalkerControl(const rpc::WalkerControl &control) // 带参数的构造函数
      : direction{control.direction.x, control.direction.y, control.direction.z}, // 初始化方向
        speed(control.speed), // 初始化速度
        jump(control.jump) {} // 初始化跳跃状态

    operator rpc::WalkerControl() const { // 转换操作符，将其转换为 rpc::WalkerControl
      return {geom::Vector3D{direction[0u], direction[1u], direction[2u]}, speed, jump}; // 返回对应的 rpc::WalkerControl 对象
    }

  private:

    float direction[3u]; // 方向向量
    float speed; // 速度
    bool jump; // 跳跃状态
  };

#pragma pack(pop) // 恢复对齐方式

#pragma pack(push, 1) // 开始 1 字节对齐
  struct TrafficLightData { // 定义 TrafficLightData 结构体
    TrafficLightData() = default; // 默认构造函数

    char sign_id[32u]; // 标志 ID
    float green_time; // 绿灯时间
    float yellow_time; // 黄灯时间
    float red_time; // 红灯时间
    float elapsed_time; // 已经过的时间
    uint32_t pole_index; // 标志杆索引
    bool time_is_frozen; // 时间是否被冻结
    rpc::TrafficLightState state; // 交通灯状态
  };
#pragma pack(pop) // 恢复对齐方式

#pragma pack(push, 1) // 开始 1 字节对齐
  struct TrafficSignData { // 定义 TrafficSignData 结构体
    TrafficSignData() = default; // 默认构造函数

    char sign_id[32u]; // 标志 ID
  };
#pragma pack(pop) // 恢复对齐方式
} // namespace detail

#pragma pack(push, 1) // 开始 1 字节对齐

  /// 动态状态的参与者在某一帧的状态
  struct ParticipantDynamicState {

    ActorId id; // 参与者 ID

    rpc::ActorState actor_state; // 参与者状态

    geom::Transform transform; // 转换信息

    geom::Vector3D velocity; // 速度

    geom::Vector3D angular_velocity; // 角速度

    geom::Vector3D acceleration; // 加速度

    union TypeDependentState { // 定义一个联合体，根据类型存储不同状态
      detail::TrafficLightData traffic_light_data; // 交通灯数据
      detail::TrafficSignData traffic_sign_data; // 交通标志数据
      detail::VehicleData vehicle_data; // 车辆数据
      detail::PackedWalkerControl walker_control; // 行人控制数据
    } state; // 状态
  };

#pragma pack(pop) // 恢复对齐方式

 static_assert(
    sizeof(ParticipantDynamicState) == 119u, // 确保 ParticipantDynamicState 的大小为 119 字节
    "Invalid ParticipantDynamicState size! "
    "If you modified this class please update the size here, else you may "
    "comment this assert, but your platform may have compatibility issues "
    "connecting to other platforms."); // 如果大小不匹配，给出提示信息

} // namespace data
} // namespace sensor
} // namespace carla