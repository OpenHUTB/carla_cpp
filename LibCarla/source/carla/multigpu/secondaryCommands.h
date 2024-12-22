// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

// 引入必要的头文件
// #include "carla/Logging.h" // 引入CARLA的日志模块（本例中未使用，因此注释掉）
#include "carla/Buffer.h" // 引入CARLA的缓冲区模块，用于数据传输和处理
#include "carla/multigpu/commands.h" // 引入多GPU命令模块，定义多GPU操作的相关命令
#include <functional> // 引入标准库中的函数对象头文件，用于定义和使用回调函数

namespace carla { // 定义CARLA命名空间
namespace multigpu { // 在CARLA命名空间下定义多GPU子命名空间

// 前向声明Secondary类，表示该类将在后续代码中定义或在其他头文件中定义
class Secondary;

// SecondaryCommands类定义，用于处理与Secondary对象相关的命令和回调
class SecondaryCommands {
public:
    // 定义回调函数类型，该函数接受一个MultiGPUCommand枚举和一个carla::Buffer对象作为参数
    using callback_type = std::function<void(MultiGPUCommand, carla::Buffer)>;

    // 设置Secondary对象的共享指针，用于在命令处理中引用和操作Secondary实例
    void set_secondary(std::shared_ptr<Secondary> secondary);

    // 设置回调函数，该回调将在处理完命令后被调用
    void set_callback(callback_type callback);

    // 处理命令函数，接受一个缓冲区作为输入，该缓冲区包含要处理的命令数据
    void process_command(carla::Buffer buffer);

private:
    // 存储Secondary对象的共享指针，用于在类中引用和操作Secondary实例
    std::shared_ptr<Secondary> _secondary;

    // 存储回调函数，以便在处理完命令后调用
    callback_type _callback;
};

} // 多GPU子命名空间结束
} // CARLA命名空间结束
} // namespace multigpu
} // namespace carla
