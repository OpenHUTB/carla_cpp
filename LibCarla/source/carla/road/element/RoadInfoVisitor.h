// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once // 保证头文件只被包含一次

namespace carla { // 定义命名空间 carla
namespace road { // 定义命名空间 road
namespace element { // 定义命名空间 element

  class RoadInfo; // 前向声明 RoadInfo 类
  class RoadInfoElevation; // 前向声明 RoadInfoElevation 类
  class RoadInfoGeometry; // 前向声明 RoadInfoGeometry 类
  class RoadInfoLane; // 前向声明 RoadInfoLane 类
  class RoadInfoLaneAccess; // 前向声明 RoadInfoLaneAccess 类
  class RoadInfoLaneBorder; // 前向声明 RoadInfoLaneBorder 类
  class RoadInfoLaneHeight; // 前向声明 RoadInfoLaneHeight 类
  class RoadInfoLaneMaterial; // 前向声明 RoadInfoLaneMaterial 类
  class RoadInfoLaneOffset; // 前向声明 RoadInfoLaneOffset 类
  class RoadInfoLaneRule; // 前向声明 RoadInfoLaneRule 类
  class RoadInfoLaneVisibility; // 前向声明 RoadInfoLaneVisibility 类
  class RoadInfoLaneWidth; // 前向声明 RoadInfoLaneWidth 类
  class RoadInfoMarkRecord; // 前向声明 RoadInfoMarkRecord 类
  class RoadInfoMarkTypeLine; // 前向声明 RoadInfoMarkTypeLine 类
  class RoadInfoSpeed; // 前向声明 RoadInfoSpeed 类
  class RoadInfoCrosswalk; // 前向声明 RoadInfoCrosswalk 类
  class RoadInfoSignal; // 前向声明 RoadInfoSignal 类

  class RoadInfoVisitor { // 定义 RoadInfoVisitor 类
  public:

    RoadInfoVisitor() = default; // 默认构造函数
    virtual ~RoadInfoVisitor() = default; // 虚析构函数

    RoadInfoVisitor(const RoadInfoVisitor &) = default; // 拷贝构造函数
    RoadInfoVisitor(RoadInfoVisitor &&) = default; // 移动构造函数

    RoadInfoVisitor &operator=(const RoadInfoVisitor &) = default; // 拷贝赋值运算符
    RoadInfoVisitor &operator=(RoadInfoVisitor &&) = default; // 移动赋值运算符

    // 访问不同类型的道路信息的虚函数
    virtual void Visit(RoadInfoElevation &) {} // 访问 RoadInfoElevation
    virtual void Visit(RoadInfoGeometry &) {} // 访问 RoadInfoGeometry
    virtual void Visit(RoadInfoLane &) {} // 访问 RoadInfoLane
    virtual void Visit(RoadInfoLaneAccess &) {} // 访问 RoadInfoLaneAccess
    virtual void Visit(RoadInfoLaneBorder &) {} // 访问 RoadInfoLaneBorder
    virtual void Visit(RoadInfoLaneHeight &) {} // 访问 RoadInfoLaneHeight
    virtual void Visit(RoadInfoLaneMaterial &) {} // 访问 RoadInfoLaneMaterial
    virtual void Visit(RoadInfoLaneOffset &) {} // 访问 RoadInfoLaneOffset
    virtual void Visit(RoadInfoLaneRule &) {} // 访问 RoadInfoLaneRule
    virtual void Visit(RoadInfoLaneVisibility &) {} // 访问 RoadInfoLaneVisibility
    virtual void Visit(RoadInfoLaneWidth &) {} // 访问 RoadInfoLaneWidth
    virtual void Visit(RoadInfoMarkRecord &) {} // 访问 RoadInfoMarkRecord
    virtual void Visit(RoadInfoMarkTypeLine &) {} // 访问 RoadInfoMarkTypeLine
    virtual void Visit(RoadInfoSpeed &) {} // 访问 RoadInfoSpeed
    virtual void Visit(RoadInfoCrosswalk &) {} // 访问 RoadInfoCrosswalk
    virtual void Visit(RoadInfoSignal &) {} // 访问 RoadInfoSignal
  };

} // namespace element
} // namespace road
} // namespace carla