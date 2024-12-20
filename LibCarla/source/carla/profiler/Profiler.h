// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once // 防止头文件重复包含

#ifndef LIBCARLA_ENABLE_PROFILER // 如果没有启用性能分析器
#  define CARLA_PROFILE_SCOPE(context, profiler_name) // 定义宏，空操作
#  define CARLA_PROFILE_FPS(context, profiler_name) // 定义宏，空操作，这里的代码将用于开始或更新一个性能分析器，与给定的上下文和名称相关，它包括获取当前时间戳，更新帧率统计，或者开始一个新的性能分析区间
#else

#include "carla/StopWatch.h" // 引入计时器头文件

#include <algorithm> // 包含算法库，用于常用算法
#include <limits> // 包含极限值定义
#include <string> // 包含字符串库

namespace carla { // 开始 carla 命名空间
namespace profiler { // 开始 profiler 命名空间
namespace detail { // 开始 detail 命名空间

  class ProfilerData { // 性能数据类
  public:

    explicit ProfilerData(std::string name, bool print_fps = false) // 构造函数，初始化名称和是否打印FPS
      : _name(std::move(name)), // 移动名称
        _print_fps(print_fps) {} // 设置打印FPS布尔值

    ~ProfilerData(); // 析构函数声明

    void Annotate(const StopWatch &stop_watch) { // 注解方法，记录时间数据
      const auto elapsed_microseconds = stop_watch.GetElapsedTime<std::chrono::microseconds>(); // 获取经过的微秒数
      ++_count; // 计数增加
      _total_microseconds += elapsed_microseconds; // 总时间增加
      _max_elapsed = std::max(elapsed_microseconds, _max_elapsed); // 更新最大时间
      _min_elapsed = std::min(elapsed_microseconds, _min_elapsed); // 更新最小时间
    }

    float average() const { // 计算平均时间
      return ms(_total_microseconds) / static_cast<float>(_count); // 将总时间（微秒）转换为毫秒，然后除以事件发生的次数，得到平均每次事件的毫秒数
    }

    float maximum() const { // 获取最大时间
      return ms(_max_elapsed); // 返回最大时间（毫秒）
    }

    float minimum() const { // 获取最小时间
      return ms(_min_elapsed); // 返回最小时间（毫秒）
    }

  private:

    static inline float ms(size_t microseconds) { // 将微秒转换为毫秒
      return 1e-3f * static_cast<float>(microseconds); // 转换公式
    }

    static inline float fps(float milliseconds) { // 根据毫秒计算FPS
      return milliseconds > 0.0f ? (1e3f / milliseconds) : std::numeric_limits<float>::max(); // 如果 milliseconds 大于 0，则返回其倒数（每秒），否则返回无穷大
    }

    const std::string _name; // 性能监测名称

    const bool _print_fps; // 是否打印FPS

    size_t _count = 0u; // 记录次数

    size_t _total_microseconds = 0u; // 总微秒数

    size_t _max_elapsed = 0u; // 最大经历的时间

    size_t _min_elapsed = std::numeric_limits<size_t>::max(); // 最小经历的时间（初始化为最大值）
  };

  class ScopedProfiler { // 作用域性能分析类
  public:

    explicit ScopedProfiler(ProfilerData &parent) : _profiler(parent) {} // 构造函数，接受父级性能数据引用

    ~ScopedProfiler() { // 析构函数
      _stop_watch.Stop(); // 停止计时器
      _profiler.Annotate(_stop_watch); // 使用_stop_watch对象的信息来标注或更新_profiler的性能分析数据
    }

  private:

    ProfilerData &_profiler; // 引用性能数据对象

    StopWatch _stop_watch; // 计时器对象
  };

} // namespace detail
} // namespace profiler
} // namespace carla

#ifdef LIBCARLA_WITH_GTEST // 如果启用了 GTest
#  define LIBCARLA_GTEST_GET_TEST_NAME() std::string(::testing::UnitTest::GetInstance()->current_test_info()->name()) // 获取当前测试名称
#else
#  define LIBCARLA_GTEST_GET_TEST_NAME() std::string("") // 定义一个宏，用于获取当前测试的名称，但当前实现仅返回一个空字符串
#endif // LIBCARLA_WITH_GTEST

#define CARLA_PROFILE_SCOPE(context, profiler_name) \ // 定义性能分析作用域宏
    static thread_local ::carla::profiler::detail::ProfilerData carla_profiler_ ## context ## _ ## profiler_name ## _data( \ // 创建性能数据实例
        LIBCARLA_GTEST_GET_TEST_NAME() + "." #context "." #profiler_name); \ // 初始化性能数据名称
    ::carla::profiler::detail::ScopedProfiler carla_profiler_ ## context ## _ ## profiler_name ## _scoped_profiler( \ // 创建作用域性能分析器，创建一个 ScopedProfiler实例，用于性能分析
        carla_profiler_ ## context ## _ ## profiler_name ## _data); // 关联到性能数据

#define CARLA_PROFILE_FPS(context, profiler_name) \ // 定义性能分析FPS宏
    { \ // 开始代码块
      static thread_local ::carla::StopWatch stop_watch; \ // 创建计时器实例
      stop_watch.Stop(); \ // 停止计时器
      static thread_local bool first_time = true; \ // 首次标志变量
      if (!first_time) { \ // 如果不是第一次
        static thread_local ::carla::profiler::detail::ProfilerData profiler_data( \ // 创建性能数据实例
            LIBCARLA_GTEST_GET_TEST_NAME() + "." #context "." #profiler_name, true); \ // 初始化名称并设置打印FPS为真
        profiler_data.Annotate(stop_watch); \ // 注解计时器数据
      } else { \ // 如果是第一次
        first_time = false; \ // 更新首次标志
      } \
      stop_watch.Restart(); \ // 重新启动计时器
    }

#endif // LIBCARLA_ENABLE_PROFILER 
