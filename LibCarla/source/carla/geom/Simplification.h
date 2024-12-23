// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/geom/Mesh.h" // 包含Mesh类的定义

namespace carla {
namespace geom {

  class Simplification {
  public:

    Simplification() = default; // 默认构造函数

    Simplification(float simplificationrate) /// 带参数的构造函数，用于初始化简化率
      : simplification_percentage(simplificationrate) // 初始化简化率
      {}

    float simplification_percentage; // 存储简化率

    void Simplificate(const std::unique_ptr<geom::Mesh>& pmesh); // 声明简化函数
  };

} // namespace geom
} // namespace carla
