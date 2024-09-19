// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#ifdef LIBCARLA_NO_EXCEPTIONS

namespace std {

  class exception; // 声明 std 命名空间中的 exception 类

} // namespace std

namespace carla {

  /// User define function, similar to Boost throw_exception.
  ///
  /// @important Boost exceptions are also routed to this function.
  ///
  /// When compiled with LIBCARLA_NO_EXCEPTIONS, this function is left undefined
  /// in LibCarla, and the modules using LibCarla are expected to supply an
  /// appropriate definition. Callers of throw_exception are allowed to assume
  /// that the function never returns; therefore, if the user-defined
  /// throw_exception returns, the behavior is undefined.
  [[ noreturn ]] void throw_exception(const std::exception &e);

} // namespace carla

#else

namespace carla {

  template <typename T>
  [[ noreturn ]] void throw_exception(const T &e) { // 模板函数声明，接受任何类型的异常
    throw e;
  }

} // namespace carla

#endif // LIBCARLA_NO_EXCEPTIONS
