// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once  // 确保头文件只被包含一次，避免重复包含问题

#if defined(_MSC_VER)  // 如果编译器是 Microsoft Visual C++
#  define LIBCARLA_FORCEINLINE __forceinline  // 定义 LIBCARLA_FORCEINLINE 为 __forceinline，强制内联
#  define LIBCARLA_NOINLINE __declspec(noinline)  // 定义 LIBCARLA_NOINLINE 为 __declspec(noinline)，禁止内联
#elif defined(__clang__) || defined(__GNUC__)  // 如果编译器是 Clang 或 GCC
#  if defined(NDEBUG)  // 如果在发布模式下（NDEBUG 已定义）
#    define LIBCARLA_FORCEINLINE inline __attribute__((always_inline))  // 定义 LIBCARLA_FORCEINLINE 为 inline 和 __attribute__((always_inline))，强制内联
#  else
#    define LIBCARLA_FORCEINLINE inline  // 定义 LIBCARLA_FORCEINLINE 为 inline（在调试模式下，不强制内联）
#  endif // NDEBUG
#  define LIBCARLA_NOINLINE __attribute__((noinline))  // 定义 LIBCARLA_NOINLINE 为 __attribute__((noinline))，禁止内联
#else  // 如果编译器不被支持
#  warning Compiler not supported.  // 发出警告，提示编译器不被支持
#  define LIBCARLA_NOINLINE  // 定义 LIBCARLA_NOINLINE 为空（无操作），以避免未定义行为
#endif

