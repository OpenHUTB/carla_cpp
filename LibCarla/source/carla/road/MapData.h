// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/geom/GeoLocation.h" // 引入地理位置类
#include "carla/Iterator.h" // 引入迭代器类
#include "carla/ListView.h" // 引入列表视图类
#include "carla/NonCopyable.h" // 引入不可拷贝类
#include "carla/road/Controller.h" // 引入控制器类
#include "carla/road/element/RoadInfo.h" // 引入道路信息类
#include "carla/road/Junction.h" // 引入交叉口类
#include "carla/road/Road.h" // 引入道路类
#include "carla/road/RoadTypes.h" // 引入道路类型定义
#include "carla/road/Signal.h" // 引入信号类

#include <boost/iterator/transform_iterator.hpp> // 引入 Boost 库的变换迭代器

#include <unordered_map> // 引入无序映射容器

namespace carla { // 定义 carla 命名空间
namespace road { // 定义 road 命名空间

  class Lane; // 前向声明车道类

  class MapData : private MovableNonCopyable { // 定义 MapData 类，继承不可拷贝类
  public:

    const geom::GeoLocation &GetGeoReference() const { // 获取地理参考
      return _geo_reference; // 返回地理位置引用
    }

    std::unordered_map<RoadId, Road> &GetRoads(); // 获取道路的可修改引用

    const std::unordered_map<RoadId, Road> &GetRoads() const { // 获取道路的常量引用
      return _roads; // 返回道路映射
    }

    std::unordered_map<JuncId, Junction> &GetJunctions(); // 获取交叉口的可修改引用

    const std::unordered_map<JuncId, Junction> &GetJunctions() const { // 获取交叉口的常量引用
      return _junctions; // 返回交叉口映射
    }

    bool ContainsRoad(RoadId id) const { // 检查是否包含特定道路
      return (_roads.find(id) != _roads.end()); // 返回是否找到该道路
    }

    Road &GetRoad(const RoadId id); // 获取指定道路的可修改引用

    const Road &GetRoad(const RoadId id) const; // 获取指定道路的常量引用

    Junction *GetJunction(JuncId id); // 获取指定交叉口的可修改指针

    const Junction *GetJunction(JuncId id) const; // 获取指定交叉口的常量指针

    template <typename T> // 模板函数，用于获取道路信息
    auto GetRoadInfo(const RoadId id, const double s) {
      return GetRoad(id).template GetInfo<T>(s); // 从指定道路获取信息
    }

    template <typename T> // 模板函数，用于获取车道信息
    auto GetLaneInfo(
        const RoadId road_id,
        const SectionId section_id,
        const LaneId lane_id,
        const double s) {
      return GetRoad(road_id).GetLaneById(section_id, lane_id).template GetInfo<T>(s); // 从指定车道获取信息
    }

    size_t GetRoadCount() const { // 获取道路数量
      return _roads.size(); // 返回道路映射的大小
    }

    const std::unordered_map<SignId, std::unique_ptr<Signal>> &GetSignals() const { // 获取信号的常量引用
      return _signals; // 返回信号映射
    }

    const std::unordered_map<ContId, std::unique_ptr<Controller>>& GetControllers() const { // 获取控制器的常量引用
      return _controllers; // 返回控制器映射
    }

  private:

    friend class MapBuilder; // 友元类声明

    MapData() = default; // 默认构造函数

    geom::GeoLocation _geo_reference; // 地理参考成员变量

    std::unordered_map<RoadId, Road> _roads; // 道路映射成员变量

    std::unordered_map<JuncId, Junction> _junctions; // 交叉口映射成员变量

    std::unordered_map<SignId, std::unique_ptr<Signal>> _signals; // 信号映射成员变量

    std::unordered_map<ContId, std::unique_ptr<Controller>> _controllers; // 控制器映射成员变量
  };

} // namespace road
} // namespace carla