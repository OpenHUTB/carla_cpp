// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/road/element/RoadInfo.h" // 引入carla模块中road元素的RoadInfo类
#include <string>

namespace carla {
namespace road {
namespace element {

  class RoadInfoMarkTypeLine final : public RoadInfo { // 定义一个名为RoadInfoMarkTypeLine的类，继承自RoadInfo类
  public:

    RoadInfoMarkTypeLine( // 类的构造函数，用于初始化道路标记线信息
        double s, // 道路标记线的起始位置
        int road_mark_id, // 道路标记的唯一标识符
        double length, // 道路标记线的长度
        double space, // 道路标记线之间的间隔
        double tOffset, // 道路标记线的横向偏移量
        std::string rule, // 道路标记线的规则
        double width) // 道路标记线的宽度
      : RoadInfo(s), // 调用基类RoadInfo的构造函数，传递s参数
        _road_mark_id(road_mark_id), // 初始化道路标记ID
        _length(length), // 初始化道路标记线的长度
        _space(space), // 初始化道路标记线之间的间隔
        _tOffset(tOffset), // 初始化道路标记线的横向偏移量
        _rule(rule), // 初始化道路标记线的规则
        _width(width) {} // 初始化道路标记线的宽度

    void AcceptVisitor(RoadInfoVisitor &v) final { // 实现AcceptVisitor虚函数，接受一个RoadInfoVisitor类型的引用
      v.Visit(*this); // 调用访问者的Visit方法，传递当前对象
    }

    int GetRoadMarkId() const { // 获取道路标记ID的方法
      return _road_mark_id;
    }

    double GetLength() const { // 获取道路标记线长度的方法
      return _length;
    }

    double GetSpace() const {  // 获取道路标记线间隔的方法
      return _space;
    }

    double GetTOffset() const { // 获取道路标记线横向偏移量的方法
      return _tOffset;
    }

    const std::string &GetRule() const { // 获取道路标记线规则的方法
      return _rule;
    }

    double GetWidth() const { // 获取道路标记线宽度的方法
      return _width;
    }

  private:

    const int _road_mark_id; // 私有成员变量，存储道路标记ID

    const double _length; // 私有成员变量，存储道路标记线的长度

    const double _space; // 私有成员变量，存储道路标记线之间的间隔

    const double _tOffset; // 私有成员变量，存储道路标记线的横向偏移量

    const std::string _rule; // 私有成员变量，存储道路标记线的规则

    const double _width; // 私有成员变量，存储道路标记线的宽度
  };

} // namespace element
} // namespace road
} // namespace carla
