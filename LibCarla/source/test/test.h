// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// 本作品采用MIT许可证授权。
// 如需副本, 请访问 <https://opensource.org/licenses/MIT>.

#pragma once  // 保证头文件只被包含一次
// 如果不是在调试模式下 (即在Release模式下)，定义日志等级为信息级别
#ifndef NDEBUG
#  define LIBCARLA_LOG_LEVEL LIBCARLA_LOG_LEVEL_INFO
#endif // NDEBUG

#include "test/Buffer.h"  // 包含自定义的Buffer头文件

#include <carla/Logging.h>  // 引入CARLA日志相关的头文件
#include <carla/profiler/Profiler.h>  // 引入CARLA性能分析器相关的头文件

#include <gtest/gtest.h>   // 引入Google Test框架的头文件

#include <chrono>  // 引入C++11的时间库，用于时间相关的操作
#include <cstdint>   // 引入C++标准库中的整数类型定义，提供了如uint16_t的定义
#include <iostream>  // 引入C++的输入输出流库，用于标准输出

constexpr uint16_t TESTING_PORT = 0u;  // 定义一个常量值TESTING_PORT，类型为uint16_t，初始值为0
