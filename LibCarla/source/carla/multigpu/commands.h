// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <cstdint>

namespace carla {
namespace multigpu {
// 使用uint32_t作为枚举的基础类型，确保枚举值有足够的表示范围
enum MultiGPUCommand : uint32_t {
  SEND_FRAME = 0,  // 发送帧数据，可能是模拟环境中的一帧图像或传感器数据
  LOAD_MAP,  // 加载地图数据，用于模拟环境的构建
  GET_TOKEN, // 获取某种令牌或标识符，可能用于同步或权限控制
  ENABLE_ROS,  // 启用ROS（Robot Operating System）集成，ROS是一个用于机器人开发的灵活框架
  DISABLE_ROS, // 禁用ROS集成
  IS_ENABLED_ROS, // 查询ROS集成是否启用
  YOU_ALIVE // 一种心跳或存活检查命令，用于确认接收方是否在线或响应
};
// 定义一个结构体CommandHeader，用于表示命令的头部信息  
// 头部信息通常包括命令的标识符和后续数据的大小
struct CommandHeader {
  MultiGPUCommand id; // 命令的标识符，从MultiGPUCommand枚举中选择
  uint32_t size; // 跟随此头部之后的数据的大小（以字节为单位
};

}  // namespace multigpu 结束multigpu命名空间的定义
} // namespace carla 结束carla命名空间的定义
