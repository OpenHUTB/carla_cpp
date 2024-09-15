// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <boost/iterator/transform_iterator.hpp>

namespace carla {
namespace iterator {

 
  template <typename It>
  inline static auto make_map_keys_iterator(It it) {
  	// 确定输入迭代器指向的键值对中键的类型
    using first_value_type = typename It::value_type::first_type;
    // 去除键类型中的 const 和引用修饰
    using decay_first_value_type = typename std::remove_cv_t<typename std::remove_reference_t<first_value_type>>;
    // 定义对去除修饰后的键类型的非 const 引用类型
    using ref_to_first = decay_first_value_type &;
     // 使用 Boost 的 transform_iterator 创建一个新的迭代器，该迭代器在解引用时会调用 lambda 表达式，返回对键的非 const 引用
    return boost::make_transform_iterator(it, [](auto &pair) -> ref_to_first { return pair.first; });
  }

  
  inline static auto make_map_keys_const_iterator(It it) {
    using first_value_type = typename It::value_type::first_type;
    using decay_first_value_type = typename std::remove_cv_t<typename std::remove_reference_t<first_value_type>>;
    using const_ref_to_first = const decay_first_value_type &;
    return boost::make_transform_iterator(it, [](const auto &pair) -> const_ref_to_first { return pair.first; });
  }

  
  template <typename It>
  inline static auto make_map_values_iterator(It it) {
    using second_value_type = typename It::value_type::second_type;
    using decay_second_value_type = typename std::remove_cv_t<typename std::remove_reference_t<second_value_type>>;
    using ref_to_second = decay_second_value_type &;
    // 使用 Boost 的 transform_iterator 创建一个新的迭代器，该迭代器在解引用时会调用 lambda 表达式，返回对值的非 const 引用
    return boost::make_transform_iterator(it, [](auto &pair) -> ref_to_second { return pair.second; });
  }

 
  template <typename It>
  inline static auto make_map_values_const_iterator(It it) {
    using second_value_type = typename It::value_type::second_type;
    using decay_second_value_type = typename std::remove_cv_t<typename std::remove_reference_t<second_value_type>>;
    using const_ref_to_second = const decay_second_value_type &;
    // 使用 Boost 的 transform_iterator 创建一个新的迭代器，该迭代器在解引用时会调用 lambda 表达式，返回对值的 const 引用
    return boost::make_transform_iterator(it, [](const auto &pair) -> const_ref_to_second { return pair.second; });
  }

} // namespace iterator
} // namespace carla
