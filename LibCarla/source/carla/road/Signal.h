// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy; see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Memory.h" // 引入内存管理头文件
#include "carla/NonCopyable.h" // 引入不可复制的类
#include "carla/road/RoadTypes.h" // 引入道路类型定义
#include "carla/road/LaneValidity.h" // 引入车道有效性定义
#include "carla/geom/Transform.h" // 引入几何变换定义

#include <string> // 引入字符串库
#include <vector> // 引入向量库

namespace carla {
namespace road {

  enum SignalOrientation { // 定义信号方向枚举
    Positive, // 正向
    Negative, // 负向
    Both // 双向
  };

  struct SignalDependency { // 信号依赖结构体
  public:

    SignalDependency( std::string dependency_id, std::string type) // 构造函数
      : _dependency_id(dependency_id), _type(type) {} // 初始化成员变量

    std::string _dependency_id; // 依赖ID
    std::string _type; // 依赖类型

  };

  class Signal : private MovableNonCopyable { // 定义信号类，禁止拷贝
  public:
    Signal( // 构造函数
        RoadId road_id, // 道路ID
        SignId signal_id, // 信号ID
        double s, // S坐标
        double t, // T坐标
        std::string name, // 信号名称
        std::string dynamic, // 动态属性
        std::string orientation, // 方向
        double zOffset, // Z轴偏移
        std::string country, // 国家
        std::string type, // 类型
        std::string subtype, // 子类型
        double value, // 值
        std::string unit, // 单位
        double height, // 高度
        double width, // 宽度
        std::string text, // 文本
        double hOffset, // 水平偏移
        double pitch, // 俯仰角
        double roll) // 横滚角
      : _road_id(road_id),
        _signal_id(signal_id),
        _s(s),
        _t(t),
        _name(name),
        _dynamic(dynamic),
        _orientation(orientation),
        _zOffset(zOffset),
        _country(country),
        _type(type),
        _subtype(subtype),
        _value(value),
        _unit(unit),
        _height(height),
        _width(width),
        _text(text),
        _hOffset(hOffset),
        _pitch(pitch),
        _roll(roll) {} // 初始化所有成员变量

    RoadId GetRoadId() const { // 获取道路ID
      return _road_id;
    }

    const SignId &GetSignalId() const { // 获取信号ID
      return _signal_id;
    }

    double GetS() const { // 获取S坐标
      return _s;
    }

    double GetT() const { // 获取T坐标
      return _t;
    }

    bool GetDynamic() const { // 获取动态属性
      if(_dynamic == "yes") {
        return true; // 如果动态属性为“yes”，返回true
      } else {
        return false; // 否则返回false
      }
    }

    const std::string &GetName() const { // 获取信号名称
      return _name;
    }

    SignalOrientation GetOrientation() const { // 获取信号方向
      if(_orientation == "+") {
        return SignalOrientation::Positive; // 正向
      } else if(_orientation == "-") {
        return SignalOrientation::Negative; // 负向
      } else {
        return SignalOrientation::Both; // 双向
      }
    }

    double GetZOffset() const { // 获取Z轴偏移
      return _zOffset;
    }

    const std::string &GetCountry() const { // 获取国家
      return _country;
    }

    const std::string &GetType() const { // 获取类型
      return _type;
    }

    const std::string &GetSubtype() const { // 获取子类型
      return _subtype;
    }

    double GetValue() const { // 获取值
      return _value;
    }

    const std::string &GetUnit() const { // 获取单位
      return _unit;
    }

    double GetHeight() const { // 获取高度
      return _height;
    }

    double GetWidth() const { // 获取宽度
      return _width;
    }

    const std::string &GetText() const { // 获取文本
      return _text;
    }

    double GetHOffset() const { // 获取水平偏移
      return _hOffset;
    }

    double GetPitch() const { // 获取俯仰角
      return _pitch;
    }

    double GetRoll() const { // 获取横滚角
      return _roll;
    }

    const std::vector<SignalDependency> & // 获取依赖信号
        GetDependencies() {
      return _dependencies;
    }

    const geom::Transform &GetTransform() const { // 获取几何变换
      return _transform;
    }

    const std::set<ContId>& GetControllers() const { // 获取控制器集合
      return _controllers;
    }

  private:
    friend MapBuilder; // 声明MapBuilder为友元类

    RoadId _road_id; // 道路ID
    SignId _signal_id; // 信号ID
    double _s; // S坐标
    double _t; // T坐标
    std::string _name; // 信号名称
    std::string _dynamic; // 动态属性
    std::string _orientation; // 方向
    double _zOffset; // Z轴偏移
    std::string _country; // 国家
    std::string _type; // 类型
    std::string _subtype; // 子类型
    double _value; // 值
    std::string _unit; // 单位
    double _height; // 高度
    double _width; // 宽度
    std::string _text; // 文本
    double _hOffset; // 水平偏移
    double _pitch; // 俯仰角
    double _roll; // 横滚角
    std::vector<SignalDependency> _dependencies; // 依赖信号列表
    geom::Transform _transform; // 几何变换
    std::set<ContId> _controllers; // 控制器集合
    bool _using_inertial_position = false; // 是否使用惯性位置

  };

} // road
} // carla