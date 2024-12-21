//版权所有(c) 2017，巴塞罗那自治大学（UAB）的计算机视觉中心（CVC）
//本作品依据 MIT 许可证的条款获得许可
// 如需获取副本，请访问<https://opensource.org/licenses/MIT>.

#ifdef LIBCARLA_NO_EXCEPTIONS
// 包含CARLA异常处理相关的头文件
#include <carla/Exception.h>
// 包含CARLA日志记录相关的头文件
#include <carla/Logging.h>
// 包含C++标准库中的异常处理头文件
#include <exception>

namespace carla {
  //处理异常
  void throw_exception(const std::exception &e) {
    // 记录一个严重级别的日志，包含异常的描述信息
    log_critical("carla::throw_exception:", e.what());
    // 记录一个严重级别的日志，说明因为异常被禁用
    log_critical("calling std::terminate because exceptions are disabled."); 
    std::terminate();//终止程序的执行
  }

} // namespace carla

#endif // LIBCARLA_NO_EXCEPTIONS

#include "Random.h"

namespace util {
  // 定义一个线程局部变量 _engine，其类型为std::mt19937_64（一个基于梅森旋转算法的64位伪随机数生成器），
  // 并使用std::random_device来初始化它，以获取一个初始的随机种子
  thread_local std::mt19937_64 Random::_engine((std::random_device())());

} // namespace util
