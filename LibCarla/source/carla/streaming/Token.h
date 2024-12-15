// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.
//个预处理指令，用于确保头文件只会被编译一次。它通常用于防止头文件重复包含的问题，避免多重定义
#pragma once
//假设这个文件是一个与 MsgPack 序列化相关的头文件，可能包含一些函数和类，用于将数据结构转换为 MsgPack 格式。
#include "carla/MsgPack.h"
// 头文件，它提供了一个固定大小的数组模板类 std::array，可以在运行时管理固定大小的数组。
#include <array>
//这两行是命名空间的定义，表明后续的代码属于 carla::streaming 命名空间。命名空间用来组织代码，避免不同代码部分之间的命名冲突。
namespace carla {
namespace streaming {

  // 一个唯一标识流的令牌.
  class Token {
  public: //public 关键字表示下面的成员是公开的，可以在类外部访问

    std::array<unsigned char, 24u> data;

    MSGPACK_DEFINE_ARRAY(data);
  };

} // namespace streaming（命名空间 streaming）
} // namespace carla（命名空间carla）
