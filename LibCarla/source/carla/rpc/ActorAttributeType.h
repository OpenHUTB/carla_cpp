// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <cstdint>

namespace carla {
namespace rpc {

  enum class ActorAttributeType : uint8_t {
 // 表示布尔类型的属性，枚举值被赋值为 1u（后缀 u 表示无符号整数常量），用于在代码中区分不同类型的 actor 属性
    Bool = 1u,
 // 表示整数类型的属性，枚举值为 2u，可能用于存储如年龄、数量等整数值的属性
    Int = 2u,
// 表示单精度浮点数类型的属性，枚举值为 3u，可用于存储像坐标、速度等带有小数部分的数值属性
    Float = 3u,
// 表示字符串类型的属性，枚举值为 4u，用于存储文本相关的属性信息
    String = 4u,
// 表示 RGB 颜色类型的属性，枚举值为 5u，可能用于表示颜色相关的属性
    RGBColor = 5u,

    SIZE,
    INVALID
  };

  // Serialization of this class is in ActorAttribute.h, to reduce dependencies
  // since this file is directly included in UE4 code.

} // namespace rpc
} // namespace carla
