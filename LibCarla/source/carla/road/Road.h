// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/geom/Mesh.h" // 引入 Mesh 类的定义
#include "carla/Iterator.h" // 引入 Iterator 类的定义
#include "carla/ListView.h" // 引入 ListView 类的定义
#include "carla/NonCopyable.h" // 引入 NonCopyable 类的定义
#include "carla/road/element/Geometry.h" // 引入 Geometry 相关元素的定义
#include "carla/road/element/RoadInfo.h" // 引入 RoadInfo 的定义
#include "carla/road/InformationSet.h" // 引入 InformationSet 的定义
#include "carla/road/Junction.h" // 引入 Junction 类的定义
#include "carla/road/LaneSection.h" // 引入 LaneSection 类的定义
#include "carla/road/LaneSectionMap.h" // 引入 LaneSectionMap 的定义
#include "carla/road/RoadElementSet.h" // 引入 RoadElementSet 的定义
#include "carla/road/RoadTypes.h" // 引入 RoadTypes 的定义

#include <unordered_map> // 引入 unordered_map
#include <vector> // 引入 vector

namespace carla {
namespace road {

  class MapData; // 前向声明 MapData 类
  class Elevation; // 前向声明 Elevation 类
  class MapBuilder; // 前向声明 MapBuilder 类

  class Road : private MovableNonCopyable { // 定义 Road 类，继承自 MovableNonCopyable
  public:

    Road() = default; // 默认构造函数

    const MapData *GetMap() const; // 获取地图数据的指针

    RoadId GetId() const; // 获取路段 ID

    std::string GetName() const; // 获取路段名称

    double GetLength() const; // 获取路段长度

    bool IsJunction() const; // 判断是否为交叉口

    JuncId GetJunctionId() const; // 获取交叉口 ID

    Lane &GetLaneByDistance(double s, LaneId lane_id); // 根据距离和车道 ID 获取车道引用

    const Lane &GetLaneByDistance(double s, LaneId lane_id) const; // 根据距离和车道 ID 获取常量车道引用

    /// 在特定的 s 值获取所有车道
    std::vector<Lane*> GetLanesByDistance(double s); // 返回在指定距离的车道指针

    std::vector<const Lane*> GetLanesByDistance(double s) const; // 返回在指定距离的常量车道指针

    RoadId GetSuccessor() const; // 获取下一个路段 ID

    RoadId GetPredecessor() const; // 获取前一个路段 ID

    Lane &GetLaneById(SectionId section_id, LaneId lane_id); // 根据区段 ID 和车道 ID 获取车道引用

    const Lane &GetLaneById(SectionId section_id, LaneId lane_id) const; // 根据区段 ID 和车道 ID 获取常量车道引用

    Lane *GetNextLane(const double s, const LaneId lane_id); // 获取下一个车道指针

    Lane *GetPrevLane(const double s, const LaneId lane_id); // 获取前一个车道指针

    /// 获取给定车道 ID 的起始车道段
    LaneSection *GetStartSection(LaneId id); // 根据车道 ID 获取起始车道段指针

    /// 获取给定车道 ID 的结束车道段
    LaneSection *GetEndSection(LaneId id); // 根据车道 ID 获取结束车道段指针

    std::vector<Road *> GetNexts() const; // 获取所有下一个路段的指针

    std::vector<Road *> GetPrevs() const; // 获取所有前一个路段的指针

    const geom::CubicPolynomial &GetElevationOn(const double s) const; // 获取指定距离点的高程信息

    /// 返回指定距离的中心点（车道 0）的导向点
    /// - @param s 距离路段的距离来计算点
    element::DirectedPoint GetDirectedPointIn(const double s) const; // 返回带有车道偏移和高程记录的导向点

    /// 返回指定距离的中心点（车道 0）的导向点，不考虑车道偏移
    /// - @param s 距离路段的距离来计算点
    element::DirectedPoint GetDirectedPointInNoLaneOffset(const double s) const; // 返回不考虑车道偏移的导向点

    /// 返回一个包含以下内容的对：
    /// - @b first: 从路段开始到最近点的距离（s）
    /// - @b second: 从此路段最近点到 p 的欧几里得距离
    ///   @param loc 要计算距离的点
    const std::pair<double, double> GetNearestPoint(
        const geom::Location &loc) const; // 获取到最近点的距离和欧几里得距离的对

