// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once  // ȷ��ͷ�ļ�ֻ������һ��

#include <type_traits>  // �����������Կ⣬��������������صĹ���
#include <utility>  //  ����ʵ�ù��߿⣬�ṩ std::move �ȹ���

namespace carla {  // ���������ռ� carla
namespace detail {   //  ���������ռ� detail������ʵ��ϸ��

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
