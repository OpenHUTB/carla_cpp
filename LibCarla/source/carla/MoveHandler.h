// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once  // 确保此头文件仅被包含一次

#include <type_traits>  // 包含类型特征相关的头文件，提供类型特性支持
#include <utility>  // 包含通用工具函数，比如 std::move

namespace carla {  // 定义命名空间 carla
namespace detail {   // 定义命名空间 detail，用于实现细节

  template <typename FunctorT>  // ģ�������������������� FunctorT
  struct MoveWrapper : FunctorT {// ���� MoveWrapper �ṹ���̳��� FunctorT
    MoveWrapper(FunctorT &&f) : FunctorT(std::move(f)) {}

    MoveWrapper(MoveWrapper &&) = default;
    MoveWrapper& operator=(MoveWrapper &&) = default;

    MoveWrapper(const MoveWrapper &);
    MoveWrapper& operator=(const MoveWrapper &);
  };

} // namespace detail

  /// Hack to trick asio into accepting move-only handlers, if the handler were
  /// actually copied it would result in a link error.
  ///
  /// @see https://stackoverflow.com/a/22891509.
  template <typename FunctorT>
  auto MoveHandler(FunctorT &&func) {
    using F = typename std::decay<FunctorT>::type;
    return detail::MoveWrapper<F>{std::move(func)};
  }

} // namespace carla
