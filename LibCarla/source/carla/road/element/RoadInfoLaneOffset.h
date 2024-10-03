// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/road/element/RoadInfo.h" // 包含定义道路信息元素的头文件
#include "carla/geom/CubicPolynomial.h" // 包含定义三次多项式的头文件

namespace carla {
namespace road {
namespace element {

  /// 车道偏移记录定义了车道参考线（通常与道路参考线相同）的横向位移。
  /// 这可以用于轻松实现车道相对于道路参考线的（局部）横向位移。
  /// 特别是，这个特性可以显著地促进城市内部布局或“2+1”乡村道路布局的建模。
  ///
  class RoadInfoLaneOffset final : public RoadInfo { // final关键字表示此类不允许被继承
  public:

    RoadInfoLaneOffset( // 构造函数，初始化道路信息和偏移量多项式
        double s, // 道路纵向位置s
        double a, // 三次多项式系数a
        double b, // 三次多项式系数b
        double c, // 三次多项式系数c
        double d) // 三次多项式系数d
      : RoadInfo(s), // 调用基类RoadInfo的构造函数，传递参数s
        _offset(a, b, c, d, s) {} // 初始化成员变量_offset

    void AcceptVisitor(RoadInfoVisitor &v) final { // 访问者模式的AcceptVisitor函数，允许访问者对象访问当前对象
      v.Visit(*this); // 访问者访问当前对象
    }

    const geom::CubicPolynomial &GetPolynomial() const { // 获取车道偏移量的三次多项式
      return _offset; // 返回_offset成员变量
    }

  private:

    const geom::CubicPolynomial _offset; // 私有成员变量，存储车道偏移量的三次多项式
  };

} // namespace element
} // namespace road
} // namespace carla
