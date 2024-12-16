// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/client/DebugHelper.h" // 引入DebugHelper头文件

#include "carla/client/detail/Simulator.h" // 引入Simulator的细节实现
#include "carla/rpc/DebugShape.h" // 引入DebugShape的RPC接口

namespace carla {
namespace client {

  using Shape = rpc::DebugShape; // 使用rpc::DebugShape作为Shape的别名

  // 绘制形状的模板函数
  template <typename T>
  static void DrawShape(
      detail::EpisodeProxy &episode, // 细节：当前剧集代理
      const T &primitive, // 形状的基本元素
      rpc::Color color, // 颜色
      float life_time, // 生存时间
      bool persistent_lines) { // 是否为持久线
    const Shape shape{primitive, color, life_time, persistent_lines}; // 创建形状对象
    episode.Lock()->DrawDebugShape(shape); // 锁定剧集并绘制调试形状
  }

  // 绘制点
  void DebugHelper::DrawPoint(
      const geom::Location &location, // 点的位置
      float size, // 点的大小
      sensor::data::Color color, // 点的颜色
      float life_time, // 生存时间
      bool persistent_lines) { // 是否为持久线
    Shape::Point point{location, size}; // 创建点形状
    DrawShape(_episode, point, color, life_time, persistent_lines); // 调用绘制形状
  }

  // 绘制HUD点
  void DebugHelper::DrawHUDPoint(
      const geom::Location &location, // 点的位置
      float size, // 点的大小
      sensor::data::Color color, // 点的颜色
      float life_time, // 生存时间
      bool persistent_lines) { // 是否为持久线
    Shape::HUDPoint point{location, size}; // 创建HUD点形状
    DrawShape(_episode, point, color, life_time, persistent_lines); // 调用绘制形状
  }

  // 绘制线
  void DebugHelper::DrawLine(
      const geom::Location &begin, // 线的起点
      const geom::Location &end, // 线的终点
      float thickness, // 线的粗细
      sensor::data::Color color, // 线的颜色
      float life_time, // 生存时间
      bool persistent_lines) { // 是否为持久线
    Shape::Line line{begin, end, thickness}; // 创建线形状
    DrawShape(_episode, line, color, life_time, persistent_lines); // 调用绘制形状
  }

  // 绘制HUD线
  void DebugHelper::DrawHUDLine(
      const geom::Location &begin, // 线的起点
      const geom::Location &end, // 线的终点
      float thickness, // 线的粗细
      Color color, // 线的颜色
      float life_time, // 生存时间
      bool persistent_lines) { // 是否为持久线
    Shape::HUDLine line{begin, end, thickness}; // 创建HUD线形状
    DrawShape(_episode, line, color, life_time, persistent_lines); // 调用绘制形状
  }

  // 绘制箭头
  void DebugHelper::DrawArrow(
      const geom::Location &begin, // 箭头的起点
      const geom::Location &end, // 箭头的终点
      float thickness, // 箭头的粗细
      float arrow_size, // 箭头的大小
      sensor::data::Color color, // 箭头的颜色
      float life_time, // 生存时间
      bool persistent_lines) { // 是否为持久线
    Shape::Line line{begin, end, thickness}; // 创建箭头的线形状
    Shape::Arrow arrow{line, arrow_size}; // 创建箭头形状
    DrawShape(_episode, arrow, color, life_time, persistent_lines); // 调用绘制形状
  }

  // 绘制HUD箭头
  void DebugHelper::DrawHUDArrow(
      const geom::Location &begin, // 箭头的起点
      const geom::Location &end, // 箭头的终点
      float thickness, // 箭头的粗细
      float arrow_size, // 箭头的大小
      sensor::data::Color color, // 箭头的颜色
      float life_time, // 生存时间
      bool persistent_lines) { // 是否为持久线
    Shape::HUDLine line{begin, end, thickness}; // 创建HUD箭头的线形状
    Shape::HUDArrow arrow{line, arrow_size}; // 创建HUD箭头形状
    DrawShape(_episode, arrow, color, life_time, persistent_lines); // 调用绘制形状
  }

  // 绘制框
  void DebugHelper::DrawBox(
      const geom::BoundingBox &box, // 边界框
      const geom::Rotation &rotation, // 旋转信息
      float thickness, // 边框的粗细
      sensor::data::Color color, // 边框的颜色
      float life_time, // 生存时间
      bool persistent_lines) { // 是否为持久线
    Shape::Box the_box{box, rotation, thickness}; // 创建框形状
    DrawShape(_episode, the_box, color, life_time, persistent_lines); // 调用绘制形状
  }

  // 绘制HUD框
  void DebugHelper::DrawHUDBox(
      const geom::BoundingBox &box, // 边界框
      const geom::Rotation &rotation, // 旋转信息
      float thickness, // 边框的粗细
      sensor::data::Color color, // 边框的颜色
      float life_time, // 生存时间
      bool persistent_lines) { // 是否为持久线
    Shape::HUDBox the_box{box, rotation, thickness}; // 创建HUD框形状
    DrawShape(_episode, the_box, color, life_time, persistent_lines); // 调用绘制形状
  }

  // 绘制字符串
  void DebugHelper::DrawString(
      const geom::Location &location, // 字符串的位置
      const std::string &text, // 字符串内容
      bool draw_shadow, // 是否绘制阴影
      sensor::data::Color color, // 字符串的颜色
      float life_time, // 生存时间
      bool persistent_lines) { // 是否为持久线
    Shape::String string{location, text, draw_shadow}; // 创建字符串形状
    DrawShape(_episode, string, color, life_time, persistent_lines); // 调用绘制形状
  }

} // namespace client
} // namespace carla
