// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

// 检查是否已定义LIBCARLA_ENABLE_PROFILER宏，如果没有，则定义它
// 这个宏用于控制是否启用Carla的性能分析器功能
#ifndef LIBCARLA_ENABLE_PROFILER
#  define LIBCARLA_ENABLE_PROFILER
#endif // LIBCARLA_ENABLE_PROFILER

// 引入Carla日志库的头文件，用于记录日志信息
#include "carla/Logging.h"

// 引入Carla版本信息的头文件，可能包含版本号、构建日期等信息
#include "carla/Version.h"

// 引入Carla性能分析器的头文件，包含性能分析相关的类和函数定义
#include "carla/profiler/Profiler.h"

// 引入C++标准库中的文件流处理头文件，用于读写文件
#include <fstream>

// 引入C++标准库中的格式化输入输出头文件，用于设置输出格式
#include <iomanip>

// 引入C++标准库中的标准输入输出流头文件，用于控制台输入输出
#include <iostream>

// 引入C++标准库中的互斥锁头文件，用于实现线程同步
#include <mutex>

// 定义在carla::profiler::detail命名空间下的代码
namespace carla {
namespace profiler {
namespace detail {

// 定义一个模板辅助函数，用于将参数写入CSV（逗号分隔值）格式的输出流中
// 这个函数可以接受一个或多个参数，并将它们按照指定的格式输出到提供的输出流中
template <typename Arg, typename ... Args>
static void write_csv_to_stream(std::ostream &out, Arg &&arg, Args &&... args) {
    // 以布尔值形式输出（true/false而不是1/0）
    // 设置左对齐，字段宽度为44个字符
    // 转发第一个参数到输出流中（支持左值引用和右值引用）
    out << std::boolalpha << std::left << std::setw(44) << std::forward<Arg>(arg);
    
    // 设置右对齐，并固定浮点数的小数点位数为2位
    out << std::right << std::fixed << std::setprecision(2);
    
    // 定义一个用于参数包展开的数组类型（这里实际上不会创建数组，只是利用数组初始化语法来展开参数包）
    // 展开参数包，对每个剩余参数执行输出操作，并在参数之间添加逗号和空格作为分隔符
    // 使用逗号表达式来同时执行输出操作和数组初始化（这里数组初始化只是为了产生编译时的副作用，即展开参数包）
    using expander = int[];
    (void)expander{0, (void(out << ", " << std::setw(10) << std::forward<Args>(args)), 0)...};
}
// 静态性能分析器类
class StaticProfiler {
public:

    // 构造函数，接受文件名
    StaticProfiler(std::string filename)
      : _filename(std::move(filename)) { // 移动构造文件名
        logging::log("PROFILER: writing profiling data to", _filename); // 日志记录
        std::string header = "# LibCarla Profiler "; // CSV头部信息
        header += carla::version(); // 添加版本信息
#ifdef NDEBUG
        header += " (release)"; // 如果是发布模式
#else
        header += " (debug)"; // 如果是调试模式
#endif // NDEBUG
        write_to_file(std::ios_base::out, header); // 写入文件头
        write_line("# context", "average", "maximum", "minimum", "units", "times"); // 写入列名
    }

    // 写入一行数据
    template <typename ... Args>
    void write_line(Args &&... args) {
      write_to_file(std::ios_base::app|std::ios_base::out, std::forward<Args>(args)...); // 追加写入数据
    }

private:

    // 向文件写入数据的私有函数
    template <typename ... Args>
    void write_to_file(std::ios_base::openmode mode, Args &&... args) {
      if (!_filename.empty()) { // 确保文件名不为空
        static std::mutex MUTEX; // 静态互斥锁
        std::lock_guard<std::mutex> guard(MUTEX); // 加锁，保护文件写入
        std::ofstream file(_filename, mode); // 打开文件
        write_csv_to_stream(file, std::forward<Args>(args)...); // 写入CSV格式的数据
        file << std::endl; // 换行
      }
    }

    const std::string _filename; // 保存文件名
};

// 性能数据析构函数
ProfilerData::~ProfilerData() {
    static StaticProfiler PROFILER{"profiler.csv"}; // 静态性能分析器实例
    if (_count > 0u) { // 如果调用次数大于0
      if (_print_fps) { // 检查是否打印FPS
        PROFILER.write_line(_name, fps(average()), fps(minimum()), fps(maximum()), "FPS", _count); // 写入FPS数据
      } else {
        PROFILER.write_line(_name, average(), maximum(), minimum(), "ms", _count); // 写入时间数据
      }
    } else {
      log_error("profiler", _name, " was never run!"); // 日志错误：未运行
    }
}

} // namespace detail
} // namespace profiler
} // namespace carla
