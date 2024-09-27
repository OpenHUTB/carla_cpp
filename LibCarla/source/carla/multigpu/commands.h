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

struct CommandHeader {
  MultiGPUCommand id;
  uint32_t size;
};

} // namespace multigpu
} // namespace carla
