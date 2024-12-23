// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Memory.h" // 引入Carla的Memory头文件，提供内存相关的功能
#include "carla/client/Waypoint.h" // 引入Carla的客户端Waypoint头文件，包含关于道路上位置点（Waypoint）的一些信息
#include "carla/geom/Transform.h" // 引入Carla的几何变换Transform头文件，用于处理与位置、旋转相关的变换操作
#include "carla/geom/BoundingBox.h" // 引入Carla的BoundingBox头文件，用于定义车辆或物体的边界框（BoundingBox）

#include "carla/road/element/RoadInfoSignal.h" // 引入Carla道路模块中的RoadInfoSignal头文件，涉及到道路信息及其信号处理

#include <string> // 引入标准库中的字符串头文件，用于处理字符串类型数据

namespace carla {
namespace client {

  /// Landmark 类表示地图中的一个地标，该地标与某个信号相关联。
  /// 包含关于地标的各种属性和信息，可以用来在地图上找到该地标以及获取其相关数据。
  class Landmark {
  public:
    /// 获取与该地标相关联的路径点（Waypoint）。
    /// Waypoint 表示地图上某一位置的详细信息。
    SharedPtr<Waypoint> GetWaypoint() const {
      return _waypoint;
    }
    /// 获取地标的变换信息（位置信息和朝向），该信息来自关联的信号。
    const geom::Transform &GetTransform() const {
      return _signal->GetSignal()->GetTransform();
    }
    /// 获取地标所在的道路 ID。
    road::RoadId GetRoadId() const {
      return _signal->GetRoadId();
    }
    /// 获取从搜索地标到该地标的距离。
    /// 该距离是与搜索的起点相关的，可以用来进行距离测量。
    double GetDistance() const {
      return _distance_from_search;
    }
    /// 获取地标在道路上的纵向位置（S 值），通常与路径上的进度或位置相关。
    double GetS() const {
      return _signal->GetS();
    }
    /// 获取地标在道路上的横向位置（T 值），表示地标距离中心线的偏移量。
    double GetT() const {
      return _signal->GetT();
    }
    /// 获取地标的唯一标识符。
    /// 该标识符在整个系统中是唯一的，用来区分不同的地标。
    std::string GetId() const {
      return _signal->GetSignalId();
    }
    /// 获取地标的名称。
    /// 该名称可能是与地标相关的标签或标识，通常用于人类可读的显示。
    std::string GetName() const {
      return _signal->GetSignal()->GetName();
    }
    /// 判断地标是否是动态的。
    /// 如果地标是动态的，则它的属性可能会随时间变化，例如交通信号灯的状态。
    bool IsDynamic() const {
      return _signal->IsDynamic();
    }
    /// 获取地标的方向信息。
    /// 该方向信息通常表示与道路上的行驶方向的关系。
    road::SignalOrientation GetOrientation() const {
      return _signal->GetOrientation();
    }
    /// 获取地标的 Z 轴偏移量。
    /// Z 偏移量指示该地标在垂直方向上的位置偏移。
    double GetZOffset() const {
      return _signal->GetSignal()->GetZOffset();
    }
    /// 获取地标所属的国家名称。
    /// 该属性可能与地标的交通规则或其他法律要求相关。
    std::string GetCountry() const {
      return _signal->GetSignal()->GetCountry();
    }
    /// 获取地标的类型。
    /// 该类型可以用来标识地标是信号灯、标志牌或其他类型的地标。
    std::string GetType() const {
      return _signal->GetSignal()->GetType();
    }
    /// 获取地标的子类型。
    /// 子类型进一步细化地标的具体类型，例如不同类型的交通标志或灯光。
    std::string GetSubType() const {
      return _signal->GetSignal()->GetSubtype();
    }
    /// 获取地标的数值属性（如果有）。
    /// 该数值可能表示速度限制、信号状态等。
    double GetValue() const {
      return _signal->GetSignal()->GetValue();
    }
    /// 获取地标的单位（如果有）。
    /// 该单位可以是如 km/h 或 m 等，表示地标的数值所对应的度量单位。
    std::string GetUnit() const {
      return _signal->GetSignal()->GetUnit();
    }
    /// 获取地标的高度。
    /// 高度通常表示地标在垂直方向上的尺寸或位置。
    double GetHeight() const {
      return _signal->GetSignal()->GetHeight();
    }
    /// 获取地标的宽度。
    /// 宽度通常表示地标的横向尺寸。
    double GetWidth() const {
      return _signal->GetSignal()->GetWidth();
    }
    /// 获取地标的文本信息（如果有）。
    /// 该文本信息可以是与地标相关的说明或附加信息
    std::string GetText() const {
      return _signal->GetSignal()->GetText();
    }
    /// 获取地标的水平偏移量。
    /// 水平偏移量表示地标相对于道路中心线的水平位置偏移。
    double GethOffset() const {
      return _signal->GetSignal()->GetHOffset();
    }
    /// 获取地标的俯仰角。
    /// 俯仰角用于描述地标与地面之间的角度，通常与方向或朝向有关。
    double GetPitch() const {
      return _signal->GetSignal()->GetPitch();
    }
    /// 获取地标的滚转角。
    /// 滚转角用于描述地标的旋转，特别是其绕水平轴的旋转。
    double GetRoll() const {
      return _signal->GetSignal()->GetRoll();
    }
    /// 获取地标的有效性信息。
    /// 有效性信息描述地标是否在某些条件下有效，或其在某个时间范围内的可用性。
    const auto &GetValidities() const {
      return _signal->GetValidities();
    }

  private:
    // 友元类声明，允许这些类访问 Landmark 类的私有成员。
    friend Waypoint;
    friend Map;
    /// 构造函数，用于初始化 Landmark 对象。
    /// 参数 waypoint: 与地标关联的路径点信息。
    /// 参数 parent: 地图的父对象，表示地标所在的地图。
    /// 参数 signal: 与地标相关的信号对象，包含地标的详细信息。
    /// 参数 distance_from_search: 从搜索的起点到地标的距离。
    Landmark(
        SharedPtr<Waypoint> waypoint,
        SharedPtr<const Map> parent,
        const road::element::RoadInfoSignal* signal,
        double distance_from_search = 0)
      : _waypoint(waypoint),
        _parent(parent),
        _signal(signal),
        _distance_from_search(distance_from_search) {}

    /// 与地标关联的路径点（Waypoint）。
    SharedPtr<Waypoint> _waypoint;
    /// 地标所在的地图对象。
    SharedPtr<const Map> _parent;
    /// 与地标相关的信号对象，包含地标的详细信息。
    const road::element::RoadInfoSignal* _signal;
    /// 从搜索点到地标的距离。
    double _distance_from_search;
  };

} // client
} // carla