    /// 返回最近的车道指针，给定相对于路段的 s 值和位置
    ///   @param dist 从路段起点到要计算的点的距离
    ///   @param loc 要计算距离的点
    const std::pair<const Lane *, double> GetNearestLane(
        const double s,
        const geom::Location &loc,
        uint32_t type = static_cast<uint32_t>(Lane::LaneType::Any)) const; // 获取最近车道及其距离的对

    template <typename T>
    const T *GetInfo(const double s) const { // 模板函数，根据距离获取信息指针
      return _info.GetInfo<T>(s);
    }

    template <typename T>
    std::vector<const T*> GetInfos() const { // 模板函数，获取所有信息的常量指针
      return _info.GetInfos<T>();
    }
template <typename T>
std::vector<const T*> GetInfosInRange(const double min_s, const double max_s) const {
    // 在指定范围内获取信息，并返回常量指针的向量
    return _info.GetInfos<T>(min_s, max_s);
}

auto GetLaneSections() const {
    // 获取车道段并返回一个视图
    return MakeListView(
        iterator::make_map_values_const_iterator(_lane_sections.begin()), // 从开始迭代器创建常量值迭代器
        iterator::make_map_values_const_iterator(_lane_sections.end()));   // 到结束迭代器创建常量值迭代器
}

private:

template <typename MultiMapT>
static auto GetLessEqualRange(MultiMapT &map, double s) {
    // 获取小于等于给定 s 值的范围
    if (map.find(s) == map.end()) { // 如果 s 不在地图中
        auto it = map.lower_bound(s); // 找到第一个大于 s 的迭代器
        if (it == map.begin()) { // 如果它是第一个元素
            return std::make_pair(map.end(), map.end()); // 返回结束迭代器的对
        }
        s = (--it)->first; // 向前移动一次，获取小于等于 s 的值
    }
    return std::make_pair(map.lower_bound(s), map.upper_bound(s)); // 返回小于等于和大于 s 的范围
}

public:

auto GetLaneSectionsAt(const double s) {
    // 获取在给定 s 位置的车道段
    auto pair = GetLessEqualRange(_lane_sections, s); // 获取小于等于 s 的范围
    return MakeListView(
        iterator::make_map_values_iterator(pair.first), // 创建值迭代器视图
        iterator::make_map_values_iterator(pair.second)); // 创建值迭代器视图
}

auto GetLaneSectionsAt(const double s) const {
    // 获取在给定 s 位置的车道段（常量版本）
    auto pair = GetLessEqualRange(_lane_sections, s); // 获取小于等于 s 的范围
    return MakeListView(
        iterator::make_map_values_const_iterator(pair.first), // 创建常量值迭代器视图
        iterator::make_map_values_const_iterator(pair.second)); // 创建常量值迭代器视图
}

LaneSection &GetLaneSectionById(SectionId id) {
    // 根据 ID 获取车道段引用
    return _lane_sections.GetById(id);
}

const LaneSection &GetLaneSectionById(SectionId id) const {
    // 根据 ID 获取车道段的常量引用
    return _lane_sections.GetById(id);
}

/// 返回上限 s，即给定 s 位置的车道段结束距离（限制在路段长度内）。
double UpperBound(double s) const {
    auto it = _lane_sections.upper_bound(s); // 获取大于 s 的第一个迭代器
    return it != _lane_sections.end() ? it->first : _length; // 如果找到，返回其值；否则返回路段长度
}

/// 获取在给定 s 位置的所有车道
std::map<LaneId, const Lane *> GetLanesAt(const double s) const;

private:

friend MapBuilder; // 声明 MapBuilder 为友元类

MapData *_map_data { nullptr }; // 地图数据指针，初始化为 nullptr

RoadId _id { 0 }; // 路段 ID，初始化为 0

std::string _name; // 路段名称

double _length { 0.0 }; // 路段长度，初始化为 0.0

bool _is_junction { false }; // 是否为交叉口，初始化为 false

JuncId _junction_id { -1 }; // 交叉口 ID，初始化为 -1

LaneSectionMap _lane_sections; // 车道段映射

RoadId _successor { 0 }; // 下一个路段 ID，初始化为 0

RoadId _predecessor { 0 }; // 前一个路段 ID，初始化为 0

InformationSet _info; // 信息集合

std::vector<Road *> _nexts; // 下一个路段的指针向量

std::vector<Road *> _prevs; // 前一个路段的指针向量
} // road
} // carla
