// 版权所有 (c) 2019 巴塞罗那自治大学 (UAB) 的计算机视觉中心 (CVC)。
//
// 本作品依据 MIT 许可证的条款进行授权。
// 如需许可证副本，请访问 <https://opensource.org/licenses/MIT>。

/// 用于断言和仅调试代码的宏实用程序。
///
/// 定义三个级别的断言：debug、development 和 release
/// 由default，如果定义了 NDEBUG，则级别设置为 development，
/// 否则设置为 debug。
///
/// 此处定义了以下宏：
///
///   * DEBUG_ONLY(code) - 只有当断言级别设置为调试时，才会对代码进行评估
///
///   * DEVELOPMENT_ONLY(code) - 仅当断言级别设置为development或更高时，才会评估代码。
///
///   * DEBUG_ASSERT(predicate) - 有效地调用 assert（谓词）。
///
///   * DEVELOPMENT_ASSERT(predicate) - 如果 assertion 设置为 development 或更高级别，则引发异常，并且 predicate 的计算结果为 false。
///
///   * RELEASE_ASSERT(predicate) -如果 predicate 的计算结果为 false，则引发异常。

#pragma once //这是一个预处理指令，用于确保当前头文件在当前编译单元中只被包含（include）一次。

// 包含CARLA项目中定义的异常类的头文件。
// 这个头文件可能包含了一些自定义异常类，用于在CARLA项目中处理错误情况。
#include "carla/Exception.h"

// 包含标准库中的stdexcept头文件。
// stdexcept头文件定义了一组标准异常类，这些类用于报告标准库函数中的错误情况。
// 这些异常类包括逻辑错误（如std::invalid_argument, std::out_of_range等）和运行时错误（如std::bad_alloc等）。
#include <stdexcept>

// 定义不同断言级别的宏，用于控制CARLA库中的断言行为。
// 这些级别允许开发者根据编译目标（如调试、开发或发布版本）来调整断言的严格程度。

// LIBCARLA_ASSERT_LEVEL_DEBUG：定义调试版本的断言级别为30。
// 在调试版本中，断言级别最高，意味着会执行最多的断言检查，以帮助开发者发现和修复问题。
#define LIBCARLA_ASSERT_LEVEL_DEBUG        30
// LIBCARLA_ASSERT_LEVEL_DEVELOPMENT：定义开发版本的断言级别为20。
// 在开发版本中，断言级别适中，提供足够的检查以确保代码在开发过程中按预期运行，但不会过于繁琐。
#define LIBCARLA_ASSERT_LEVEL_DEVELOPMENT  20
// LIBCARLA_ASSERT_LEVEL_RELEASE：定义发布版本的断言级别为10。
// 在发布版本中，断言级别最低，以减少运行时检查的数量，从而提高性能。
// 通常，发布版本中的断言主要用于捕获严重的逻辑错误，而不是用于调试目的。
#define LIBCARLA_ASSERT_LEVEL_RELEASE      10

// 如果没有显式设置 LIBCARLA_ASSERT_LEVEL，则根据是否定义了 NDEBUG 来选择断言级别
// 检查是否已定义了LIBCARLA_ASSERT_LEVEL宏。
// 如果没有定义，则根据编译环境（是否定义了NDEBUG）来选择合适的断言级别。
#ifndef LIBCARLA_ASSERT_LEVEL
// 检查是否定义了NDEBUG宏。
// NDEBUG是一个标准预处理宏，通常在发布版本的编译中定义，以禁用assert()宏。
#  ifdef NDEBUG
// 如果定义了NDEBUG，则设置断言级别为开发版本级别。
// 这意味着在发布版本中，我们希望减少断言的数量，但仍然保留一些基本的检查。
#    define LIBCARLA_ASSERT_LEVEL LIBCARLA_ASSERT_LEVEL_DEVELOPMENT
#  else
// 如果没有定义NDEBUG，则设置断言级别为调试版本级别。
// 在调试版本中，我们希望执行尽可能多的断言检查，以帮助开发者发现和修复问题。
#    define LIBCARLA_ASSERT_LEVEL LIBCARLA_ASSERT_LEVEL_DEBUG
#  endif // 结束NDEBUG的检查
#endif // 结束LIBCARLA_ASSERT_LEVEL的检查

// 如果断言级别大于或等于调试级别，且没有定义 NDEBUG（不在发布模式），则启用断言
#if (LIBCARLA_ASSERT_LEVEL >= LIBCARLA_ASSERT_LEVEL_DEBUG)
#  ifdef NDEBUG
#    error Cannot have debug asserts with NDEBUG enabled.
#  endif
#  include <cassert>
#endif // NDEBUG

// 如果断言级别大于或等于调试级别，定义 DEBUG_ONLY 宏：仅在调试模式下有效
#if (LIBCARLA_ASSERT_LEVEL >= LIBCARLA_ASSERT_LEVEL_DEBUG)
#  define DEBUG_ONLY(code) code
#else
#  define DEBUG_ONLY(code)
#endif

// 如果断言级别大于或等于开发级别，定义 DEVELOPMENT_ONLY 宏：仅在开发模式下有效
#if (LIBCARLA_ASSERT_LEVEL >= LIBCARLA_ASSERT_LEVEL_DEVELOPMENT)
#  define DEVELOPMENT_ONLY(code) code
#else
#  define DEVELOPMENT_ONLY(code)
#endif

// 定义用于调试断言的宏
#define DEBUG_ASSERT(predicate) DEBUG_ONLY(assert(predicate));

#define DEBUG_ERROR DEBUG_ASSERT(false);

// 如果启用了 Google Test (LIBCARLA_WITH_GTEST)，则使用 GTest 的断言功能
#ifdef LIBCARLA_WITH_GTEST
#  include <gtest/gtest.h>

// 通过 GTest 的 EXPECT_EQ 和 EXPECT_NE 定义调试断言
#  define DEBUG_ASSERT_EQ(lhs, rhs) DEBUG_ONLY(EXPECT_EQ(lhs, rhs));DEBUG_ASSERT(lhs == rhs);
#  define DEBUG_ASSERT_NE(lhs, rhs) DEBUG_ONLY(EXPECT_NE(lhs, rhs));DEBUG_ASSERT(lhs != rhs);
#else
// 如果没有启用 GTest，使用标准的调试断言来检查等于和不等于
#  define DEBUG_ASSERT_EQ(lhs, rhs) DEBUG_ASSERT((lhs) == (rhs))
#  define DEBUG_ASSERT_NE(lhs, rhs) DEBUG_ASSERT((lhs) != (rhs))
#endif // LIBCARLA_WITH_GTEST

#define LIBCARLA_ASSERT_THROW__(pred, msg)  if (!(pred)) { ::carla::throw_exception(std::runtime_error(msg)); }

#define DEVELOPMENT_ASSERT(pred) DEVELOPMENT_ONLY(LIBCARLA_ASSERT_THROW__(pred, #pred))

#define RELEASE_ASSERT(pred) LIBCARLA_ASSERT_THROW__(pred, #pred)
