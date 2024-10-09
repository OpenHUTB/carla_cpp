// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <chrono>  // 引入时间相关的头文件
#include <memory>  // 引入智能指针相关的头文件
#include <unordered_map>  // 引入无序映射相关的头文件
#include <unordered_set>  // 引入无序集合相关的头文件

#if defined(__clang__)  // 如果使用的是clang编译器
#  pragma clang diagnostic push  // 保存当前诊断状态
#  pragma clang diagnostic ignored "-Wshadow"  // 忽略变量遮蔽警告
#endif
#include "boost/geometry.hpp"  // 引入Boost.Geometry库
#include "boost/geometry/geometries/point.hpp"  // 引入点几何体定义
#include "boost/geometry/index/rtree.hpp"  // 引入R树索引
#if defined(__clang__)  // 如果使用的是clang编译器
#  pragma clang diagnostic pop  // 恢复之前的诊断状态
#endif

#include "carla/client/Map.h"  // 引入CARLA客户端地图定义
#include "carla/client/Waypoint.h"  // 引入CARLA客户端路径点定义
#include "carla/geom/Location.h"  // 引入CARLA几何位置定义
#include "carla/geom/Math.h"  // 引入CARLA数学相关定义
#include "carla/Memory.h"  // 引入CARLA内存管理定义
#include "carla/road/RoadTypes.h"  // 引入CARLA道路类型定义

#include "carla/trafficmanager/RandomGenerator.h"  // 引入随机生成器定义
#include "carla/trafficmanager/SimpleWaypoint.h"  // 引入简单路径点定义
#include "carla/trafficmanager/CachedSimpleWaypoint.h"  // 引入缓存的简单路径点定义

namespace carla {
namespace traffic_manager {

namespace cg = carla::geom;  // 简化命名空间
namespace cc = carla::client;  // 简化客户端命名空间
namespace crd = carla::road;  // 简化道路命名空间
namespace bg = boost::geometry;  // 简化Boost.Geometry命名空间
namespace bgi = boost::geometry::index;  // 简化Boost.Geometry索引命名空间

using WaypointPtr = carla::SharedPtr<cc::Waypoint>;  // 定义路径点智能指针类型
using SimpleWaypointPtr = std::shared_ptr<SimpleWaypoint>;  // 定义简单路径点共享指针类型
using NodeList = std::vector<SimpleWaypointPtr>;  // 定义路径点列表类型
using GeoGridId = crd::JuncId;  // 定义地理网格ID类型
using WorldMap = carla::SharedPtr<const cc::Map>;  // 定义世界地图类型

using Point3D = bg::model::point<float, 3, bg::cs::cartesian>;  // 定义三维点类型
using Box = bg::model::box<Point3D>;  // 定义三维盒子类型
using SpatialTreeEntry = std::pair<Point3D, SimpleWaypointPtr>;  // 定义空间树条目类型

using SegmentId = std::tuple<crd::RoadId, crd::LaneId, crd::SectionId>;  // 定义段ID类型
using SegmentTopology = std::map<SegmentId, std::pair<std::vector<SegmentId>, std::vector<SegmentId>>>;  // 定义段拓扑图类型
using SegmentMap = std::map<SegmentId, std::vector<SimpleWaypointPtr>>;  // 定义段地图类型
using Rtree = bgi::rtree<SpatialTreeEntry, bgi::rstar<16>>;  // 定义R树类型

/// 此类构建一个离散的本地地图缓存。
/// 使用世界实例化该类并运行SetUp()构造本地地图。
class InMemoryMap {

private:

    /// 保存构造函数接收的世界地图对象。
    WorldMap _world_map;
    /// 存储所有自定义路径点对象的结构，经过稀疏拓扑插值处理。
    NodeList dense_topology;
    /// 用于索引和查询路径点的空间二维R树。
    Rtree rtree;

public:

    InMemoryMap(WorldMap world_map);  // 构造函数，接收世界地图
    ~InMemoryMap();  // 析构函数

    static void Cook(WorldMap world_map, const std::string& path);  // 静态方法，用于处理地图并保存到指定路径

    //bool Load(const std::string& filename);  // 加载地图的方法（未实现）
    bool Load(const std::vector<uint8_t>& content);  // 从字节内容加载地图的方法

    /// 此方法以采样分辨率构建本地地图。
    void SetUp();

    /// 此方法返回给定位置上最近的路径点。
    SimpleWaypointPtr GetWaypoint(const cg::Location loc) const;

    /// 此方法返回与自我车辆距离一定范围内的n个路径点。
    NodeList GetWaypointsInDelta(const cg::Location loc, const uint16_t n_points, const float random_sample) const;

    /// 此方法返回本地缓存中离散样本的完整列表。
    NodeList GetDenseTopology() const;

    std::string GetMapName();  // 获取地图名称

    const cc::Map& GetMap() const;  // 获取地图引用

private:
    void Save(const std::string& path);  // 保存地图到指定路径

    void SetUpDenseTopology();  // 设置稠密拓扑
    void SetUpSpatialTree();  // 设置空间树
    void SetUpRoadOption();  // 设置道路选项

    /// 此方法用于查找和链接车道变更连接。
    void FindAndLinkLaneChange(SimpleWaypointPtr reference_waypoint);

    NodeList GetSuccessors(const SegmentId segment_id,  // 获取后续路径点
                          const SegmentTopology &segment_topology,
                          const SegmentMap &segment_map);
    NodeList GetPredecessors(const SegmentId segment_id,  // 获取前驱路径点
                            const SegmentTopology &segment_topology,
                            const SegmentMap &segment_map);

    /// 计算给定路径点的段ID。
    /// ID考虑了OpenDrive的道路ID、车道ID和分段ID。
    SegmentId GetSegmentId(const WaypointPtr &wp) const;
    SegmentId GetSegmentId(const SimpleWaypointPtr &swp) const;
};

} // namespace traffic_manager
} // namespace carla