// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
// 
// 画质
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/MsgPack.h"

#include <cstdint>

namespace carla {
namespace rpc {

  enum class QualityLevel : uint8_t {
    Low,  // 低画质，通过命令启动：CarlaUE4.exe -quality-level=Low
    // Medium,
    // High,
    Epic,  // 高画质（史诗级）

    SIZE,
    INVALID
  };

} // namespace rpc
} // namespace carla

MSGPACK_ADD_ENUM(carla::rpc::QualityLevel);
