// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

  #pragma once // 防止头文件被多次包含，提高编译效率

#include <chrono> // 引入 chrono 库，用于时间测量功能
#include <cstdint> // 引入 cstdint 库，提供标准整数类型

namespace carla { // 定义 carla 命名空间
namespace detail { // 定义 detail 命名空间，用于封装实现细节

  template <typename CLOCK> // 模板类，允许传入不同类型的时钟
  class StopWatchTmpl {
    static_assert(CLOCK::is_steady, "The StopWatch's clock must be steady");
    // 静态断言，确保传入的时钟类型是稳定的（steady），避免时间回退问题

  public:
    using clock = CLOCK; // 定义 clock 为传入的时钟类型别名

    StopWatchTmpl() 
      : _start(clock::now()), // 初始化开始时间为当前时间
        _end(), // 初始化结束时间为默认值
        _is_running(true) {} // 初始化为秒表正在运行状态

    void Restart() {
      _is_running = true; // 设置秒表为运行状态
      _start = clock::now(); // 更新开始时间为当前时间
    }

    void Stop() {
      _end = clock::now(); // 更新结束时间为当前时间
      _is_running = false; // 设置秒表为停止状态
    }

    typename clock::duration GetDuration() const {
      // 如果秒表正在运行，返回当前时间与开始时间的差值
      // 否则，返回结束时间与开始时间的差值
      return _is_running ? clock::now() - _start : _end - _start;
    }

    template <class RESOLUTION=std::chrono::milliseconds>
    size_t GetElapsedTime() const {
      // 将时间间隔转换为指定精度的时间单位（默认为毫秒），并返回其计数值
      return static_cast<size_t>(std::chrono::duration_cast<RESOLUTION>(GetDuration()).count());
    }

    bool IsRunning() const {
      return _is_running; // 返回秒表是否正在运行的状态
    }

  private:
    typename clock::time_point _start; // 秒表开始时间点
    typename clock::time_point _end;   // 秒表结束时间点
    bool _is_running;                  // 秒表是否正在运行的状态
  };

} // namespace detail

  using StopWatch = detail::StopWatchTmpl<std::chrono::steady_clock>;
  // 实例化 StopWatchTmpl 模板，使用 std::chrono::steady_clock 作为时钟类型，定义 StopWatch 类型

} // namespace carla

