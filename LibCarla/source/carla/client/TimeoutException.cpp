s// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/client/TimeoutException.h" // 引入TimeoutException头文件

namespace carla { // 定义carla命名空间
namespace client { // 定义client子命名空间

  using namespace std::string_literals; // 使用字符串字面量的命名空间

  TimeoutException::TimeoutException( // TimeoutException构造函数
      const std::string &endpoint, // 输入参数：端点地址
      time_duration timeout) // 输入参数：超时时间
    : std::runtime_error( // 初始化基类std::runtime_error
        "time-out of "s + std::to_string(timeout.milliseconds()) + // 构建错误信息，包含超时时间
        "ms while waiting for the simulator, " // 提示信息，等待模拟器时发生超时
        "make sure the simulator is ready and connected to " + endpoint) {} // 提示用户确保模拟器已就绪并连接到指定端点

} // namespace client
} // namespace carla

