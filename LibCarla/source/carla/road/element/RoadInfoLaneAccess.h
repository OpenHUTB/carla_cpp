// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once // 确保该头文件只被包含一次

#include "carla/road/element/RoadInfo.h" // 引入RoadInfo类的头文件
#include <string> // 引入字符串类的头文件

namespace carla { // 定义carla命名空间
namespace road { // 定义road命名空间
namespace element { // 定义element命名空间

  /// 此记录定义了某些类型道路使用者的访问限制。
  /// 该记录可用于补充来自标志或信号的限制，以控制场景中的交通流。
  /// 每个条目在s坐标增加的方向上有效，直到定义新的条目。
  /// 如果定义了多个条目，它们必须按增加顺序列出。
  class RoadInfoLaneAccess final : public RoadInfo { // 定义RoadInfoLaneAccess类，继承自RoadInfo
  public:

    // 构造函数，接受起始位置和限制字符串
    RoadInfoLaneAccess(
        double s,   // 相对于前一车道段的位置的起始位置
        std::string restriction) // 限制的描述
      : RoadInfo(s), // 调用基类构造函数
        _restriction(restriction) {} // 初始化限制字符串

    // 接受访问者模式的访问器
    void AcceptVisitor(RoadInfoVisitor &v) final { 
      v.Visit(*this); // 调用访问者的Visit方法
    }

    // 获取限制字符串
    const std::string &GetRestriction() const { 
      return _restriction; // 返回限制字符串的引用
    }

  private:

    const std::string _restriction; // 限制字符串，例如：Simulator、Autonomous Traffic、Pedestrian 和 None
  };

} // namespace element
} // namespace road
} // namespace carla