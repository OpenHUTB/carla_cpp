// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
// 预处理指令，保证该头文件在整个项目的编译过程中只会被包含一次，防止重复定义等问题
#include "carla/Buffer.h"  // 引入carla项目中名为Buffer的头文件，大概率其中定义了用于存储数据的缓冲区相关的类、结构体或者函数等内容
#include "carla/rpc/Transform.h"  // 引入carla项目中rpc模块下关于Transform（可能表示坐标变换等相关概念）的头文件，应该包含了Transform相关的类型定义、操作函数等内容

namespace carla {
namespace sensor {
namespace s11n {

  ///  类的功能注释，说明这个类用于对所有传感器数据附带的元信息（头部信息）进行序列化操作
  class SensorHeaderSerializer {
  public:

#pragma pack(push, 1)
// 这是一个预处理指令，用于设置结构体的字节对齐方式。这里将对齐方式设置为1字节对齐，这样结构体成员在内存中的布局会更紧凑，按照定义顺序依次紧密排列，常用于和二进制数据交互等场景，保证结构体在内存中的布局和后续要处理的实际数据格式一致（比如网络传输或者文件存储格式）

    struct Header {
      uint64_t sensor_type; // 定义一个64位无符号整数类型的成员变量
      uint64_t frame; // 同样是64位无符号整数类型，
      double timestamp; // 定义一个双精度浮点数类型的成员变量，用于存储时间戳信息，代表传感器采集该数据的时间点
      rpc::Transform sensor_transform;// 包含了rpc模块中定义的Transform类型的成员变量，用于存储传感器的坐标变换相关信息
    };
#pragma pack(pop)
// 恢复之前的默认字节对齐方式，结束由#pragma pack(push, 1)设置的紧凑对齐状态

    constexpr static auto header_offset = sizeof(Header);
// 使用constexpr关键字定义一个静态常量，其值为Header结构体的大小（字节数）
    static Buffer Serialize(
        uint64_t index,
        uint64_t frame,
        double timestamp,
        rpc::Transform transform);
// 定义一个静态函数Serialize，用于将传入的传感器相关索引
    static const Header &Deserialize(const Buffer &message) {
      return *reinterpret_cast<const Header *>(message.data());
    }
// 定义一个静态函数Deserialize，用于将传入的Buffer类型的消息数据
// 进行反序列化操作，通过将缓冲区的起始地址（message.data()）强制转换为指向Header结构体的指针类型（reinterpret_cast），并返回其引用，从而获取到其中解析出来的头部信息结构体，便于后续使用其中的各个成员变量进行相应处理
  };

} // namespace s11n
} // namespace sensor
} // namespace carla
