// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
#include "carla/NonCopyable.h" // 引入不可拷贝类的头文件
#include "carla/road/RoadTypes.h" // 引入道路类型的头文件

#include <string> // 引入字符串库

namespace carla { // 定义 carla 命名空间
namespace road { // 定义 road 命名空间

  class Object : private MovableNonCopyable { // 定义 Object 类，继承自 MovableNonCopyable（不可拷贝但可移动）
  public:

    Object() = default; // 默认构造函数

  private:

    ObjId _id = 0u; // 对象 ID，初始化为 0
    std::string _type; // 对象类型
    std::string _name; // 对象名称
    double _s = 0.0; // 对象在路径上的位置（s坐标），初始化为 0.0
    double _t = 0.0; // 对象的横向位置（t坐标），初始化为 0.0
    double _zOffset = 0.0; // 对象的 Z 轴偏移量，初始化为 0.0
    double _validLength = 0.0; // 对象的有效长度，初始化为 0.0
    std::string _orientation; // 对象的朝向
    double _lenght = 0.0; // 对象的长度，初始化为 0.0（注意：变量名拼写错误，应为 '_length'）
    double _width = 0.0; // 对象的宽度，初始化为 0.0
    double _hdg = 0.0; // 对象的航向角，初始化为 0.0
    double _pitch = 0.0; // 对象的俯仰角，初始化为 0.0
    double _roll = 0.0; // 对象的滚转角，初始化为 0.0
  };

} // road
} // carla