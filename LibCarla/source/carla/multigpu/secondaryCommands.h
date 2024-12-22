// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

// #include "carla/Logging.h" // 引入CARLA的日志模块（暂时注释掉） 
#include "carla/Buffer.h" // 引入CARLA的缓冲区模块
#include "carla/multigpu/commands.h" // 引入多GPU命令模块
#include <functional> // 引入函数对象的头文件

namespace carla { // CARLA项目的顶级命名空间
namespace multigpu { // CARLA项目中与多GPU相关功能的子命名空间

// 前向声明Secondary类，以便在SecondaryCommands类中使用其共享指针
class Secondary;

// SecondaryCommands类定义，用于管理和处理与Secondary类相关的命令和回调
class SecondaryCommands {
  public:

    // 定义回调函数类型，该函数接受一个MultiGPUCommand枚举值和一个carla::Buffer对象作为参数
    // MultiGPUCommand可能是一个枚举类型，表示不同类型的GPU命令
    // carla::Buffer是一个用于数据传输的缓冲区类
    using callback_type = std::function<void(MultiGPUCommand, carla::Buffer)>;

    // 设置Secondary对象的共享指针，以便在需要时访问Secondary类的实例
    // 这个方法允许SecondaryCommands类与Secondary类实例进行交互
    void set_secondary(std::shared_ptr<Secondary> secondary);

    // 设置回调函数，该函数将在处理完命令后被调用
    // 回调函数将接收一个MultiGPUCommand枚举值和一个包含命令数据的carla::Buffer对象
    void set_callback(callback_type callback);

    // 处理从Secondary接收到的命令
    // 这个方法接受一个包含命令数据的缓冲区作为参数，并解析命令，然后根据需要调用设置的回调函数
    void process_command(carla::Buffer buffer);

  private:
    // 存储Secondary对象的共享指针，以便在处理命令时访问Secondary类的实例
    std::shared_ptr<Secondary> _secondary;

    // 存储回调函数，以便在处理完命令后调用它
    // 回调函数将使用从命令中解析出的数据和命令类型作为参数进行调用
    callback_type _callback;
};

// 注意：MultiGPUCommand枚举类型和carla::Buffer类的定义没有在这个代码片段中给出，
// 它们可能在其他地方（如其他头文件）被定义。此外，Secondary类的定义也没有给出，
// 但通过前向声明，SecondaryCommands类可以引用Secondary类的共享指针。
} // namespace multigpu
} // namespace carla
