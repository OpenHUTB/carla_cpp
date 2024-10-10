// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/road/element/RoadInfo.h"
#include <string>

namespace carla {
namespace road {
namespace element {

  class RoadInfoLaneMaterial final : public RoadInfo { // 定义RoadInfoLaneMaterial类，继承自RoadInfo
  public:

    RoadInfoLaneMaterial( // 构造函数，用于初始化RoadInfoLaneMaterial对象
        double s,   // start position relative to the position of the preceding
                    // lane section
        std::string surface, // surface参数表示路面的材质类型
        double friction, // friction参数表示路面的摩擦系数
        double roughness) // roughness参数表示路面的粗糙度
      : RoadInfo(s), // 调用基类RoadInfo的构造函数，传递s参数
        _surface(std::move(surface)), // 初始化路面材质
        _friction(friction), // 初始化摩擦系数
        _roughness(roughness) {} // 初始化粗糙度

    void AcceptVisitor(RoadInfoVisitor &v) override final { // AcceptVisitor函数用于接受一个访问者对象v，并将当前对象的控制权交给访问者
      v.Visit(*this); // 调用访问者的Visit函数，传递当前对象的引用
    }

    const std::string &GetSurface() const { // 获取路面材质
      return _surface;
    }

    double GetFriction() const { // 获取摩擦系数
      return _friction;
    }

    double GetRoughness() const { // 获取粗糙度
      return _roughness;
    }

  private:

    const std::string _surface; // _surface成员变量用于存储车道的路面材质

    const double _friction; // _friction成员变量用于存储车道的摩擦系数

    const double _roughness; // _roughness成员变量用于存储车道的粗糙度
  };

} // namespace element
} // namespace road
} // namespace carla
