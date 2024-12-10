// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <boost/iterator/transform_iterator.hpp>
// 定义一个命名空间carla下的iterator命名空间 
namespace carla {
namespace iterator {

 /// 创建一个迭代器，遍历map中键的非const引用
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
  /// 创建一个迭代器，该迭代器提供对map键的const引用的遍历。
  template <typename It>
  inline static auto make_map_keys_const_iterator(It it) {
    using first_value_type = typename It::value_type::first_type;
    using decay_first_value_type = typename std::remove_cv_t<typename std::remove_reference_t<first_value_type>>;
    using const_ref_to_first = const decay_first_value_type &;
    return boost::make_transform_iterator(it, [](const auto &pair) -> const_ref_to_first { return pair.first; });
  }

  /// 创建一个迭代器，该迭代器提供对map值的非const引用的遍历。
  template <typename It>
  inline static auto make_map_values_iterator(It it) {
    using second_value_type = typename It::value_type::second_type;
    using decay_second_value_type = typename std::remove_cv_t<typename std::remove_reference_t<second_value_type>>;
    using ref_to_second = decay_second_value_type &;
    // 使用 Boost 的 transform_iterator 创建一个新的迭代器，该迭代器在解引用时会调用 lambda 表达式，返回对值的非 const 引用
    return boost::make_transform_iterator(it, [](auto &pair) -> ref_to_second { return pair.second; });
  }

 /// 创建一个迭代器，该迭代器提供对map值的const引用的遍历。
  template <typename It>
  inline static auto make_map_values_const_iterator(It it) {
    using second_value_type = typename It::value_type::second_type;  // 获取迭代器当前指向的键值对中的值的类型
    using decay_second_value_type = typename std::remove_cv_t<typename std::remove_reference_t<second_value_type>>;    // 去除值类型的 const 和引用修饰，获取实际的值类型
    using const_ref_to_second = const decay_second_value_type &; // 定义值类型的 const 引用类型
    // 使用 Boost 的 transform_iterator 创建一个新的迭代器，该迭代器在解引用时会调用 lambda 表达式，返回对值的 const 引用
    return boost::make_transform_iterator(it, [](const auto &pair) -> const_ref_to_second { return pair.second; });
  }

} // namespace iterator
} // namespace carla
