// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <chrono>
#include <string>
#include <unordered_map>

namespace carla {
namespace traffic_manager {

namespace chr = std::chrono;
using namespace chr;
// 定义一个时间点类型，以系统时钟和纳秒精度表示
using TimePoint = chr::time_point<chr::system_clock, chr::nanoseconds>;

// This class can be used to measure execution time, total call duration and number of calls
// of any code snippet by assigning it a name.
class SnippetProfiler {

private:
// 存储打印时间点的哈希表，键为代码片段名称，值为时间点
  std::unordered_map<std::string, TimePoint> print_clocks;
   // 存储代码片段不同时间点的哈希表
  std::unordered_map<std::string, TimePoint> snippet_clocks;
  std::unordered_map<std::string, chr::duration<float>> snippet_durations;
  std::unordered_map<std::string, unsigned long> number_of_calls;

public:
  SnippetProfiler(){};
// 测量代码片段的执行时间
  void MeasureExecutionTime(std::string snippet_name, bool begin_or_end) {
    TimePoint current_time = chr::system_clock::now();
 // 如果打印时间点哈希表中没有该代码片段的记录，则插入当前时间点
    if (print_clocks.find(snippet_name) == print_clocks.end()) {
      print_clocks.insert({snippet_name, current_time});
    }
	// 如果代码片段开始时间点哈希表中没有该代码片段的记录，则插入当前时间点
    if (snippet_clocks.find(snippet_name) == snippet_clocks.end()) {
      snippet_clocks.insert({snippet_name, current_time});
    }
	 // 如果代码片段持续时间哈希表中没有该代码片段的记录，则插入 0 持续时间
    if (snippet_durations.find(snippet_name) == snippet_durations.end()) {
      snippet_durations.insert({snippet_name, chr::duration<float>()});
    }
	// 如果代码片段调用次数哈希表中没有该代码片段的记录，则插入 0 次调用
    if (number_of_calls.find(snippet_name) == number_of_calls.end()) {
      number_of_calls.insert({snippet_name, 0u});
    }
// 获取代码片段的打印时间点、开始时间点、持续时间和调用次数的引用
    TimePoint &print_clock = print_clocks.at(snippet_name);
    TimePoint &snippet_clock = snippet_clocks.at(snippet_name);
    chr::duration<float> &snippet_duration = snippet_durations.at(snippet_name);
    unsigned long &call_count = number_of_calls.at(snippet_name);
// 如果参数 begin_or_end 为 true，表示开始测量，将当前时间点设置为代码片段开始时间点
    if (begin_or_end) {
      snippet_clock = current_time;
    } else {
      chr::duration<float> measured_duration = current_time - snippet_clock;
      snippet_duration += measured_duration;
      ++call_count;
    }

    chr::duration<float> print_duration = current_time - print_clock;
    if (print_duration.count() > 1.0f) {
		// 如果调用次数为 0，则设置为 1，避免除以 0 的错误
      call_count = call_count == 0u ? 1 : call_count;
      std::cout << "Snippet name : " << snippet_name << ", "
                << "avg. duration : " << 1000 * snippet_duration.count() / call_count << " ms, "
                << "total duration : " << snippet_duration.count() << " s, "
                << "total calls : " << call_count << ", "
                << std::endl;
// 重置总持续时间和调用次数
      snippet_duration = 0s;
      call_count = 0u;
// 更新打印时间点
      print_clock = current_time;
    }
  }
};

} // namespace traffic_manager
} // namespace carla
