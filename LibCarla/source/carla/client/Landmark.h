// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Memory.h"
#include "carla/client/Waypoint.h"
#include "carla/geom/Transform.h"
#include "carla/geom/BoundingBox.h"

#include "carla/road/element/RoadInfoSignal.h"

#include <string>

namespace carla {
namespace client {

  /// 包含对 RoadInfoSignal 的引用类
  class Landmark {
  public:
// 获取地标对应的路径点
    SharedPtr<Waypoint> GetWaypoint() const {
      return _waypoint;
    }
// 获取地标的变换信息（从引用的信号中获取）
    const geom::Transform &GetTransform() const {
      return _signal->GetSignal()->GetTransform();
    }
// 获取地标所在的道路 ID
    road::RoadId GetRoadId() const {
      return _signal->GetRoadId();
    }
 // 获取搜索到该地标时的距离
    double GetDistance() const {
      return _distance_from_search;
    }
// 获取地标在道路上的纵向位置
    double GetS() const {
      return _signal->GetS();
    }
// 获取地标在道路上的横向位置
    double GetT() const {
      return _signal->GetT();
    }
// 获取地标的唯一标识
    std::string GetId() const {
      return _signal->GetSignalId();
    }
// 获取地标的名称
    std::string GetName() const {
      return _signal->GetSignal()->GetName();
    }
// 判断地标是否是动态的
    bool IsDynamic() const {
      return _signal->IsDynamic();
    }
// 获取地标的方向
    road::SignalOrientation GetOrientation() const {
      return _signal->GetOrientation();
    }
// 获取地标的 Z 轴偏移量
    double GetZOffset() const {
      return _signal->GetSignal()->GetZOffset();
    }
// 获取地标的所属国家
    std::string GetCountry() const {
      return _signal->GetSignal()->GetCountry();
    }
// 获取地标的类型
    std::string GetType() const {
      return _signal->GetSignal()->GetType();
    }
// 获取地标的子类型
    std::string GetSubType() const {
      return _signal->GetSignal()->GetSubtype();
    }
// 获取地标的数值（如果有）
    double GetValue() const {
      return _signal->GetSignal()->GetValue();
    }
// 获取地标的单位（如果有）
    std::string GetUnit() const {
      return _signal->GetSignal()->GetUnit();
    }
// 获取地标的高度
    double GetHeight() const {
      return _signal->GetSignal()->GetHeight();
    }
// 获取地标的宽度
    double GetWidth() const {
      return _signal->GetSignal()->GetWidth();
    }
// 获取地标的文本信息（如果有）
    std::string GetText() const {
      return _signal->GetSignal()->GetText();
    }
// 获取地标的水平偏移量
    double GethOffset() const {
      return _signal->GetSignal()->GetHOffset();
    }
// 获取地标的俯仰角
    double GetPitch() const {
      return _signal->GetSignal()->GetPitch();
    }
// 获取地标的滚转角
    double GetRoll() const {
      return _signal->GetSignal()->GetRoll();
    }
// 获取地标的有效性信息
    const auto &GetValidities() const {
      return _signal->GetValidities();
    }

  private:
// 友元类声明，允许这些类访问私有成员
    friend Waypoint;
    friend Map;
// 构造函数，用于创建地标对象
    Landmark(
        SharedPtr<Waypoint> waypoint,
        SharedPtr<const Map> parent,
        const road::element::RoadInfoSignal* signal,
        double distance_from_search = 0)
      : _waypoint(waypoint),
        _parent(parent),
        _signal(signal),
        _distance_from_search(distance_from_search) {}

    /// 信号影响的路径点
    SharedPtr<Waypoint> _waypoint;

    SharedPtr<const Map> _parent;

    const road::element::RoadInfoSignal* _signal;

    double _distance_from_search;
  };

} // client
} // carla
