// 版权所有 (c) 2017 巴塞罗那自治大学 (UAB) 计算机视觉中心 (CVC)。
//
// 本作品根据 MIT 许可证的条款授权。
// 如需副本，请参阅 <https://opensource.org/licenses/MIT>。

#pragma once   // 防止头文件被重复包含，确保每个文件只包含一次

#include "carla/Platform.h"   // 包含Carla平台的头文件，可能包含平台特定的定义或配置
// 定义日志级别的宏，用于控制日志输出的详细程度  
#define LIBCARLA_LOG_LEVEL_DEBUG     10 // 调试级别，输出最详细的日志信息
#define LIBCARLA_LOG_LEVEL_INFO      20 // 信息级别，输出一般信息
#define LIBCARLA_LOG_LEVEL_WARNING   30 // 警告级别，输出潜在问题的警告
#define LIBCARLA_LOG_LEVEL_ERROR     40 // 错误级别，输出错误信息
#define LIBCARLA_LOG_LEVEL_CRITICAL  50 // 严重错误级别，输出严重错误或系统崩溃前的信息
#define LIBCARLA_LOG_LEVEL_NONE     100 // 不输出任何日志

// 如果没有定义LIBCARLA_LOG_LEVEL，则根据是否定义了NDEBUG来设置默认日志级别
#ifndef LIBCARLA_LOG_LEVEL
#  ifdef NDEBUG
#    define LIBCARLA_LOG_LEVEL LIBCARLA_LOG_LEVEL_WARNING  // 如果没有定义NDEBUG（即处于调试模式），则设置为警告级别
#  else
#    define LIBCARLA_LOG_LEVEL LIBCARLA_LOG_LEVEL_INFO  // 如果定义了NDEBUG（即处于发布模式），则设置为信息级别 
#  endif // NDEBUG
#endif // 结束对 LIBCARLA_LOG_LEVEL 是否未定义的检查

// 下面的日志函数在LIBCARLA_LOG_LEVEL大于等于函数对应的日志级别时才会激活
//
//  * log_debug      - 输出调试信息
//  * log_info       - 输出一般信息
//  * log_error      - 输出错误信息
//  * log_critical   - 输出严重错误信息
//
// 以及宏
//
//  * LOG_DEBUG_ONLY(/* code here */)  - 仅在调试级别激活时执行代码
//  * LOG_INFO_ONLY(/* code here */)   - 仅在信息级别激活时执行代码
 
// =============================================================================
// -- 日志函数的实现 ------------------------------------------
// =============================================================================

#include <iostream> // 包含标准输入输出流库

namespace carla {

namespace logging {

  // https://stackoverflow.com/a/27375675
  // 辅助函数，用于将参数列表写入输出流
  // 使用参数包展开技术来逐个写入参数
  template <typename Arg, typename ... Args>
  LIBCARLA_NOINLINE // 阻止内联，可能是为了避免编译器优化导致的代码重复
  static void write_to_stream(std::ostream &out, Arg &&arg, Args && ... args) {
    out << std::boolalpha << std::forward<Arg>(arg); // 输出第一个参数，并启用bool值的alpha表示（true/false）
    // 使用一个数组初始化器来展开参数包，并逐个写入参数
    using expander = int[];
    (void) expander{0, (void(out << ' ' << std::forward<Args>(args)), 0) ...};
    // 注意：这里的void(out << ' ' << ...)是一个逗号表达式，其结果为最后一个操作的结果（这里是0）
  }

  // 通用日志函数，用于输出日志信息（不带前缀）
  template <typename ... Args>
  static inline void log(Args && ... args) {
    logging::write_to_stream(std::cout, std::forward<Args>(args) ..., '\n');
  }

} // namespace logging

// 根据当前日志级别定义具体的日志输出函数
// 如果当前日志级别小于等于某个特定级别，则定义对应的日志函数；否则，定义一个空函数（不执行任何操作）
#if LIBCARLA_LOG_LEVEL <= LIBCARLA_LOG_LEVEL_DEBUG

  template <typename ... Args>
  static inline void log_debug(Args && ... args) {
    logging::write_to_stream(std::cout, "DEBUG:", std::forward<Args>(args) ..., '\n');
  }

#else

  template <typename ... Args>
  static inline void log_debug(Args && ...) {} // 空函数，不执行任何操作

#endif

#if LIBCARLA_LOG_LEVEL <= LIBCARLA_LOG_LEVEL_INFO

  template <typename ... Args>
  static inline void log_info(Args && ... args) {
    logging::write_to_stream(std::cout, "INFO: ", std::forward<Args>(args) ..., '\n');
  }

#else

  template <typename ... Args>
  static inline void log_info(Args && ...) {} // 空函数，不执行任何操作

#endif

#if LIBCARLA_LOG_LEVEL <= LIBCARLA_LOG_LEVEL_WARNING

  template <typename ... Args>
  static inline void log_warning(Args && ... args) {
    logging::write_to_stream(std::cerr, "WARNING:", std::forward<Args>(args) ..., '\n');
  }

#else

  template <typename ... Args>
  static inline void log_warning(Args && ...) {}  // 空函数，不执行任何操作

#endif

#if LIBCARLA_LOG_LEVEL <= LIBCARLA_LOG_LEVEL_ERROR

  template <typename ... Args>
  static inline void log_error(Args && ... args) {
    logging::write_to_stream(std::cerr, "ERROR:", std::forward<Args>(args) ..., '\n');
  }

#else

  template <typename ... Args>
  static inline void log_error(Args && ...) {} // 空函数，不执行任何操作

#endif

#if LIBCARLA_LOG_LEVEL <= LIBCARLA_LOG_LEVEL_CRITICAL

  template <typename ... Args>
  static inline void log_critical(Args && ... args) {
    logging::write_to_stream(std::cerr, "CRITICAL:", std::forward<Args>(args) ..., '\n');
  }

#else

  template <typename ... Args>
  static inline void log_critical(Args && ...) {} // 空函数，不执行任何操作

#endif

} // namespace carla

// =============================================================================
// ------------------------------- 宏的实现 ------------------------------------
// =============================================================================

// 根据当前日志级别定义宏，用于在特定级别下执行代码
#if LIBCARLA_LOG_LEVEL <= LIBCARLA_LOG_LEVEL_DEBUG
#  define LOG_DEBUG_ONLY(code) code // 如果当前日志级别小于等于调试级别，则执行代码
#else
#  define LOG_DEBUG_ONLY(code)  // 否则，不执行代码（宏展开为空）
#endif

#if LIBCARLA_LOG_LEVEL <= LIBCARLA_LOG_LEVEL_INFO
#  define LOG_INFO_ONLY(code) code // 如果当前日志级别小于等于信息级别，则执行代码
#else
#  define LOG_INFO_ONLY(code) // 否则，不执行代码（宏展开为空）
#endif
