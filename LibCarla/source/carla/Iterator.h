// 版权所有 (c) 2017 巴塞罗那自治大学（UAB）计算机视觉中心（CVC）。
//
// 本作品依据 MIT 许可证条款进行授权。
// 如需获取许可证副本，请访问 <https://opensource.org/licenses/MIT>。

#pragma once

#include <boost/iterator/transform_iterator.hpp>
// 定义carla命名空间下的iterator子命名空间
namespace carla {
namespace iterator {

 
  /// 创建一个迭代器，用于遍历map中键的非const引用
  /// \param it 输入的map迭代器
  /// \return 返回一个transform_iterator，解引用时返回map中键的非const引用
  template <typename It>
  inline static auto make_map_keys_iterator(It it) {
  	 // 确定map迭代器指向的键值对中键的类型
    using first_value_type = typename It::value_type::first_type;
    // 去除键类型中的const和引用修饰，得到实际的键类型
    using decay_first_value_type = typename std::remove_cv_t<typename std::remove_reference_t<first_value_type>>;
    // 定义对去除修饰后的键类型的非 const 引用类型
    using ref_to_first = decay_first_value_type &;
    // 使用Boost的transform_iterator创建一个新的迭代器，该迭代器在解引用时会返回键的非const引用
    return boost::make_transform_iterator(it, [](auto &pair) -> ref_to_first { return pair.first; });
  }
  /// 创建一个迭代器，用于遍历map中键的const引用
  /// \param it 输入的map迭代器
  /// \return 返回一个transform_iterator，解引用时返回map中键的const引用
  template <typename It>
  inline static auto make_map_keys_const_iterator(It it) {
    // 确定map迭代器指向的键值对中键的类型
    using first_value_type = typename It::value_type::first_type;
    // 去除键类型中的const和引用修饰，得到实际的键类型
    using decay_first_value_type = typename std::remove_cv_t<typename std::remove_reference_t<first_value_type>>;
    // 定义对去除修饰后的键类型的const引用类型
    using const_ref_to_first = const decay_first_value_type &;
    // 使用Boost的transform_iterator创建一个新的迭代器，该迭代器在解引用时会返回键的const引用
    return boost::make_transform_iterator(it, [](const auto &pair) -> const_ref_to_first { return pair.first; });
  }

  /// 创建一个迭代器，用于遍历map中值的非const引用
  /// \param it 输入的map迭代器
  /// \return 返回一个transform_iterator，解引用时返回map中值的非const引用
  template <typename It>
  inline static auto make_map_values_iterator(It it) {
    // 确定map迭代器指向的键值对中值的类型
    using second_value_type = typename It::value_type::second_type;
    // 去除值类型中的const和引用修饰，得到实际的值类型
    using decay_second_value_type = typename std::remove_cv_t<typename std::remove_reference_t<second_value_type>>;
    // 定义对去除修饰后的值类型的非const引用类型
    using ref_to_second = decay_second_value_type &;
    // 使用 Boost 的 transform_iterator 创建一个新的迭代器，该迭代器在解引用时会调用 lambda 表达式，返回对值的非 const 引用
    return boost::make_transform_iterator(it, [](auto &pair) -> ref_to_second { return pair.second; });
  }

  /// 创建一个迭代器，用于遍历map中值的const引用
  /// \param it 输入的map迭代器
  /// \return 返回一个transform_iterator，解引用时返回map中值的const引用
  template <typename It>
  inline static auto make_map_values_const_iterator(It it) {
    // 确定map迭代器指向的键值对中值的类型
    using second_value_type = typename It::value_type::second_type;
    // 去除值类型中的const和引用修饰，得到实际的值类型
    using decay_second_value_type = typename std::remove_cv_t<typename std::remove_reference_t<second_value_type>>;
    // 定义对去除修饰后的值类型的const引用类型
    using const_ref_to_second = const decay_second_value_type &; // 定义值类型的 const 引用类型
    // 使用 Boost 的 transform_iterator 创建一个新的迭代器，该迭代器在解引用时会调用 lambda 表达式，返回对值的 const 引用
    return boost::make_transform_iterator(it, [](const auto &pair) -> const_ref_to_second { return pair.second; });
  }

} // 命名空间 iterator 结束
} // 命名空间 carla 结束
