// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once  // 防止头文件重复包含

#include "carla/road/Signal.h"  // 包含 Signal 的定义
#include "carla/road/element/RoadInfo.h"  // 包含 RoadInfo 的定义

namespace carla {  // 定义 carla 命名空间
namespace road {  // 定义 road 子命名空间
namespace element {  // 定义 element 子命名空间

  class RoadInfoSignal final : public RoadInfo {  // 定义 RoadInfoSignal 类，继承自 RoadInfo
  public:

    // 构造函数，接受信号 ID、信号对象、道路 ID、坐标 s、t 和方向
    RoadInfoSignal(
        SignId signal_id,  // 信号 ID
        Signal* signal,  // 信号对象指针
        RoadId road_id,  // 道路 ID
        double s,  // 坐标 s
        double t,  // 坐标 t
        std::string orientation)  // 方向字符串
      : RoadInfo(s),  // 初始化基类 RoadInfo
        _signal_id(signal_id),  // 初始化信号 ID
        _signal(signal),  // 初始化信号对象
        _road_id(road_id),  // 初始化道路 ID
        _s(s),  // 初始化坐标 s
        _t(t),  // 初始化坐标 t
        _orientation(orientation) {}  // 初始化方向

    // 另一个构造函数，接受信号 ID、道路 ID、坐标 s、t 和方向
    RoadInfoSignal(
        SignId signal_id,  // 信号 ID
        RoadId road_id,  // 道路 ID
        double s,  // 坐标 s
        double t,  // 坐标 t
        std::string orientation)  // 方向字符串
      : RoadInfo(s),  // 初始化基类 RoadInfo
        _signal_id(signal_id),  // 初始化信号 ID
        _road_id(road_id),  // 初始化道路 ID
        _s(s),  // 初始化坐标 s
        _t(t),  // 初始化坐标 t
        _orientation(orientation) {}  // 初始化方向

    // 接受访问者的函数
    void AcceptVisitor(RoadInfoVisitor &v) final {  // 重写接受访问者的方法
      v.Visit(*this);  // 调用访问者的 Visit 方法
    }

    // 获取信号 ID
    SignId GetSignalId() const {  // 返回信号 ID
      return _signal_id;  // 返回信号 ID
    }

    // 获取信号对象
    const Signal* GetSignal() const {  // 返回信号对象的常量指针
      return _signal;  // 返回信号对象
    }

    // 获取道路 ID
    RoadId GetRoadId() const {  // 返回道路 ID
      return _road_id;  // 返回道路 ID
    }

    // 判断信号是否是动态的
    bool IsDynamic() const {  // 返回信号是否动态
      return _signal->GetDynamic();  // 调用信号对象的方法获取动态状态
    }

    // 获取坐标 s
    double GetS() const {  // 返回坐标 s
      return _s;  // 返回坐标 s
    }

    // 获取坐标 t
    double GetT() const {  // 返回坐标 t
      return _t;  // 返回坐标 t
    }

    // 获取信号的方向
    SignalOrientation GetOrientation() const {  // 返回信号方向
      if(_orientation == "+") {  // 如果方向为 "+"
        return SignalOrientation::Positive;  // 返回正向
      } else if(_orientation == "-") {  // 如果方向为 "-"
        return SignalOrientation::Negative;  // 返回反向
      } else {
        return SignalOrientation::Both;  // 否则返回双向
      }
    }

    // 获取有效车道信息
    const std::vector<LaneValidity> &GetValidities() const {  // 返回有效性向量的常量引用
      return _validities;  // 返回有效性向量
    }

  private:
    friend MapBuilder;  // 声明 MapBuilder 为友元类，允许其访问私有成员

    SignId _signal_id;  // 信号 ID
    Signal* _signal;  // 信号对象指针
    RoadId _road_id;  // 道路 ID
    double _s;  // 坐标 s
    double _t;  // 坐标 t
    std::string _orientation;  // 方向字符串
    std::vector<LaneValidity> _validities;  // 有效性向量
  };

} // namespace element
} // namespace road
} // namespace carla
