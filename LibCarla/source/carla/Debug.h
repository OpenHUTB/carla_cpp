// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

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

#pragma once

#include "carla/Exception.h"

#include <stdexcept>

// 定义不同的断言级别
#define LIBCARLA_ASSERT_LEVEL_DEBUG        30
#define LIBCARLA_ASSERT_LEVEL_DEVELOPMENT  20
#define LIBCARLA_ASSERT_LEVEL_RELEASE      10

// 如果没有显式设置 LIBCARLA_ASSERT_LEVEL，则根据是否定义了 NDEBUG 来选择断言级别
#ifndef LIBCARLA_ASSERT_LEVEL
#  ifdef NDEBUG
#    define LIBCARLA_ASSERT_LEVEL LIBCARLA_ASSERT_LEVEL_DEVELOPMENT
#  else
#    define LIBCARLA_ASSERT_LEVEL LIBCARLA_ASSERT_LEVEL_DEBUG
#  endif // NDEBUG
#endif // LIBCARLA_ASSERT_LEVEL

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
// LIBCARLA_ASSERT_THROW__ 是一个宏，用于在断言失败时抛出异常
// 它接受两个参数：pred（一个布尔表达式）和 msg（一个错误消息字符串）
// 如果 pred 为 false（即断言失败），则调用 ::carla::throw_exception 函数抛出一个 std::runtime_error 异常，异常信息为 msg
#define DEVELOPMENT_ASSERT(pred) DEVELOPMENT_ONLY(LIBCARLA_ASSERT_THROW__(pred, #pred))
// DEVELOPMENT_ASSERT 是一个宏，用于在开发阶段进行断言检查
// 它接受一个参数：pred（一个布尔表达式）
// 在开发版本中，它使用 DEVELOPMENT_ONLY 宏来包含 LIBCARLA_ASSERT_THROW__ 宏的调用
// 而在发布版本中，DEVELOPMENT_ONLY 宏可能定义为空，因此 DEVELOPMENT_ASSERT 不会有任何效果
#define RELEASE_ASSERT(pred) LIBCARLA_ASSERT_THROW__(pred, #pred)
// RELEASE_ASSERT 是一个宏，用于在发布版本中进行断言检查
// 它与 DEVELOPMENT_ASSERT 类似，但不受 DEVELOPMENT_ONLY 宏的影响
// 因此，在发布版本中，RELEASE_ASSERT 仍然会进行断言检查，并在断言失败时抛出异常
// 注意事项：
// 1. #pred 是一个预处理操作符，用于将 pred 表达式转换为字符串字面量
//    这样，当断言失败时，抛出的异常信息将包含导致失败的表达式文本
// 2. ::carla::throw_exception 可能是一个在 carla 命名空间中定义的函数，用于统一处理异常抛出
//    这个函数可能包含了一些额外的日志记录或错误处理逻辑
// 3. DEVELOPMENT_ONLY 是一个宏，其定义取决于编译时的配置（例如，通过预处理器定义）
//    在开发版本中，它可能定义为 #define DEVELOPMENT_ONLY(x) x
//    而在发布版本中，它可能定义为 #define DEVELOPMENT_ONLY(x) /* 空操作 */ 或类似的东西
// 4. 使用这些宏时，请确保您的项目中已经定义了 carla 命名空间和 ::carla::throw_exception 函数
//    否则，编译时将出现错误
