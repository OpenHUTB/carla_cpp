// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/MsgPack.h"

#include <cstdint>

#ifdef LIBCARLA_INCLUDED_FROM_UE4
#include <compiler/enable-ue4-macros.h>
#include "Math/Color.h"
#include <compiler/disable-ue4-macros.h>
#endif // LIBCARLA_INCLUDED_FROM_UE4

namespace carla {
namespace rpc {
// 定义一个 Color 类用于表示颜色
  class Color {
  public:

    uint8_t r = 0u;// 红色通道的值，0~255之间的整数
    uint8_t g = 0u; // 绿色通道的值
    uint8_t b = 0u; // 蓝色通道的值

    Color() = default;// 默认构造函数，初始化为黑色（r=0, g=0, b=0）
    Color(const Color &) = default;// 拷贝构造函数

    Color(uint8_t r, uint8_t g, uint8_t b)
      : r(r), g(g), b(b) {} // 构造函数，接受 r, g, b 三个颜色分量值来初始化 Color 对象

    Color &operator=(const Color &) = default;

#ifdef LIBCARLA_INCLUDED_FROM_UE4

    Color(const FColor &color)
      : Color(color.R, color.G, color.B) {}

    Color(const FLinearColor &color)
      : Color(color.R * 255.0f, color.G  * 255.0f, color.B  * 255.0f) {}

    operator FColor() const {
      return FColor{r, g, b};
    }

    operator FLinearColor() const {
      return FLinearColor{
        static_cast<float>(r)/255.0f, // 将整数 RGB 分量转换为浮动值（0.0~1.0）
        static_cast<float>(g)/255.0f,
        static_cast<float>(b)/255.0f,
        1.0f // 透明度设为 1.0（不透明）
      };
    }

#endif // LIBCARLA_INCLUDED_FROM_UE4 // 使用 MsgPack 序列化库定义颜色类的序列化规则

    MSGPACK_DEFINE_ARRAY(r, g, b);// 定义序列化数组，确保 r, g, b 被正确序列化和反序列化
  };

} // namespace rpc
} // namespace carla
