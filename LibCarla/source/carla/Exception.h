// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once // 防止头文件被多次包含

#ifdef LIBCARLA_NO_EXCEPTIONS // 如果定义了 LIBCARLA_NO_EXCEPTIONS

namespace std { // 定义标准命名空间

    class exception; // 前向声明异常类

} // namespace std

namespace carla { // 定义 carla 命名空间

    /// 用户自定义的函数，类似于 Boost 的 throw_exception。
   ///
   /// @important Boost 异常也会路由到此函数。
   ///
   /// 当使用 LIBCARLA_NO_EXCEPTIONS 编译时，此函数在 LibCarla 中未定义，
   /// 使用 LibCarla 的模块需要提供合适的定义。调用 throw_exception 的代码
   /// 可以假设此函数不会返回；因此，如果用户定义的 throw_exception 返回，
   /// 行为是未定义的。
    [[ noreturn ]] void throw_exception(const std::exception& e); // 声明不返回的异常抛出函数

} // namespace carla

#else // 如果未定义 LIBCARLA_NO_EXCEPTIONS

namespace carla { // 定义 carla 命名空间

    template <typename T>
    [[ noreturn ]] void throw_exception(const T& e) { // 模板异常抛出函数
        throw e; // 抛出异常
    }

} // namespace carla

#endif // LIBCARLA_NO_EXCEPTIONS
