// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once // 指示这个头文件被包含一次，防止重复包含

#include "carla/road/element/RoadInfo.h"
#include "carla/geom/CubicPolynomial.h"

namespace carla {
namespace road {
namespace element {

  class RoadInfoElevation final : public RoadInfo { // RoadInfoElevation类继承自RoadInfo类
  public:

    RoadInfoElevation( // RoadInfoElevation的构造函数
        double s, // 道路的s偏移量
        double a, // 高程
        double b, // 坡度
        double c, // 垂直曲率
        double d) // 曲率变化
      : RoadInfo(s), // 调用基类构造函数
        _elevation(a, b, c, d, s) {} // 初始化_elevation成员变量

    void AcceptVisitor(RoadInfoVisitor &v) final { // 接受访问者模式的函数，用于访问RoadInfoElevation对象
      v.Visit(*this); // 调用访问者对象的Visit函数
    }

    // @todo 未使用？你可以直接使用多项式。
    double Evaluate(const double dist, double &out_tan) const { // Evaluate函数用于计算给定距离处的elevation值和其切线值
      out_tan = _elevation.Tangent(dist); // 计算并设置切线值
      return _elevation.Evaluate(dist); // 返回elevation值
    }

    const geom::CubicPolynomial &GetPolynomial() const { // GetPolynomial函数返回_elevation的CubicPolynomial对象
      return _elevation;
    }

  private:

    const geom::CubicPolynomial _elevation; // _elevation是CubicPolynomial类型的成员变量，用于表示道路的高程
  };

} // namespace element
} // namespace road
} // namespace carla
