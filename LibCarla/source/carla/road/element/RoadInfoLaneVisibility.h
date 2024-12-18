// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once // 防止头文件被重复包含

#include "carla/road/element/RoadInfo.h"

namespace carla {
namespace road {
namespace element {

  ///  RoadInfoLaneVisibility类表示道路横截面中每个车道的可见性信息
  /// 此类定义了车道相对于其方向的四个方向（前、后、左、右）的可见性
  /// 每个条目直到定义了新的条目之前都是有效的
  /// 如果定义了多个条目，它们必须按递增顺序列出
  ///
  /// 对于左侧车道（正ID），前方方向与轨道方向相反
  /// 对于右侧车道，前方方向与轨道方向相同
  class RoadInfoLaneVisibility final : public RoadInfo {
  public:

    RoadInfoLaneVisibility( // 构造函数，初始化道路可见性信息
        double s,  // 相对于前一个车道段的位置s
        double forward, // 车道前方的可见性距离
        double back, // 车道后方的可见性距离
        double left, // 车道左侧的可见性距离
        double right) // 车道右侧的可见性距离
      : RoadInfo(s), // 调用基类RoadInfo的构造函数
        _forward(forward), // 初始化前方可见性
        _back(back), // 初始化后方可见性
        _left(left), // 初始化左侧可见性
        _right(right) {} // 初始化右侧可见性

    void AcceptVisitor(RoadInfoVisitor &v) final { // 接受访问者模式中的访问者访问
      v.Visit(*this); // 访问者访问当前对象
    }

    double GetForward() const { // 获取前方可见性距离
      return _forward;
    }

    double GetBack() const { // 获取后方可见性距离
      return _back;
    }

    double GetLeft() const { // 获取左侧可见性距离
      return _left;
    }

    double GetRight() const { // 获取右侧可见性距离
      return _right;
    }

  private:

    const double _forward; // 前方可见性距离

    const double _back; // 后方可见性距离

    const double _left; // 左侧可见性距离

    const double _right; // 右侧可见性距离
  };

} // namespace element
} // namespace road
} // namespace carla
