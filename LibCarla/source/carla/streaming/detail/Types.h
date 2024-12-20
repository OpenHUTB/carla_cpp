// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
/**
 * @file
 * @brief 包含与Carla流处理相关的底层细节的头文件。
 *
 * 此文件定义了一些与Carla流处理相关的底层类型和静态断言，确保类型一致性。
 */
#include "carla/Buffer.h"// @brief 引入Carla的Buffer类定义。

#include <cstdint>// @brief 引入标准整数类型定义。
#include <type_traits>// @brief 引入类型特征库，用于类型检查和转换。
 /**
  * @namespace carla::streaming::detail
  * @brief Carla流处理模块中的底层细节命名空间。
  *
  * 该命名空间包含了Carla流处理模块中不希望被外部直接访问的底层实现细节。
  */
namespace carla {
namespace streaming {
namespace detail {
    /**
   * @typedef stream_id_type
   * @brief 流ID的类型定义。
   *
   * 使用uint32_t作为流ID的类型，以确保跨平台的类型一致性。
   */
  using stream_id_type = uint32_t;
  /**
   * @typedef message_size_type
   * @brief 消息大小的类型定义。
   *
   * 使用uint32_t作为消息大小的类型，以确保跨平台的类型一致性。
   */
  using message_size_type = uint32_t;
  /**
   * @brief 静态断言，确保message_size_type与Buffer::size_type类型一致。
   *
   * 此静态断言用于在编译时检查message_size_type是否与Buffer类中的size_type类型相同，
   * 如果不相同，则编译失败并显示错误信息"uint type mismatch!"。
   */
  static_assert(
      std::is_same<message_size_type, Buffer::size_type>::value,
      "uint type mismatch!");// @brief 错误信息：如果类型不匹配，则显示此信息。

} // namespace detail
} // namespace streaming
} // namespace carla
