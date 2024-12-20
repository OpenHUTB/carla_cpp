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
  inline static auto make_map_keys_const_iterator(It it) {    // 使用类型别名来定义迭代器指向的元素的值的类型（即map的value_type::second_type）
    // 这里假设It是一个指向std::pair<Key, Value>的迭代器，这是std::map等关联容器的典型情况。
    using first_value_type = typename It::value_type::first_type;// 使用std::remove_cv_t和std::remove_reference_t来移除second_value_type上的const、volatile和引用修饰符
    // 这样做是为了得到一个“纯净”的值类型，不受原始迭代器指向的元素类型的影响。
    using decay_first_value_type = typename std::remove_cv_t<typename std::remove_reference_t<first_value_type>>;    // 定义一个类型别名，表示对decay_second_value_type的引用。
    // 注意：这个类型别名可能在这个函数的上下文中没有直接用途，
    // 因为函数实际上并没有返回一个引用到map的值。
    // 它可能是为了展示如何构建这样的类型，或者作为后续实现的一部分。
    using const_ref_to_first = const decay_first_value_type &;
    return boost::make_transform_iterator(it, [](const auto &pair) -> const_ref_to_first { return pair.first; });    // 这里应该有返回语句，但实际上由于标准容器不提供直接指向值的迭代器，
    // 所以这个函数无法直接返回一个有效的迭代器。
    // 可能的实现方式包括返回一个适配器，该适配器接受原始迭代器并提供对值的访问，
    // 或者修改函数的目的和签名以匹配一个可实现的功能。
 
    // 示例（非实际实现）：
    // return some_adapter_or_proxy_that_wraps(it);
  }

  /// 创建一个迭代器，该迭代器提供对map值的非const引用的遍历。
// 模板函数，用于创建一个与给定迭代器兼容的、指向map值类型的迭代器（或类似迭代器）的帮助函数
// 注意：实际上，这个函数不会直接返回一个迭代器到值，因为标准容器不提供这样的迭代器。
// 这个函数的目的可能是为了展示类型推导和转换，或者作为实现此类功能的一部分的占位符。
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
  inline static auto make_map_values_const_iterator(It it) { // 注释应该放在这里，解释函数的功能、参数和返回值
    using second_value_type = typename It::value_type::second_type;  // 获取迭代器当前指向的键值对中的值的类型
    using decay_second_value_type = typename std::remove_cv_t<typename std::remove_reference_t<second_value_type>>;    // 去除值类型的 const 和引用修饰，获取实际的值类型
    using const_ref_to_second = const decay_second_value_type &; // 定义值类型的 const 引用类型
    // 使用 Boost 的 transform_iterator 创建一个新的迭代器，该迭代器在解引用时会调用 lambda 表达式，返回对值的 const 引用
    return boost::make_transform_iterator(it, [](const auto &pair) -> const_ref_to_second { return pair.second; });
  }

} // namespace iterator
} // namespace carla
