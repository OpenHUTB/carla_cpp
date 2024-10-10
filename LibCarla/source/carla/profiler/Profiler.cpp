// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#ifndef LIBCARLA_ENABLE_PROFILER
#  define LIBCARLA_ENABLE_PROFILER // 定义性能分析器宏
#endif // LIBCARLA_ENABLE_PROFILER

#include "carla/Logging.h" // 引入日志库
#include "carla/Version.h" // 引入版本信息
#include "carla/profiler/Profiler.h" // 引入性能分析器头文件

#include <fstream> // 引入文件流处理
#include <iomanip> // 引入格式化输入输出
#include <iostream> // 引入标准输入输出流
#include <mutex> // 引入互斥锁

namespace carla {
namespace profiler {
namespace detail {

// 将参数写入CSV流的辅助函数
template <typename Arg, typename ... Args>
static void write_csv_to_stream(std::ostream &out, Arg &&arg, Args &&... args) {
    out << std::boolalpha // 以布尔值形式输出
        << std::left << std::setw(44) // 左对齐，宽度设置为44
        << std::forward<Arg>(arg) // 转发第一个参数
        << std::right // 右对齐
        << std::fixed << std::setprecision(2); // 设置浮点数格式与精度
    using expander = int[]; // 用于展开参数包
    (void)expander{0, (void(out << ", " << std::setw(10) << std::forward<Args>(args)),0)...}; // 输出剩余参数
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
