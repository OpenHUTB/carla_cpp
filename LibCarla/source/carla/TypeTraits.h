// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once  // ȷ��ͷ�ļ�ֻ������һ��

#include <type_traits> // �����������Կ⣬��������������صĹ���

namespace carla {// ���������ռ� carla

  template <typename... Ts>
  struct are_same;

  template <typename T0, typename T1, typename... Ts>
  struct are_same<T0, T1, Ts...> {
    static constexpr bool value = std::is_same<T0, T1>::value && are_same<T0, Ts...>::value;//定义一个静态常量布尔成员变量 
  };

  template <typename T0, typename T1>
  struct are_same<T0, T1> {
    static constexpr bool value = std::is_same<T0, T1>::value;
  };

} // namespace carla
