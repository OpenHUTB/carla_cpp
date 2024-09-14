// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once  // 确保该头文件仅被包含一次

#include "carla/Buffer.h"  // 包含 Buffer 类的定义

#include <rpc/msgpack.hpp>  // 包含 MessagePack 的头文件

namespace carla {  // carla 命名空间

  class MsgPack {  // MsgPack 类定义
  public:

    // 模板方法，序列化对象 obj 为 Buffer 类型
    template <typename T>
    static Buffer Pack(const T &obj) {
      namespace mp = ::clmdep_msgpack;  // 为 MessagePack 命名空间起别名

      mp::sbuffer sbuf;  // 创建一个 MessagePack 的缓冲区对象
      mp::pack(sbuf, obj);  // 使用 MessagePack 序列化对象 obj 到 sbuf 中

      // 将 sbuf 中的数据和大小封装到 Buffer 对象中并返回
      return Buffer(reinterpret_cast<const unsigned char *>(sbuf.data()), sbuf.size());
    }

    // 模板方法，从 Buffer 对象反序列化为对象 T
    template <typename T>
    static T UnPack(const Buffer &buffer) {
      namespace mp = ::clmdep_msgpack;  // 为 MessagePack 命名空间起别名

      // 使用 MessagePack 从 Buffer 中的数据反序列化为对象 T
      return mp::unpack(reinterpret_cast<const char *>(buffer.data()), buffer.size()).template as<T>();
    }

    // 模板方法，从原始数据（unsigned char *）和大小反序列化为对象 T
    template <typename T>
    static T UnPack(const unsigned char *data, size_t size) {
      namespace mp = ::clmdep_msgpack;  // 为 MessagePack 命名空间起别名

      // 使用 MessagePack 从原始数据反序列化为对象 T
      return mp::unpack(reinterpret_cast<const char *>(data), size).template as<T>();
    }
  };

} // namespace carla

