// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/road/element/RoadInfo.h"

namespace carla {
namespace road {
namespace element {

  class RoadInfoSpeed final : public RoadInfo { // RoadInfoSpeed类继承自RoadInfo类，用于表示道路的速度信息
  public:

    RoadInfoSpeed(double s, double speed) // 构造函数，初始化时设置道路的累计距离s和速度限制speed
      : RoadInfo(s), // 调用基类构造函数，初始化累计距离s
        _speed(speed), // 初始化速度限制speed
        _type("Town") {} // 初始化类型为"Town"

    RoadInfoSpeed(double s, double speed, std::string& type) // 构造函数，初始化时设置道路的累计距离s、速度限制speed和类型type
      : RoadInfo(s), // 调用基类构造函数，初始化累计距离s
        _speed(speed), // 初始化速度限制speed
        _type(type) {} // 初始化类型为传入的type
    void AcceptVisitor(RoadInfoVisitor &v) final { // 接受一个访问者对象v，调用v的Visit方法来访问当前对象
      v.Visit(*this);
    }

    double GetSpeed() const { // 获取当前道路信息的速度限制
      return _speed;
    }

    std::string GetType() const{  // 获取当前道路信息的类型
      return _type;
    }

  private:

    const double _speed; // 速度限制值
    const std::string _type; // 道路信息类型
  };

} // namespace element
} // namespace road
} // namespace carla
