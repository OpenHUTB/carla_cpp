// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once // 指示编译器这个头文件只包含一次，防止重复包含。

#include "carla/road/element/RoadInfo.h" // 包含RoadInfo类的头文件。
#include <string>

namespace carla {
namespace road {
namespace element {

  /// 这个记录定义了可以应用于车道的规则，以描述其他属性未覆盖的附加属性。
  ///
  class RoadInfoLaneRule final : public RoadInfo { // RoadInfoLaneRule类继承自RoadInfo类。
  public:

    RoadInfoLaneRule( // RoadInfoLaneRule类的构造函数
        double s,   // 相对于前一个车道段的位置的起始位置
                    //
        std::string value) // 车道规则的值
      : RoadInfo(s), // 调用基类RoadInfo的构造函数，传递起始位置s
        _value(std::move(value)) {} // 初始化_value成员变量

    void AcceptVisitor(RoadInfoVisitor &v) override final { // 接受访问者模式的函数
      v.Visit(*this); // 访问者访问当前对象
    }

    /// 推荐值：任何时候都不停，禁止停车，和车辆共享
    /// 
    const std::string &GetValue() const { // 获取车道规则的值
      return _value; // 返回_value成员变量的引用
    }

  private:

    const std::string _value; // 私有成员变量，存储车道规则的值
  };

} // namespace element
} // namespace road
} // namespace carla
