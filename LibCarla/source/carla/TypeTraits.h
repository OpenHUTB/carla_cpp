// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once  // 确保头文件只被包含一次

#include <type_traits> // 包含类型特性库，用于类型特性相关的工具

namespace carla {// 定义命名空间 carla

  template <typename... Ts>
  struct are_same;

  template <typename T0, typename T1, typename... Ts>
  struct are_same<T0, T1, Ts...> {
    static constexpr bool value = std::is_same<T0, T1>::value && are_same<T0, Ts...>::value;/// 检查 T0 和 T1 是否相同，并递归检查 Ts...
  };

  template <typename T0, typename T1>
  struct are_same<T0, T1> {
    static constexpr bool value = std::is_same<T0, T1>::value;
  };

} // namespace carla
