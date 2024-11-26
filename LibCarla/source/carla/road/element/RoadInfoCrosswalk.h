// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once // 防止头文件被多重包含

#include "carla/road/element/RoadInfo.h" // 引入 RoadInfo 头文件

namespace carla { // 开始 carla 命名空间
namespace road { // 开始 road 命名空间
namespace element { // 开始 element 命名空间

  // 定义一个结构体 CrosswalkPoint，表示人行横道的点
  struct CrosswalkPoint {
    double u            { 0.0 }; // 横向坐标 u，初始化为 0.0
    double v            { 0.0 }; // 纵向坐标 v，初始化为 0.0
    double z            { 0.0 }; // 高度坐标 z，初始化为 0.0
    // 构造函数，初始化 u、v 和 z 的值
    CrosswalkPoint(double _u, double _v, double _z) : u(_u), v(_v), z(_z) {};
  };

  // 定义一个类 RoadInfoCrosswalk，继承自 RoadInfo
  class RoadInfoCrosswalk final : public RoadInfo {
  public:

    // 构造函数，初始化 RoadInfoCrosswalk 的成员变量
    RoadInfoCrosswalk(
        const double s, // 路段的 s 坐标
        const std::string name, // 人行横道的名称
        const double t, // 路段的 t 坐标
        const double zOffset, // z 方向的偏移量
        const double hdg, // 航向角
        const double pitch, // 俯仰角
        const double roll, // 横滚角
        const std::string orientation, // 定位方向
        const double width, // 人行横道的宽度
        const double length, // 人行横道的长度
        const std::vector<CrosswalkPoint> points) // 人行横道的点集合
      : RoadInfo(s), // 初始化基类 RoadInfo 的 s 坐标
        _name(name), // 初始化名称
        _t(t), // 初始化 t 坐标
        _zOffset(zOffset), // 初始化 z 偏移量
        _hdg(hdg), // 初始化航向角
        _pitch(pitch), // 初始化俯仰角
        _roll(roll), // 初始化横滚角
        _orientation(orientation), // 初始化定位方向
        _width(width), // 初始化宽度
        _length(length), // 初始化长度
        _points(points) {} // 初始化人行横道的点集合

    // 接受访问者模式
    void AcceptVisitor(RoadInfoVisitor &v) final {
      v.Visit(*this); // 让访问者访问当前对象
    }

    // 获取 s 坐标
    double GetS() const { return GetDistance(); };
    // 获取 t 坐标
    double GetT() const { return _t; };
    // 获取人行横道的宽度
    double GetWidth() const { return _width; };
    // 获取人行横道的长度
    double GetLength() const { return _length; };
    // 获取航向角
    double GetHeading() const { return _hdg; };
    // 获取俯仰角
    double GetPitch() const { return _pitch; };
    // 获取横滚角
    double GetRoll() const { return _roll; };
    // 获取 z 偏移量
    double GetZOffset() const { return _zOffset; };
    // 获取定位方向
    std::string GetOrientation() const { return _orientation; };
    // 获取人行横道的点集合
    const std::vector<CrosswalkPoint> &GetPoints() const { return _points; };

  private:
    std::string _name; // 存储人行横道的名称
    double _t; // 存储 t 坐标
    double _zOffset; // 存储 z 偏移量
    double _hdg; // 存储航向角
    double _pitch; // 存储俯仰角
    double _roll; // 存储横滚角
    std::string _orientation; // 存储定位方向
    double _width; // 存储人行横道的宽度
    double _length; // 存储人行横道的长度
    std::vector<CrosswalkPoint> _points; // 存储人行横道的点集合
  };

} // namespace element
} // namespace road
} // namespace carla