// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <string>

namespace carla {
namespace profiler {
// 定义了一个名为LifetimeProfiled的类，用于对象的生命周期性能分析
  class LifetimeProfiled {
  protected:
// 当未定义LIBCARLA_ENABLE_LIFETIME_PROFILER宏时， LifetimeProfiled类的构造函数被设置为默认构造函数， 
#ifndef LIBCARLA_ENABLE_LIFETIME_PROFILER

    LifetimeProfiled() = default;

#else
// 当定义了LIBCARLA_ENABLE_LIFETIME_PROFILER宏时，LifetimeProfiled类的构造函数接受一个std::string类型的参数display_name， 用于在性能分析中标识该对象。  
    LifetimeProfiled(std::string display_name);

  public:
 // 析构函数，用于在对象生命周期结束时执行清理工作，同时也可以在这里记录对象的销毁时间，作为性能分析的一部分
    ~LifetimeProfiled();

#endif // LIBCARLA_ENABLE_LIFETIME_PROFILER
  };

} // namespace profiler
} // namespace carla
// 根据是否定义了LIBCARLA_ENABLE_LIFETIME_PROFILER宏，  
// LIBCARLA_INITIALIZE_LIFETIME_PROFILER宏被用来方便地初始化LifetimeProfiled对象
#ifndef LIBCARLA_ENABLE_LIFETIME_PROFILER
#  define LIBCARLA_INITIALIZE_LIFETIME_PROFILER(display_name) ::carla::profiler::LifetimeProfiled()
// 当性能分析功能被禁用时，该宏简单地创建一个LifetimeProfiled对象, 但不使用display_name参数
#else
#  define LIBCARLA_INITIALIZE_LIFETIME_PROFILER(display_name) ::carla::profiler::LifetimeProfiled(display_name)
// 当性能分析功能被启用时，该宏使用提供的display_name参数来创建LifetimeProfiled对象，以便在性能分析中包含该对象的标识信息
#endif // LIBCARLA_ENABLE_LIFETIME_PROFILER
