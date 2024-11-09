// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/road/element/RoadInfo.h" // 包含RoadInfo基类的定义
#include "carla/geom/CubicPolynomial.h" // 包含用于描述多项式曲线的CubicPolynomial类的定义

namespace carla {
namespace road {
namespace element {

  /// RoadInfoLaneHeight类是RoadInfo的派生类，用于表示车道的表面高度信息。
  /// 它提供了一种简化的方法来描述车道表面相对于道路平面的偏移量，
  /// 通过在车道轮廓的离散位置设置内部和外部偏移量。
  ///
  class RoadInfoLaneHeight final : public RoadInfo {
  public:

    RoadInfoLaneHeight( // 构造函数，初始化RoadInfoLaneHeight对象
        double s,  // s参数表示当前车道段相对于前一个车道段的位置
        double inner, // inner参数表示车道内侧相对于道路平面的偏移量
        double outer) // outer参数表示车道外侧相对于道路平面的偏移量
      : RoadInfo(s), // 调用基类RoadInfo的构造函数，传递s参数
        _inner(inner), // 初始化内部偏移量成员变量
        _outer(outer) {} // 初始化外部偏移量成员变量

    void AcceptVisitor(RoadInfoVisitor &v) final { // 实现基类中声明的虚函数AcceptVisitor，以支持访问者模式
      v.Visit(*this); // 调用访问者对象的Visit方法，传入当前对象的引用
    }

    /// 获取内部偏移量的方法
    double GetInner() const {
      return _inner; // 返回内部偏移量成员变量的值
    }

    /// 获取外部偏移量的方法
    double GetOuter() const {
      return _outer; // 返回外部偏移量成员变量的值
    }

  private:

    const double _inner;  // 内部偏移量成员变量，表示车道内侧相对于道路平面的偏移量

    const double _outer;  // 外部偏移量成员变量，表示车道外侧相对于道路平面的偏移量
  };

} // namespace element
} // namespace road
} // namespace carla
