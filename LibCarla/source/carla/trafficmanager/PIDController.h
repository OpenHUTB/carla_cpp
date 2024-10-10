// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

##include <algorithm>  // 引入算法库

#include "carla/trafficmanager/Constants.h"  // 引入常量定义
#include "carla/trafficmanager/DataStructures.h"  // 引入数据结构定义

namespace carla {  // 定义命名空间 carla
namespace traffic_manager {  // 定义命名空间 traffic_manager

namespace chr = std::chrono;  // 为 std::chrono 起别名为 chr

using namespace constants::PID;  // 使用 constants::PID 命名空间中的内容

using TimeInstance = chr::time_point<chr::system_clock, chr::nanoseconds>;  // 定义时间实例类型

namespace PID {  // 定义命名空间 PID

/// 此函数计算基于车辆当前状态变化的执行信号，以最小化 PID 错误。
inline ActuationSignal RunStep(StateEntry present_state,  // 当前状态
                        StateEntry previous_state,  // 前一个状态
                        const std::vector<float> &longitudinal_parameters,  // 纵向参数
                        const std::vector<float> &lateral_parameters) {  // 横向参数

  // 纵向 PID 计算。
  const float expr_v =
      longitudinal_parameters[0] * present_state.velocity_deviation +  // 比例项
      longitudinal_parameters[1] * (present_state.velocity_deviation + previous_state.velocity_deviation) * DT +  // 积分项
      longitudinal_parameters[2] * (present_state.velocity_deviation - previous_state.velocity_deviation) * INV_DT;  // 微分项

  float throttle;  // 油门变量
  float brake;  // 刹车变量

  if (expr_v > 0.0f) {  // 如果计算得到的速度表达式大于零
    throttle = std::min(expr_v, MAX_THROTTLE);  // 油门取较小值
    brake = 0.0f;  // 刹车为零
  } else {  // 否则
    throttle = 0.0f;  // 油门为零
    brake = std::min(std::abs(expr_v), MAX_BRAKE);  // 刹车取较小值
  }

  // 横向 PID 计算。
  float steer =
      lateral_parameters[0] * present_state.angular_deviation +  // 比例项
      lateral_parameters[1] * (present_state.angular_deviation + previous_state.angular_deviation) * DT +  // 积分项
      lateral_parameters[2] * (present_state.angular_deviation - previous_state.angular_deviation) * INV_DT;  // 微分项

  steer = std::max(previous_state.steer - MAX_STEERING_DIFF, std::min(steer, previous_state.steer + MAX_STEERING_DIFF));  // 限制转向角度变化
  steer = std::max(-MAX_STEERING, std::min(steer, MAX_STEERING));  // 限制转向范围

  return ActuationSignal{throttle, brake, steer};  // 返回执行信号
}

} // namespace PID
} // namespace traffic_manager
} // namespace carla
