// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once  // È·±£Í·ÎÄ¼şÖ»±»°üº¬Ò»´Î

#include <type_traits> // °üº¬ÀàĞÍÌØĞÔ¿â£¬ÓÃÓÚÀàĞÍÌØĞÔÏà¹ØµÄ¹¤¾ß

namespace carla {// ¶¨ÒåÃüÃû¿Õ¼ä carla

  template <typename... Ts>
  struct are_same;

  template <typename T0, typename T1, typename... Ts>
  struct are_same<T0, T1, Ts...> {
    static constexpr bool value = std::is_same<T0, T1>::value && are_same<T0, Ts...>::value;//å®šä¹‰ä¸€ä¸ªé™æ€å¸¸é‡å¸ƒå°”æˆå‘˜å˜é‡ 
  };

  template <typename T0, typename T1>
  struct are_same<T0, T1> {
    static constexpr bool value = std::is_same<T0, T1>::value;
  };

} // namespace carla
