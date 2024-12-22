// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once// 预处理指令，保证该头文件在一个编译单元中只会被包含一次，避免重复包含导致的重复定义等编译错误

#include "carla/MsgPack.h"// 引入 "carla/MsgPack.h" 文件，大概率是用于支持将自定义的数据类型进行MsgPack序列化和反序列化操作。

#include <cstdint>// 引入 <cstdint> 头文件，该头文件定义了一些具有固定宽度的整数类型，例如这里后续会用到的uint8_t类型

namespace carla {
namespace rpc {

  enum class TrafficLightState : uint8_t {// 定义一个强类型的枚举（enum class），名为TrafficLightState，其底层数据类型被指定为uint8_t。
    Red,// 表示交通信号灯处于红色亮起的状态，即车辆需要停车等待的情况
    Yellow,// 表示交通信号灯处于黄色亮起的状态，通常用于提示车辆准备停车或者谨慎通行
    Green,// 表示交通信号灯处于绿色亮起的状态，意味着车辆可以正常通行
    Off,// 表示交通信号灯处于关闭状态，可能是信号灯故障、未启用或者当前时段不需要工作等情况
    Unknown,// 表示交通信号灯的状态无法明确确定，比如由于传感器故障、遮挡等原因导致无法准确判断其处于哪种具体亮灯状态
    SIZE// 这里的SIZE不是一个实际的交通信号灯状态值，它可能是用于表示这个枚举类型中不同状态值的数量（一种约定俗成的用法）
  };

} // namespace rpc
} // namespace carla

MSGPACK_ADD_ENUM(carla::rpc::TrafficLightState);// 调用MSGPACK_ADD_ENUM宏（应该是由 "carla/MsgPack.h" 提供或者相关联的定义）
