// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#ifdef LIBCARLA_NO_EXCEPTIONS

#include <carla/Exception.h>
#include <carla/Logging.h>

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

  thread_local std::mt19937_64 Random::_engine((std::random_device())());

} // namespace util
