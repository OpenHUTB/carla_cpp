// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once  // 防止头文件被重复包含
// 检查是否使用Microsoft Visual C++编译器
#if defined(_MSC_VER)
   // 定义LIBCARLA_FORCEINLINE宏为__forceinline，用于强制内联函数 
#  define LIBCARLA_FORCEINLINE __forceinline
   // 定义LIBCARLA_NOINLINE宏为__declspec(noinline)，用于禁止函数内联RCEINLINE __forceinline  
#  define LIBCARLA_NOINLINE __declspec(noinline)


  // 检查是否使用Clang或GCC编译器
#elif defined(__clang__) || defined(__GNUC__)
  // 如果定义了NDEBUG（即非调试模式），则定义LIBCARLA_FORCEINLINE宏为inline并添加always_inline属性 
#  if defined(NDEBUG)
#    define LIBCARLA_FORCEINLINE inline __attribute__((always_inline))
  // 否则，在非调试模式下，仅定义为inline
#  else
#    define LIBCARLA_FORCEINLINE inline
#  endif // NDEBUG
#  define LIBCARLA_NOINLINE __attribute__((noinline))  // 禁止内联
#else
#  warning Compiler not supported.  // 编译器不支持的警告
#  define LIBCARLA_NOINLINE // 定义LIBCARLA_NOINLINE为空
#endif
