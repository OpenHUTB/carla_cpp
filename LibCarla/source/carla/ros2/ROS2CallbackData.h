// Copyright (c) 2023 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#ifdef _MSC_VER
#pragma warning(push)                   // 保存当前的编译器警告状态
#pragma warning(disable:4583)
#pragma warning(disable:4582)
#include <boost/variant2/variant.hpp>   // 这样在编译这行代码时，4583、4582 两个警告将不会出现
#pragma warning(pop)                    // 恢复原先的警告状态
#else
#include <boost/variant2/variant.hpp>
#endif

namespace carla {
namespace ros2 {

  struct VehicleControl
  {
    float   throttle;           // 油门
    float   steer;              // 方向盘
    float   brake;              // 刹车
    bool    hand_brake;         // 手刹
    bool    reverse;            // 倒档
    int32_t gear;               // 档位
    bool    manual_gear_shift;  // 手动档位换
  };

  using ROS2CallbackData = boost::variant2::variant<VehicleControl>;

  using ActorCallback = std::function<void(void *actor, ROS2CallbackData data)>;

} // namespace ros2
} // namespace carla
