// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

// #include "carla/Logging.h" // 引入CARLA的日志模块（暂时注释掉） 
#include "carla/Buffer.h" // 引入CARLA的缓冲区模块
#include "carla/multigpu/commands.h" // 引入多GPU命令模块
#include <functional> // 引入函数对象的头文件

namespace carla { // CARLA命名空间
namespace multigpu { // 多GPU子命名空间

class Secondary; // 前向声明Secondary类

class SecondaryCommands { // SecondaryCommands类定义
  public:

  using callback_type = std::function<void(MultiGPUCommand, carla::Buffer)>; // 定义回调函数类型

  void set_secondary(std::shared_ptr<Secondary> secondary); // 设置Secondary对象的共享指针
  void set_callback(callback_type callback); // 设置回调函数
  void process_command(Buffer buffer); // 处理命令，接受一个缓冲区作为参数

  private:
  std::shared_ptr<Secondary>  _secondary; // 存储Secondary对象的共享指针
  callback_type               _callback; // 存储回调函数
};

} // namespace multigpu
} // namespace carla
