// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/MsgPack.h"
#include "carla/streaming/Token.h"

#include <cstdint>
#include <string>

namespace carla {
namespace rpc {

  /// @class EpisodeInfo
  /// @brief 该类可能用于存储与某个 episode 相关的信息，但根据 @todo 注释，该类名可能不再合适，因为它不再准确代表 episode 信息。
  class EpisodeInfo {
  public:
    // 定义一个类型别名 id_type 为 uint64_t，用于表示 id 的数据类型
    using id_type = uint64_t;

    // 存储 episode 的唯一标识符
    id_type id;

    // 存储 streaming 的令牌
    streaming::Token token;

    // 使用 MSGPACK_DEFINE_ARRAY 宏定义如何将该类的成员变量打包到 msgpack 中，
    // 这里表示将 id 和 token 按顺序打包
    MSGPACK_DEFINE_ARRAY(id, token);
  };

} // namespace rpc
} // namespace carla
