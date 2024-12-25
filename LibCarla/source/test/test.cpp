//版权所有(c) 2017，巴塞罗那自治大学（UAB）的计算机视觉中心（CVC）
//本作品依据 MIT 许可证的条款获得许可
// 如需获取副本，请访问<https://opensource.org/licenses/MIT>.


#ifdef LIBCARLA_NO_EXCEPTIONS
#ifdef LIBCARLA_NO_EXCEPTIONS // 如果定义了LIBCARLA_NO_EXCEPTIONS宏，
// 则表示CARLA库被配置为不使用异常处理机制。

// 包含CARLA异常处理相关的头文件，但在此场景下，这些头文件的内容可能仅用于日志记录或定义异常类型，
// 因为异常处理本身被禁用了。
#include <carla/Exception.h>

// 包含CARLA日志记录相关的头文件，用于在不使用异常的情况下记录错误和关键信息。
#include <carla/Logging.h>

// 包含C++标准库中的异常处理头文件，但在此场景下，我们不会抛出异常，而是会调用std::terminate来终止程序。
#include <exception>

// 定义在carla命名空间下
namespace carla {
  // 函数功能：处理异常情况
  // 参数：接受一个std::exception类型的常引用，表示捕获到的异常对象
  // 当异常被禁用时（通过LIBCARLA_NO_EXCEPTIONS宏控制），这个函数会执行以下操作：
  // 1. 记录一个严重级别的日志，日志内容包含异常的描述信息（通过e.what()获取），方便后续排查问题，知晓是何种异常触发了此情况。
  // 2. 再记录一个严重级别的日志，告知因为异常被禁用了，所以要进行下一步特殊处理。
  // 3. 调用std::terminate()函数来终止整个程序的执行，因为在异常被禁用的情况下，无法按照常规的异常处理流程来处理异常，只能强行终止程序。
  // 定义一个函数，用于处理异常（尽管在这个配置下，异常实际上不会被抛出，但这个函数提供了一种错误处理机制）。
  // 当异常被禁用时，这个函数会记录错误信息，并终止程序的执行。
  void throw_exception(const std::exception &e) {
    // 记录一个严重级别的日志，包含异常的描述信息，帮助开发者了解发生了什么错误。
    log_critical("carla::throw_exception:", e.what());
    
    // 记录另一个严重级别的日志，明确说明因为异常被禁用，程序将调用std::terminate来终止执行。
    // 这有助于开发者理解程序为何突然终止。
    log_critical("calling std::terminate because exceptions are disabled."); 
    
    // 调用std::terminate来终止程序的执行。这是在不使用异常处理机制时的一种错误处理方式。
    std::terminate();
  }

} // namespace carla

#endif // LIBCARLA_NO_EXCEPTIONS


// 包含Random.h头文件，从名字推测可能和随机数相关功能有关，但具体内容未知，需要查看该头文件定义
// 包含Random类的头文件，该类提供了随机数生成的功能。

#include "Random.h"

// 定义在util命名空间下
namespace util {

  // 声明一个线程局部（thread_local）的静态成员变量 _engine，类型为std::mt19937_64（这是C++标准库中定义的一种基于梅森旋转算法的64位伪随机数生成器）。
  // 它使用std::random_device()作为种子来初始化，std::random_device通常用于获取一个真正的随机种子（依赖于操作系统提供的随机源，比如硬件的随机数生成器等），这样可以保证每次生成的随机数序列起始状态不同，更具随机性。
  // 定义一个线程局部变量_engine，其类型为std::mt19937_64，这是一个基于梅森旋转算法的64位伪随机数生成器。
  // 每个线程都会有一个独立的_engine实例，确保随机数生成的线程安全性。
  // 使用std::random_device来初始化_engine，以获取一个高质量的随机种子。
  // std::random_device是一个能够生成非确定性随机数的类，通常用于初始化随机数生成器的种子。
  thread_local std::mt19937_64 Random::_engine((std::random_device())());

  // 注意：这里只定义了_engine变量，并没有展示Random类的其他部分（如成员函数等）。
  // Random类可能提供了获取随机数的方法，这些方法内部会使用_engine来生成随机数。
} // namespace util
