// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

// 防止头文件被重复包含
#pragma once

// 引入Carla的MsgPack相关头文件，可能用于消息的序列化、反序列化等操作
#include "carla/MsgPack.h"
// 引入Carla中与流相关的Token头文件，Token可能在流通信等场景中有特定作用
#include "carla/streaming/Token.h"

// 引入标准C++中表示无符号整数类型的头文件，这里后续用于定义特定的类型
#include <cstdint>
// 引入标准C++中表示字符串的头文件
#include <string>

// 定义在Carla命名空间下的rpc命名空间，用于组织相关的RPC（远程过程调用）相关的类和结构等
namespace carla {
namespace rpc {

  /// @todo Rename, does not represent the episode info anymore.
  /// 定义一个名为EpisodeInfo的类，从注释来看，它目前的命名不太准确，已经不再能很好地表示剧集（episode）相关信息了，后续可能需要重命名
  class EpisodeInfo {
  public:
    // 定义一个类型别名id_type，它实际上是一个64位的无符号整数类型，用于表示某种唯一标识（可能是剧集的ID之类的，具体取决于使用场景）
    using id_type = uint64_t;

    // 声明一个id成员变量，类型为上面定义的id_type，用于存储相关的唯一标识
    id_type id;
    // 声明一个token成员变量，类型为streaming::Token，可能用于流相关的认证、标识或者操作等，具体取决于其实现和使用方式
    streaming::Token token;

    // 使用MsgPack的宏来定义如何将这个类的对象进行序列化和反序列化，这里指定将id和token成员变量包含在序列化和反序列化的操作中
    MSGPACK_DEFINE_ARRAY(id, token);
  };

} // namespace rpc
} // namespace carla
