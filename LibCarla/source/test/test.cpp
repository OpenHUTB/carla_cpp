// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

// 如果定义了LIBCARLA_NO_EXCEPTIONS这个宏，以下代码段才会被编译
#ifdef LIBCARLA_NO_EXCEPTIONS

#include <carla/Exception.h>
#include <carla/Logging.h>

#include <exception>

// 定义在carla命名空间下
namespace carla {
  // 函数功能：处理异常情况
  // 参数：接受一个std::exception类型的常引用，表示捕获到的异常对象
  // 当异常被禁用时（通过LIBCARLA_NO_EXCEPTIONS宏控制），这个函数会执行以下操作：
  // 1. 记录一个严重级别的日志，日志内容包含异常的描述信息（通过e.what()获取），方便后续排查问题，知晓是何种异常触发了此情况。
  // 2. 再记录一个严重级别的日志，告知因为异常被禁用了，所以要进行下一步特殊处理。
  // 3. 调用std::terminate()函数来终止整个程序的执行，因为在异常被禁用的情况下，无法按照常规的异常处理流程来处理异常，只能强行终止程序。
  void throw_exception(const std::exception &e) {
    // 记录一个严重级别的日志，包含异常的描述信息
    log_critical("carla::throw_exception:", e.what());
    // 记录一个严重级别的日志，说明因为异常被禁用
    log_critical("calling std::terminate because exceptions are disabled."); 
    std::terminate();//终止程序的执行
  }

} // namespace carla

#endif // LIBCARLA_NO_EXCEPTIONS

// 包含Random.h头文件，从名字推测可能和随机数相关功能有关，但具体内容未知，需要查看该头文件定义
#include "Random.h"

// 定义在util命名空间下
namespace util {

  // 声明一个线程局部（thread_local）的静态成员变量 _engine，类型为std::mt19937_64（这是C++标准库中定义的一种基于梅森旋转算法的64位伪随机数生成器）。
  // 它使用std::random_device()作为种子来初始化，std::random_device通常用于获取一个真正的随机种子（依赖于操作系统提供的随机源，比如硬件的随机数生成器等），这样可以保证每次生成的随机数序列起始状态不同，更具随机性。
  thread_local std::mt19937_64 Random::_engine((std::random_device())());

} // namespace util
