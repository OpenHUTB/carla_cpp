// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <type_traits>

namespace carla {//定义一个叫做carla的类 

  template <typename... Ts>  // 定义一个模板结构体，接受可变数量的类型参数
  struct are_same;

  template <typename T0, typename T1, typename... Ts> // 针对至少两个类型的特化
  struct are_same<T0, T1, Ts...> {
    static constexpr bool value = std::is_same<T0, T1>::value && are_same<T0, Ts...>::value;//定义一个静态常量布尔成员变量 
  };

  template <typename T0, typename T1>  // 针对两个类型的特化
  struct are_same<T0, T1> {
    static constexpr bool value = std::is_same<T0, T1>::value;  // 检查 T0 和 T1 是否相同
  };

} // namespace carla
