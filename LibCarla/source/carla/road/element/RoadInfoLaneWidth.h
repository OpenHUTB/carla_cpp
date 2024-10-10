// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/road/element/RoadInfo.h" // 包含RoadInfo类的定义
#include "carla/geom/CubicPolynomial.h" // 包含CubicPolynomial类的定义

namespace carla {
namespace road { // 定义road命名空间
namespace element { // 定义element命名空间

  ///车道宽度记录：道路上每个交叉部分的车道可以提供多个宽度条目。
  ///每个车道至少必须定义一个条目，除了按照惯例中心车道宽度为零。
  ///每个条目在定义新条目之前都是有效的。如果为一个车道定义了多个条目，
  /// 它们必须按升序排列。
  class RoadInfoLaneWidth final : public RoadInfo { // 构造函数，初始化RoadInfo和CubicPolynomial对象
  public:

    RoadInfoLaneWidth(
        double s, // s表示沿道路的长度参数
        double a, // a是多项式的系数
        double b, // b是多项式的系数
        double c, // c是多项式的系数
        double d) // d是多项式的系数
      : RoadInfo(s),  // 初始化基类
        _width(a, b, c, d, s) {} // 初始化成员变量_width

    void AcceptVisitor(RoadInfoVisitor &v) final { // 接受访问者模式的访问
      v.Visit(*this); // 访问当前对象
    }

    const geom::CubicPolynomial &GetPolynomial() const { // 获取多项式对象
      return _width; // 返回_width对象
    }

  private:
     // 定义一个CubicPolynomial对象，表示车道宽度
    const geom::CubicPolynomial _width;
  };

} // namespace element
} // namespace road
} // namespace carla
