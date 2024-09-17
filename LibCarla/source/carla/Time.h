// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

<<<<<<< HEAD
#pragma once // 确保头文件只被包含一次

#include "carla/Debug.h" // 包含自定义的调试工具

#include <boost/date_time/posix_time/posix_time_types.hpp> // 包含 Boost 的 POSIX 时间类型
=======
#pragma once// 防止头文件被多次包含

#include "carla/Debug.h"// 引入调试相关的头文件

#include <boost/date_time/posix_time/posix_time_types.hpp>// 引入 Boost 的时间处理功能
>>>>>>> 4abc25176ac5d5930e03168b4e3d67d59cc132e7

#include <chrono> // 包含标准库中的时间处理支持

namespace carla {

  /// 正时间持续时间，精确到毫秒。可以自动在 std::chrono::duration 和 boost::posix_time::time_duration 之间转换。
  class time_duration {
  public:
// 静态成员函数，用于创建一个表示指定秒数的时间间隔
    static inline time_duration seconds(size_t timeout) {
      // 返回一个初始化为指定秒数的 time_duration 对象
      return std::chrono::seconds(timeout);
    }
// 静态成员函数，用于创建一个表示指定毫秒数的时间间隔
    static inline time_duration milliseconds(size_t timeout) {
      // 返回一个初始化为指定毫秒数的 time_duration 对象
      return std::chrono::milliseconds(timeout);
    }
// 默认构造函数，创建一个表示 0 毫秒的时间间隔
    constexpr time_duration() noexcept : _milliseconds(0u) {}
      // 默认构造函数，将持续时间初始化为 0 毫秒

// 模板构造函数，接受一个 std::chrono::duration 类型的参数，将其转换为以毫秒为单位的时间间隔
    template <typename Rep, typename Period>
    time_duration(std::chrono::duration<Rep, Period> duration)
      : _milliseconds([=]() {
<<<<<<< HEAD
          // 将任意 std::chrono::duration 转换为毫秒
=======
      	// 将传入的 duration 转换为毫秒数，并进行断言确保结果非负
>>>>>>> 4abc25176ac5d5930e03168b4e3d67d59cc132e7
          const auto count = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
          DEBUG_ASSERT(count >= 0); // 断言持续时间是非负的
          return static_cast<size_t>(count); // 转换为 size_t 类型
        }()) {}
// 构造函数，接受一个 boost::posix_time::time_duration 类型的参数，将其转换为以毫秒为单位的时间间隔
    time_duration(boost::posix_time::time_duration timeout)
      : time_duration(std::chrono::milliseconds(timeout.total_milliseconds())) {}
<<<<<<< HEAD
      // 从 Boost 的 time_duration 初始化，将其转换为毫秒

    time_duration(const time_duration &) = default;
    time_duration &operator=(const time_duration &) = default;
      // 默认的复制构造函数和赋值运算符

=======
// 拷贝构造函数，使用默认实现
    time_duration(const time_duration &) = default;
    time_duration &operator=(const time_duration &) = default;
// 将当前时间间隔转换为 boost::posix_time::time_duration 类型
>>>>>>> 4abc25176ac5d5930e03168b4e3d67d59cc132e7
    boost::posix_time::time_duration to_posix_time() const {
      // 将内部的毫秒转换为 Boost 的 time_duration
      return boost::posix_time::milliseconds(_milliseconds);
    }
// 将当前时间间隔转换为 std::chrono::milliseconds 类型
    constexpr auto to_chrono() const {
      // 将内部持续时间作为 std::chrono::milliseconds 返回
      return std::chrono::milliseconds(_milliseconds);
    }
// 类型转换运算符，将当前对象转换为 boost::posix_time::time_duration 类型
    operator boost::posix_time::time_duration() const {
      // 隐式转换操作符，将内部持续时间转换为 Boost 的 time_duration
      return to_posix_time();
    }

    constexpr size_t milliseconds() const noexcept {
      // 返回持续时间的毫秒数
      return _milliseconds;
    }

  private:
<<<<<<< HEAD

    size_t _milliseconds; // 用于存储时间持续时间的毫秒数
=======
// 存储时间间隔的毫秒数
    size_t _milliseconds;
>>>>>>> 4abc25176ac5d5930e03168b4e3d67d59cc132e7
  };

} // namespace carla
