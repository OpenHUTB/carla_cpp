// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy; see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/NonCopyable.h"   // 引入不可拷贝类的定义
#include "carla/road/RoadTypes.h"  // 引入道路类型的定义


#include <set>    // 引入集合容器


namespace carla {   // carla 命名空间
namespace road {  // road 命名空间

  class MapBuilder;   // 前向声明 MapBuilder 类

  class Controller : private MovableNonCopyable {  // 定义 Controller 类，继承自不可拷贝类

  public:

    Controller(  // 构造函数
        ContId id,  // 控制器 ID
        std::string name,  // 控制器名称
        uint32_t sequence)  // 控制器序列号
      : _id(id),   // 初始化 _id
        _name(name),  // 初始化 _name
        _sequence(sequence){}  // 初始化 _sequence

    const ContId& GetControllerId() const{  // 获取控制器 ID 的方法
      return _id;  // 返回控制器 ID
    }

    const std::string& GetName() const {  // 获取控制器名称的方法
      return _name;  // 返回控制器名称
    }

    const uint32_t &GetSequence() const {  // 获取控制器序列号的方法
      return _sequence;   // 返回控制器序列号
    }

    const std::set<SignId>&  GetSignals() const {  // 获取信号集合的方法
      return _signals;  // 返回信号集合
    }

    const std::set<JuncId>&  GetJunctions() const {  // 获取交叉口集合的方法
      return _junctions;   // 返回交叉口集合
    }

  private:

    friend MapBuilder;  // 声明 MapBuilder 为友元类

    ContId _id;   // 控制器 ID
    std::string _name;  // 控制器名称
    uint32_t _sequence;   // 控制器序列号

    std::set<JuncId> _junctions;   // 存储交叉口的集合
    std::set<SignId> _signals;  // 存储信号的集合
  };

} // namespace road
} // namespace carla
