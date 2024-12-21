// Copyright (c) 2023 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
// 这是一个预处理指令，用于确保头文件只被编译一次，避免重复定义等问题
#ifdef _MSC_VER
// 判断当前是否使用的是微软的编译器（Visual C++ 编译器等
#pragma warning(push)                   // 保存当前的编译器警告状态
#pragma warning(disable:4583)
// 禁用编号为4583的编译器警告，可能是针对当前包含的头文件等情况避免出现该特定警告影响编译提示信息的简洁性等

#pragma warning(disable:4582)
#include <boost/variant2/variant.hpp>   // 这样在编译这行代码时，4583、4582 两个警告将不会出现
#pragma warning(pop)                    // 恢复原先的警告状态
#else
// 如果不是微软编译器（_MSC_VER这个条件不满足），就正常包含下面这个头文件
#include <boost/variant2/variant.hpp>
#endif

namespace carla {
// 定义名为carla的命名空间，用于对相关的类型、函数等进行逻辑上的分组，避免名字冲突
namespace ros2 {
// 在carla命名空间内再定义一个ros2命名空间，进一步细分组织代码元素
  struct VehicleControl
// 定义一个名为VehicleControl的结构体，用于封装车辆控制相关的属性
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
// 使用类型别名定义ROS2CallbackData，它是一个基于 boost::variant2::variant 的类型，能够容纳VehicleControl类型的数据，
  // 可能用于在回调函数等场景中传递不同类型的数据（这里目前只指定了VehicleControl这一种类型）
  using ActorCallback = std::function<void(void *actor, ROS2CallbackData data)>;
// 使用类型别名定义ActorCallback，它是一个函数类型别名，代表一个函数指针类型，该函数接受一个void*类型的actor参数（可能指向某个演员对象之类的，
  // 具体要看上下文实际含义）和ROS2CallbackData类型的数据参数，并且无返回值，通常可用于定义回调函数的类型规范
} // namespace ros2
} // namespace carla
