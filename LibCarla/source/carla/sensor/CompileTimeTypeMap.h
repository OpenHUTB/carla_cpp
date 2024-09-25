// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <type_traits>
#include <utility>

namespace carla {
namespace sensor {

namespace detail {

  /// CompileTimeTypeMap 的私有实现
  template <size_t Size, typename...>
  struct CompileTimeTypeMapImpl;

  /// 空映射的特化，当找不到键时返回该特化
  template <size_t Size>
  struct CompileTimeTypeMapImpl<Size> {

    template <typename InKey>
    struct get {
      using type = void;// 默认类型为 void
      static constexpr size_t index = Size;// 索引为 Size
    };

    template <size_t Index>
    struct get_by_index {
      using type = void;
      using key = void;
    };
  };

  template <size_t Size, typename Key, typename Value, typename... Rest>
  struct CompileTimeTypeMapImpl<Size, std::pair<Key, Value>, Rest...> {

    static constexpr size_t current_index() {
      return Size - 1u - sizeof...(Rest);// 计算当前索引
    }

    // 递归调用该结构体，直到 InKey 匹配到某个元素
    template <typename InKey>
    struct get {
      using type = typename std::conditional<
          std::is_same<InKey, Key>::value,
          Value,
          typename CompileTimeTypeMapImpl<Size, Rest...>::template get<InKey>::type
        >::type;
      static constexpr size_t index =
          std::is_same<InKey, Key>::value ?
              current_index() :
              CompileTimeTypeMapImpl<Size, Rest...>::template get<InKey>::index;
    };

    // 递归调用该结构体，直到 Index 匹配到某个元素
    template <size_t Index>
    struct get_by_index {
      using type = typename std::conditional<
          Index == current_index(),
          Value,
          typename CompileTimeTypeMapImpl<Size, Rest...>::template get_by_index<Index>::type
        >::type;

      using key = typename std::conditional<
          Index == current_index(),
          Key,
          typename CompileTimeTypeMapImpl<Size, Rest...>::template get_by_index<Index>::key
        >::type;
    };
  };

} // namespace detail

  /// 一个编译时结构，用于映射两种类型。可以通过键或索引查找元素。
  ///
  /// Example usage:
  ///
  ///     using MyMap = CompileTimeTypeMap<std::pair<A, B>, std::pair<C, D>>;
  ///     using type_B = MyMap::get<A>::type;
  ///     constexpr size_t index_B = MyMap::get<A>::index;
  ///     using type_B_too = MyMap::get_by_index<index_B>::type;
  ///
  template <typename... Items>
  struct CompileTimeTypeMap {

    static constexpr size_t size() {
      return sizeof...(Items);// 返回映射大小
    }

    template <typename InKey>
    using get = typename detail::CompileTimeTypeMapImpl<sizeof...(Items), Items...>::template get<InKey>;

    template <size_t Index>
    using get_by_index = typename detail::CompileTimeTypeMapImpl<sizeof...(Items), Items...>::template get_by_index<Index>;
  };

} // namespace sensor
} // namespace carla
