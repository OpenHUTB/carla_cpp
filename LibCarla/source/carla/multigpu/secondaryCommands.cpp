// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

// #include "carla/Logging.h"
#include "carla/multigpu/secondaryCommands.h"
// #include "carla/streaming/detail/tcp/Message.h"

namespace carla {
namespace multigpu {
 
// 设置SecondaryCommands实例中的_secondary成员变量
// 参数：secondary - 一个指向Secondary类实例的智能指针
void SecondaryCommands::set_secondary(std::shared_ptr<Secondary> secondary) {
  _secondary = secondary;  // 将传入的智能指针赋值给_secondary成员变量
}
 
// 设置一个回调函数，当处理命令时会被调用
// 参数：callback - 一个函数指针或可调用对象，接受命令ID和Buffer作为参数
void SecondaryCommands::set_callback(callback_type callback) {
  _callback = callback;  // 将传入的回调函数赋值给_callback成员变量
  // 注意：callback_type是一个未在代码段中定义的类型，它可能是一个函数指针类型或std::function等
}
 
// 处理一个命令，从Buffer中提取命令头，然后调用设置的回调函数
// 参数：buffer - 包含命令数据的Buffer对象
void SecondaryCommands::process_command(Buffer buffer) {
  // 从Buffer中提取命令头
  // 假设CommandHeader是一个结构体，包含了命令的ID和大小等信息
  CommandHeader *header;
  header = reinterpret_cast<CommandHeader *>(buffer.data());  // 将Buffer的数据指针转换为CommandHeader指针
  
  // 创建一个新的Buffer对象，用于存储命令数据（不包括命令头）
  // 如果header->size确实包含了命令头的大小，那么下面的代码将正确地跳过命令头
  Buffer data(buffer.data() + sizeof(CommandHeader), header->size);
  
  // 调用之前设置的回调函数，传递命令ID和命令数据
  _callback(header->id, std::move(data));  // 使用std::move是为了避免不必要的拷贝
  
  // 下面的日志语句被注释掉了，如果取消注释，它将输出一条日志信息
  // log_info("Secondary got a command to process");  // 假设log_info是一个用于输出日志信息的函数
}
// 这些类型和类可能是在其他地方定义的，用于支持SecondaryCommands类的功能


}
}  // 命名空间结束
