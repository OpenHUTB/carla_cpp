// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once// 防止头文件被多次包含

#include "carla/Debug.h"// 引入调试相关的头文件

#include <boost/date_time/posix_time/posix_time_types.hpp>// 引入 Boost 的时间处理功能

#include <chrono>

namespace carla {

  /// Positive time duration up to milliseconds resolution. Automatically casts
  /// between std::chrono::duration and boost::posix_time::time_duration.
  class time_duration {
  public:
// 静态成员函数，用于创建一个表示指定秒数的时间间隔
    static inline time_duration seconds(size_t timeout) {
      return std::chrono::seconds(timeout);
    }
// 静态成员函数，用于创建一个表示指定毫秒数的时间间隔
    static inline time_duration milliseconds(size_t timeout) {
      return std::chrono::milliseconds(timeout);
    }
// 默认构造函数，创建一个表示 0 毫秒的时间间隔
    constexpr time_duration() noexcept : _milliseconds(0u) {}

// 模板构造函数，接受一个 std::chrono::duration 类型的参数，将其转换为以毫秒为单位的时间间隔
    template <typename Rep, typename Period>
    time_duration(std::chrono::duration<Rep, Period> duration)
      : _milliseconds([=]() {
      	// 将传入的 duration 转换为毫秒数，并进行断言确保结果非负
          const auto count = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
          DEBUG_ASSERT(count >= 0);
          return static_cast<size_t>(count);
        }()) {}
// 构造函数，接受一个 boost::posix_time::time_duration 类型的参数，将其转换为以毫秒为单位的时间间隔
    time_duration(boost::posix_time::time_duration timeout)
      : time_duration(std::chrono::milliseconds(timeout.total_milliseconds())) {}
// 拷贝构造函数，使用默认实现
    time_duration(const time_duration &) = default;
    time_duration &operator=(const time_duration &) = default;
// 将当前时间间隔转换为 boost::posix_time::time_duration 类型
    boost::posix_time::time_duration to_posix_time() const {
      return boost::posix_time::milliseconds(_milliseconds);
    }
// 将当前时间间隔转换为 std::chrono::milliseconds 类型
    constexpr auto to_chrono() const {
      return std::chrono::milliseconds(_milliseconds);
    }
// 类型转换运算符，将当前对象转换为 boost::posix_time::time_duration 类型
    operator boost::posix_time::time_duration() const {
      return to_posix_time();
    }

    constexpr size_t milliseconds() const noexcept {
      return _milliseconds;
    }

  private:
// 存储时间间隔的毫秒数
    size_t _milliseconds;
  };

} // namespace carla
