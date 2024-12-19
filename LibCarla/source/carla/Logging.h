// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once   // 防止头文件被重复包含

#include "carla/Platform.h"   // 包含Carla平台的头文件
// 定义日志级别的宏，用于控制日志输出的详细程度  
#define LIBCARLA_LOG_LEVEL_DEBUG     10
#define LIBCARLA_LOG_LEVEL_INFO      20
#define LIBCARLA_LOG_LEVEL_WARNING   30
#define LIBCARLA_LOG_LEVEL_ERROR     40
#define LIBCARLA_LOG_LEVEL_CRITICAL  50
#define LIBCARLA_LOG_LEVEL_NONE     100

#ifndef LIBCARLA_LOG_LEVEL
#  ifdef NDEBUG
#    define LIBCARLA_LOG_LEVEL LIBCARLA_LOG_LEVEL_WARNING  // 如果没有定义NDEBUG，则设置为警告级别
#  else
#    define LIBCARLA_LOG_LEVEL LIBCARLA_LOG_LEVEL_INFO  // 如果没有定义NDEBUG且没有指定日志级别，则设置为信息级别   
#  endif // NDEBUG
#endif // 结束对 LIBCARLA_LOG_LEVEL 是否未定义的检查

// The following log functions are available, they are only active if
// LIBCARLA_LOG_LEVEL is greater equal the function's log level.
//
//  * log_debug
//  * log_info
//  * log_error
//  * log_critical
//
// And macros
//
//  * LOG_DEBUG_ONLY(/* code here */)
//  * LOG_INFO_ONLY(/* code here */)

// =============================================================================
// -- Implementation of log functions ------------------------------------------
// =============================================================================

#include <iostream>

namespace carla {

namespace logging {

  // https://stackoverflow.com/a/27375675
  template <typename Arg, typename ... Args>
  LIBCARLA_NOINLINE
  static void write_to_stream(std::ostream &out, Arg &&arg, Args && ... args) {
            // 将第一个参数 arg 输出到流对象 out 中，设置 std::boolalpha 标志使得布尔类型的值以文本形式（true 或者 false）输出，通过 std::forward<Arg>(arg) 完美转发参数，保持其原始的左值或右值属性，避免不必要的拷贝或者移动开销。
    out << std::boolalpha << std::forward<Arg>(arg);
            // 通过定义一个匿名的 int 数组 expander，利用逗号表达式的求值特性，依次执行 (void(out << ' ' << std::forward<Args>(args)), 0) 这个表达式，实现将每个参数输出到流对象中，并且每个参数之间用空格隔开，最后一个参数输出后结束整个可变参数的处理过程。
    using expander = int[];
    (void) expander{0, (void(out << ' ' << std::forward<Args>(args)), 0) ...};
  }
    // 函数模板 log，它接受任意数量和类型的参数（通过可变参数模板实现），功能是将传入的所有参数依次输出到标准输出流（std::cout）中，并在最后输出一个换行符，实现简单的日志信息输出功能，调用了前面定义的 write_to_stream 函数来完成实际的参数输出操作。
  template <typename ... Args>
  static inline void log(Args && ... args) {
    logging::write_to_stream(std::cout, std::forward<Args>(args) ..., '\n');
  }

} // namespace logging

#if LIBCARLA_LOG_LEVEL <= LIBCARLA_LOG_LEVEL_DEBUG

  template <typename ... Args>
  static inline void log_debug(Args && ... args) {
    logging::write_to_stream(std::cout, "DEBUG:", std::forward<Args>(args) ..., '\n');
  }

#else

  template <typename ... Args>
  static inline void log_debug(Args && ...) {}

#endif

#if LIBCARLA_LOG_LEVEL <= LIBCARLA_LOG_LEVEL_INFO

  template <typename ... Args>
  static inline void log_info(Args && ... args) {
    logging::write_to_stream(std::cout, "INFO: ", std::forward<Args>(args) ..., '\n');
  }

#else

  template <typename ... Args>
  static inline void log_info(Args && ...) {}

#endif

#if LIBCARLA_LOG_LEVEL <= LIBCARLA_LOG_LEVEL_WARNING

  template <typename ... Args>
  static inline void log_warning(Args && ... args) {
    logging::write_to_stream(std::cerr, "WARNING:", std::forward<Args>(args) ..., '\n');
  }

#else

  template <typename ... Args>
  static inline void log_warning(Args && ...) {}

#endif

#if LIBCARLA_LOG_LEVEL <= LIBCARLA_LOG_LEVEL_ERROR

  template <typename ... Args>
  static inline void log_error(Args && ... args) {
    logging::write_to_stream(std::cerr, "ERROR:", std::forward<Args>(args) ..., '\n');
  }

#else

  template <typename ... Args>
  static inline void log_error(Args && ...) {}

#endif

#if LIBCARLA_LOG_LEVEL <= LIBCARLA_LOG_LEVEL_CRITICAL

  template <typename ... Args>
  static inline void log_critical(Args && ... args) {
    logging::write_to_stream(std::cerr, "CRITICAL:", std::forward<Args>(args) ..., '\n');
  }

#else

  template <typename ... Args>
  static inline void log_critical(Args && ...) {}

#endif

} // namespace carla

// =============================================================================
// -- Implementation of macros -------------------------------------------------
// =============================================================================

#if LIBCARLA_LOG_LEVEL <= LIBCARLA_LOG_LEVEL_DEBUG
#  define LOG_DEBUG_ONLY(code) code
#else
#  define LOG_DEBUG_ONLY(code)
#endif

#if LIBCARLA_LOG_LEVEL <= LIBCARLA_LOG_LEVEL_INFO
#  define LOG_INFO_ONLY(code) code
#else
#  define LOG_INFO_ONLY(code)
#endif
