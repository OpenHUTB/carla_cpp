// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/client/detail/EpisodeProxy.h"  // 包含 EpisodeProxy 的头文件
#include "carla/geom/BoundingBox.h"              // 包含 BoundingBox 的头文件
#include "carla/geom/Location.h"                 // 包含 Location 的头文件
#include "carla/geom/Rotation.h"                 // 包含 Rotation 的头文件
#include "carla/sensor/data/Color.h"             // 包含 Color 数据类型的头文件

namespace carla {
namespace client {

  class DebugHelper {
  public:

    using Color = sensor::data::Color;  // 定义 Color 类型为 sensor::data::Color

    // 构造函数，接受一个 EpisodeProxy 对象
    explicit DebugHelper(detail::EpisodeProxy episode)
      : _episode(std::move(episode)) {}  // 初始化成员变量 _episode

    // 绘制点
    void DrawPoint(
        const geom::Location &location,   // 点的位置
        float size = 0.1f,                // 点的大小，默认值为 0.1
        Color color = Color{255u, 0u, 0u}, // 点的颜色，默认值为红色
        float life_time = -1.0f,          // 点的生命周期，默认值为无限
        bool persistent_lines = true);     // 是否绘制持久线

    // 在 HUD 上绘制点
    void DrawHUDPoint(
        const geom::Location &location,   // 点的位置
        float size = 0.1f,                // 点的大小，默认值为 0.1
        Color color = Color{255u, 0u, 0u}, // 点的颜色，默认值为红色
        float life_time = -1.0f,          // 点的生命周期，默认值为无限
        bool persistent_lines = true);     // 是否绘制持久线

    // 绘制线段
    void DrawLine(
        const geom::Location &begin,      // 线段起点的位置
        const geom::Location &end,        // 线段终点的位置
        float thickness = 0.1f,           // 线段的粗细，默认值为 0.1
        Color color = Color{255u, 0u, 0u}, // 线段的颜色，默认值为红色
        float life_time = -1.0f,          // 线段的生命周期，默认值为无限
        bool persistent_lines = true);     // 是否绘制持久线

    // 在 HUD 上绘制线段
    void DrawHUDLine(
        const geom::Location &begin,      // 线段起点的位置
        const geom::Location &end,        // 线段终点的位置
        float thickness = 1.0f,           // 线段的粗细，默认值为 1.0
        Color color = Color{225u, 0u, 0u}, // 线段的颜色，默认值为深红色
        float life_time = -1.0f,          // 线段的生命周期，默认值为无限
        bool persistent_lines = true);     // 是否绘制持久线

    // 绘制箭头
    void DrawArrow(
        const geom::Location &begin,      // 箭头的起点位置
        const geom::Location &end,        // 箭头的终点位置
        float thickness = 0.1f,           // 箭头的粗细，默认值为 0.1
        float arrow_size = 0.1f,          // 箭头的大小，默认值为 0.1
        Color color = Color{255u, 0u, 0u}, // 箭头的颜色，默认值为红色
        float life_time = -1.0f,          // 箭头的生命周期，默认值为无限
        bool persistent_lines = true);     // 是否绘制持久线

    // 在 HUD 上绘制箭头
    void DrawHUDArrow(
        const geom::Location &begin,      // 箭头的起点位置
        const geom::Location &end,        // 箭头的终点位置
        float thickness = 0.1f,           // 箭头的粗细，默认值为 0.1
        float arrow_size = 0.1f,          // 箭头的大小，默认值为 0.1
        Color color = Color{255u, 0u, 0u}, // 箭头的颜色，默认值为红色
        float life_time = -1.0f,          // 箭头的生命周期，默认值为无限
        bool persistent_lines = true);     // 是否绘制持久线

    // 绘制矩形框
    void DrawBox(
        const geom::BoundingBox &box,     // 矩形框的边界
        const geom::Rotation &rotation,    // 矩形框的旋转
        float thickness = 0.1f,           // 矩形框的粗细，默认值为 0.1
        Color color = Color{255u, 0u, 0u}, // 矩形框的颜色，默认值为红色
        float life_time = -1.0f,          // 矩形框的生命周期，默认值为无限
        bool persistent_lines = true);     // 是否绘制持久线

    // 在 HUD 上绘制矩形框
    void DrawHUDBox(
        const geom::BoundingBox &box,     // 矩形框的边界
        const geom::Rotation &rotation,    // 矩形框的旋转
        float thickness = 0.1f,           // 矩形框的粗细，默认值为 0.1
        Color color = Color{255u, 0u, 0u}, // 矩形框的颜色，默认值为红色
        float life_time = -1.0f,          // 矩形框的生命周期，默认值为无限
        bool persistent_lines = true);     // 是否绘制持久线

    // 绘制字符串
    void DrawString(
        const geom::Location &location,   // 字符串的位置
        const std::string &text,           // 要绘制的字符串
        bool draw_shadow = false,          // 是否绘制阴影
        Color color = Color{255u, 0u, 0u}, // 字符串的颜色，默认值为红色
        float life_time = -1.0f,          // 字符串的生命周期，默认值为无限
        bool persistent_lines = true);     // 是否绘制持久线

  private:

    detail::EpisodeProxy _episode;  // 存储 EpisodeProxy 对象
  };

} // namespace client
} // namespace carla

