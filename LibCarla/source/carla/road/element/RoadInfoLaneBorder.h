// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once // 确保该头文件只被包含一次

#include "carla/road/element/RoadInfo.h" // 引入RoadInfo类的定义
#include "carla/geom/CubicPolynomial.h" // 引入CubicPolynomial类的定义

namespace carla {
namespace road {
namespace element {

  /// 与其通过宽度条目描述车道，依赖于内层车道对外层车道的影响，
  /// 不如独立描述每个车道的外边界。这种方式在道路数据源自测量时特别有用，
  /// 可以提供一种更方便的方法，而无需将道路段划分为过多部分。
  /// 注意：车道边界和宽度是互斥的。
  class RoadInfoLaneBorder final : public RoadInfo { // 定义RoadInfoLaneBorder类，继承自RoadInfo
  public:

    // 构造函数，接受五个参数来初始化RoadInfo和边界多项式
    RoadInfoLaneBorder(
        double s, // 曲线参数s
        double a, // 多项式系数a
        double b, // 多项式系数b
        double c, // 多项式系数c
        double d) // 多项式系数d
      : RoadInfo(s), // 初始化基类RoadInfo
        _border(a, b, c, d, s) {} // 初始化边界多项式

    // 接受访问者模式的实现
    void AcceptVisitor(RoadInfoVisitor &v) final {
      v.Visit(*this); // 调用访问者的Visit方法
    }

    // 获取边界多项式
    const geom::CubicPolynomial &GetPolynomial() const {
      return _border; // 返回边界多项式
    }

  private:

    const geom::CubicPolynomial _border; // 声明一个常量的CubicPolynomial对象用于存储边界
  };

} // namespace element
} // namespace road
} // namespace carla