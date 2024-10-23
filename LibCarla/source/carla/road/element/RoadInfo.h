// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/road/element/RoadInfoVisitor.h"// 包含RoadInfoVisitor类的定义，用于访问者模式。
#include "carla/NonCopyable.h"// 包含NonCopyable类的定义，用于禁止拷贝操作。

#include <map>// 提供map容器的定义。
#include <string>// 提供string类的定义。
#include <vector>// 提供vector容器的定义。
//使用carla命名空间下的road和element子命名空间。
namespace carla {
namespace road {
namespace element {
    // 定义RoadInfo类，它继承自NonCopyable（私有继承），用于禁止拷贝操作。
  class RoadInfo : private NonCopyable {
  public:
      // 定义一个虚析构函数，确保派生类能够正确析构。
    virtual ~RoadInfo() = default;
    // 定义一个纯虚函数AcceptVisitor，接受一个RoadInfoVisitor引用作为参数。
    // 这是访问者模式的实现，允许外部通过访问者对象访问RoadInfo的内部状态。
    virtual void AcceptVisitor(RoadInfoVisitor &) = 0;
    /// 获取从道路起始位置的距离。
    /// Distance from road's start location.
    double GetDistance() const {
      return _s;// 返回私有成员变量_s的值。
    }

  protected:
      // 定义受保护的构造函数，允许派生类构造RoadInfo对象。
      // 带有默认参数0.0，表示默认距离为0。
    RoadInfo(double distance = 0.0) : _s(distance) {}

  private:
      // 私有成员变量_s，存储从道路起始位置的距离。
    double _s;
  };

} // namespace element
} // namespace road
} // namespace carla
