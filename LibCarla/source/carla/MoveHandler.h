// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once  // 防止头文件被重复包含

#include <type_traits>  // 包含类型特性的库，尽管在这个例子中未直接使用，但可能是为了未来扩展或与其他模板特性兼容
#include <utility>   // 包含实用工具函数，如std::move，用于支持移动语义  

namespace carla {  // ���������ռ� carla
namespace detail {   // ���������ռ� detail������ʵ��ϸ��

  template <typename FunctorT>
  struct MoveWrapper : FunctorT {
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
