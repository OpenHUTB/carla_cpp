// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/MsgPack.h"
#include "carla/MsgPackAdaptors.h"
#include "carla/geom/BoundingBox.h"
#include "carla/geom/Location.h"
#include "carla/geom/Rotation.h"
#include "carla/rpc/Color.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4583)
#pragma warning(disable:4582)
#include <boost/variant2/variant.hpp>
#pragma warning(pop)
#else
#include <boost/variant2/variant.hpp>
#endif

namespace carla {
namespace rpc {

  // 定义 DebugShape 类，用于表示调试绘制的几何形状
  class DebugShape {
  public:

    // 点结构体，包含位置和大小
    struct Point {
      geom::Location location;  // 点的位置（geom::Location 类型）
      float size;               // 点的大小
      MSGPACK_DEFINE_ARRAY(location, size);  // 使用 MsgPack 序列化，定义该结构的字段
    };

    // HUD 点结构体，包含位置和大小
    struct HUDPoint {
      geom::Location location;  // 点的位置（geom::Location 类型）
      float size;               // 点的大小
      MSGPACK_DEFINE_ARRAY(location, size);  // 使用 MsgPack 序列化，定义该结构的字段
    };

    // 线段结构体，包含起始点、结束点和线的粗细
    struct Line {
      geom::Location begin;     // 线段的起始点
      geom::Location end;       // 线段的结束点
      float thickness;          // 线段的粗细
      MSGPACK_DEFINE_ARRAY(begin, end, thickness);  // 使用 MsgPack 序列化，定义该结构的字段
    };

    // HUD 线段结构体，包含起始点、结束点和线的粗细
    struct HUDLine {
      geom::Location begin;     // 线段的起始点
      geom::Location end;       // 线段的结束点
      float thickness;          // 线段的粗细
      MSGPACK_DEFINE_ARRAY(begin, end, thickness);  // 使用 MsgPack 序列化，定义该结构的字段
    };

    // 箭头结构体，包含线段和箭头大小
    struct Arrow {
      Line line;                // 箭头的线段
      float arrow_size;         // 箭头的大小
      MSGPACK_DEFINE_ARRAY(line, arrow_size);  // 使用 MsgPack 序列化，定义该结构的字段
    };

    // HUD 箭头结构体，包含线段和箭头大小
    struct HUDArrow {
      HUDLine line;             // HUD 箭头的线段
      float arrow_size;         // 箭头的大小
      MSGPACK_DEFINE_ARRAY(line, arrow_size);  // 使用 MsgPack 序列化，定义该结构的字段
    };

    // 方框结构体，包含边界框、旋转角度和线的粗细
    struct Box {
      geom::BoundingBox box;    // 方框的边界框
      geom::Rotation rotation;  // 方框的旋转角度
      float thickness;          // 方框的线粗细
      MSGPACK_DEFINE_ARRAY(box, rotation, thickness);  // 使用 MsgPack 序列化，定义该结构的字段
    };

    // HUD 方框结构体，包含边界框、旋转角度和线的粗细
    struct HUDBox {
      geom::BoundingBox box;    // 方框的边界框
      geom::Rotation rotation;  // 方框的旋转角度
      float thickness;          // 方框的线粗细
      MSGPACK_DEFINE_ARRAY(box, rotation, thickness);  // 使用 MsgPack 序列化，定义该结构的字段
    };

    // 字符串结构体，包含位置、文本和是否绘制阴影
    struct String {
      geom::Location location;  // 文本的绘制位置
      std::string text;         // 要绘制的文本
      bool draw_shadow;         // 是否绘制文本阴影
      MSGPACK_DEFINE_ARRAY(location, text, draw_shadow);  // 使用 MsgPack 序列化，定义该结构的字段
    };

    // 使用 boost::variant 存储多种几何形状的变种类型
    boost::variant2::variant<Point, Line, Arrow, Box, String, HUDPoint, HUDLine, HUDArrow, HUDBox> primitive;

    // 颜色，默认为红色（255u, 0u, 0u）
    Color color = {255u, 0u, 0u};

    // 生命周期，默认为 -1.0f 表示永远存在
    float life_time = -1.0f;

    // 是否为持久化线条，默认为 true
    bool persistent_lines = true;

    // 使用 MsgPack 序列化，定义该结构的字段
    MSGPACK_DEFINE_ARRAY(primitive, color, life_time, persistent_lines);
  };

} // namespace rpc
} // namespace carla