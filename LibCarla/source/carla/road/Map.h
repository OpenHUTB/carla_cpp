// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/geom/Mesh.h" // 包含Mesh类的定义
#include "carla/geom/Rtree.h" // 包含R树类的定义
#include "carla/geom/Transform.h" // 包含Transform类的定义
#include "carla/NonCopyable.h" // 包含不可复制类的定义
#include "carla/road/element/LaneMarking.h" // 包含车道标记类的定义
#include "carla/road/element/RoadInfoMarkRecord.h" // 包含道路信息标记记录类的定义
#include "carla/road/element/Waypoint.h" // 包含路径点类的定义
#include "carla/road/MapData.h" // 包含地图数据类的定义
#include "carla/road/RoadTypes.h" // 包含道路类型的定义
#include "carla/road/MeshFactory.h" // 包含网格工厂类的定义
#include "carla/geom/Vector3D.h" // 包含三维向量类的定义
#include "carla/rpc/OpendriveGenerationParameters.h" // 包含OpenDrive生成参数的定义

#include <boost/optional.hpp> // 包含可选类型的定义

#include <vector> // 包含向量类的定义

namespace carla {
namespace road {

  class Map : private MovableNonCopyable { // 地图类，禁止复制
  public:

    using Waypoint = element::Waypoint; // 定义Waypoint为element::Waypoint的别名

    /// ========================================================================
    /// -- Constructor ---------------------------------------------------------
    /// ========================================================================

    Map(MapData m) : _data(std::move(m)) { // 构造函数，初始化_map数据
      CreateRtree(); // 创建R树
    }

    /// ========================================================================
    /// -- Georeference --------------------------------------------------------
    /// ========================================================================

    const geom::GeoLocation &GetGeoReference() const { // 获取地理参考
      return _data.GetGeoReference(); // 返回地图数据中的地理参考
    }

    /// ========================================================================
    /// -- Geometry ------------------------------------------------------------
    /// ========================================================================

    boost::optional<element::Waypoint> GetClosestWaypointOnRoad( // 获取道路上最近的路径点
        const geom::Location &location, // 输入位置
        int32_t lane_type = static_cast<int32_t>(Lane::LaneType::Driving)) const; // 默认车道类型为驾驶车道

    boost::optional<element::Waypoint> GetWaypoint( // 获取指定位置的路径点
        const geom::Location &location, // 输入位置
        int32_t lane_type = static_cast<int32_t>(Lane::LaneType::Driving)) const; // 默认车道类型为驾驶车道

    boost::optional<element::Waypoint> GetWaypoint( // 根据道路ID和车道ID获取路径点
        RoadId road_id, // 道路ID
        LaneId lane_id, // 车道ID
        float s) const; // s表示沿车道的距离

    geom::Transform ComputeTransform(Waypoint waypoint) const; // 计算路径点的变换

    /// ========================================================================
    /// -- Road information ----------------------------------------------------
    /// ========================================================================

    const Lane &GetLane(Waypoint waypoint) const; // 获取路径点所在的车道

    Lane::LaneType GetLaneType(Waypoint waypoint) const; // 获取路径点所在车道的类型

    double GetLaneWidth(Waypoint waypoint) const; // 获取路径点所在车道的宽度

    JuncId GetJunctionId(RoadId road_id) const; // 获取道路的交叉口ID

    bool IsJunction(RoadId road_id) const; // 判断道路是否为交叉口

    std::pair<const element::RoadInfoMarkRecord *, const element::RoadInfoMarkRecord *> // 获取路径点的标记记录
        GetMarkRecord(Waypoint waypoint) const;

    std::vector<element::LaneMarking> CalculateCrossedLanes( // 计算经过的车道
        const geom::Location &origin, // 起始位置
        const geom::Location &destination) const; // 终止位置

    /// 返回定义二维区域的位置信息，重复位置表示一个区域结束
    std::vector<geom::Location> GetAllCrosswalkZones() const; // 获取所有人行道区域

    /// 数据结构用于信号搜索
    struct SignalSearchData { 
      const element::RoadInfoSignal *signal; // 信号指针
      Waypoint waypoint; // 路径点
      double accumulated_s = 0; // 累计距离
    };


     /// 从初始路点搜索信号，直到定义的距离。
    std::vector<SignalSearchData> GetSignalsInDistance(
        Waypoint waypoint, double distance, bool stop_at_junction = false) const; // 获取指定距离内的信号

    /// 返回地图中的所有 RoadInfoSignal
    std::vector<const element::RoadInfoSignal*>
        GetAllSignalReferences() const; // 获取所有信号的引用

    /// ========================================================================
    /// -- 路点生成 -------------------------------------------------------------
    /// ========================================================================

    /// 返回每个可行驶后续车道入口处的路点列表；即从 @a waypoint 出发，
    /// 车辆可以驶向的下一个路段的每个路点列表。
    std::vector<Waypoint> GetSuccessors(Waypoint waypoint) const; // 获取后继路点
    std::vector<Waypoint> GetPredecessors(Waypoint waypoint) const; // 获取前驱路点

    /// 返回距离 @a waypoint @a distance 的路点列表，
    /// 使得车辆可以驶向这些路点。
    std::vector<Waypoint> GetNext(Waypoint waypoint, double distance) const; // 获取下一个路点
    /// 返回距离 @a waypoint @a distance 的路点列表，
    /// 使得车辆可以反向驶向这些路点。
    std::vector<Waypoint> GetPrevious(Waypoint waypoint, double distance) const; // 获取上一个路点

    /// 返回 @a waypoint 右侧车道的路点。
    boost::optional<Waypoint> GetRight(Waypoint waypoint) const; // 获取右侧路点

    /// 返回 @a waypoint 左侧车道的路点。
    boost::optional<Waypoint> GetLeft(Waypoint waypoint) const; // 获取左侧路点

    /// 在 @a map 中生成所有路点，路点之间相隔 @a approx_distance。
    std::vector<Waypoint> GenerateWaypoints(double approx_distance) const; // 生成路点

    /// 在每个 @a lane 的入口处生成路点，
    /// 默认是行驶车道类型。
    std::vector<Waypoint> GenerateWaypointsOnRoadEntries(Lane::LaneType lane_type = Lane::LaneType::Driving) const; // 生成道路入口的路点

    /// 在指定道路的每个车道入口处生成路点。
    std::vector<Waypoint> GenerateWaypointsInRoad(RoadId road_id, Lane::LaneType lane_type = Lane::LaneType::Driving) const; // 生成道路上的路点

    /// 生成定义 @a map 拓扑结构的最小路点集。
    /// 路点放置在每个车道入口处。
    std::vector<std::pair<Waypoint, Waypoint>> GenerateTopology() const; // 生成拓扑结构

    /// 生成交叉口的路点。
    std::vector<std::pair<Waypoint, Waypoint>> GetJunctionWaypoints(JuncId id, Lane::LaneType lane_type) const; // 获取交叉口路点

    Junction* GetJunction(JuncId id); // 获取交叉口指针

    const Junction* GetJunction(JuncId id) const; // 获取交叉口常量指针

    std::unordered_map<road::RoadId, std::unordered_set<road::RoadId>>
        ComputeJunctionConflicts(JuncId id) const; // 计算交叉口冲突

    /// 基于 OpenDRIVE 构建网格
    geom::Mesh GenerateMesh(
        const double distance,
        const float extra_width = 0.6f,
        const  bool smooth_junctions = true) const; // 生成网格

    std::vector<std::unique_ptr<geom::Mesh>> GenerateChunkedMesh(
        const rpc::OpendriveGenerationParameters& params) const; // 生成分块网格

    std::map<road::Lane::LaneType , std::vector<std::unique_ptr<geom::Mesh>>>
      GenerateOrderedChunkedMeshInLocations( const rpc::OpendriveGenerationParameters& params,
                                             const geom::Vector3D& minpos,
                                             const geom::Vector3D& maxpos) const; // 在指定位置生成有序分块网格

/// Buids a mesh of all crosswalks based on the OpenDRIVE  // 基于OpenDRIVE构建所有人行横道的网格
geom::Mesh GetAllCrosswalkMesh() const;

std::vector<std::pair<geom::Transform, std::string>> GetTreesTransform(  // 获取树木的变换信息
  const geom::Vector3D& minpos,  // 最小位置
  const geom::Vector3D& maxpos,  // 最大位置
  float distancebetweentrees,  // 树木之间的距离
  float distancefromdrivinglineborder,  // 距离行车线边界的距离
  float s_offset = 0) const;  // 偏移量，默认值为0

geom::Mesh GenerateWalls(const double distance, const float wall_height) const;  // 生成墙体网格，参数为距离和墙高

// 构建与车道标记相关的网格列表
std::vector<std::unique_ptr<geom::Mesh>> GenerateLineMarkings(
  const rpc::OpendriveGenerationParameters& params,  // OpenDRIVE生成参数
  const geom::Vector3D& minpos,  // 最小位置
  const geom::Vector3D& maxpos,  // 最大位置
  std::vector<std::string>& outinfo ) const;  // 输出信息列表

const std::unordered_map<SignId, std::unique_ptr<Signal>>& GetSignals() const {  // 获取信号映射
  return _data.GetSignals();  // 返回信号数据
}

const std::unordered_map<ContId, std::unique_ptr<Controller>>& GetControllers() const {  // 获取控制器映射
  return _data.GetControllers();  // 返回控制器数据
}

std::vector<carla::geom::BoundingBox> GetJunctionsBoundingBoxes() const;  // 获取交叉口的边界框

#ifdef LIBCARLA_WITH_GTEST
    MapData &GetMap() {  // 获取地图数据
      return _data;  // 返回地图数据
    }
#endif // LIBCARLA_WITH_GTEST

private:

    friend MapBuilder;  // 友元类
    MapData _data;  // 地图数据

    using Rtree = geom::SegmentCloudRtree<Waypoint>;  // 使用R树结构
    Rtree _rtree;  // R树对象

    void CreateRtree();  // 创建R树

    // 辅助函数，用于构造R树元素列表
    void AddElementToRtree(  // 将元素添加到R树
        std::vector<Rtree::TreeElement> &rtree_elements,  // R树元素列表
        geom::Transform &current_transform,  // 当前变换
        geom::Transform &next_transform,  // 下一个变换
        Waypoint &current_waypoint,  // 当前路点
        Waypoint &next_waypoint);  // 下一个路点

    void AddElementToRtreeAndUpdateTransforms(  // 添加元素到R树并更新变换
        std::vector<Rtree::TreeElement> &rtree_elements,  // R树元素列表
        geom::Transform &current_transform,  // 当前变换
        Waypoint &current_waypoint,  // 当前路点
        Waypoint &next_waypoint);  // 下一个路点

public:
    inline float GetZPosInDeformation(float posx, float posy) const;  // 获取变形中的Z轴位置

    std::map<road::Lane::LaneType, std::vector<std::unique_ptr<geom::Mesh>>>  // 多线程生成道路网格
      GenerateRoadsMultithreaded( const carla::geom::MeshFactory& mesh_factory,  // 网格工厂
        const std::vector<RoadId>& RoadsID,  // 道路ID列表
        const size_t index,  // 当前索引
        const size_t number_of_roads_per_thread) const;  // 每个线程的道路数量

    void GenerateJunctions(const carla::geom::MeshFactory& mesh_factory,  // 生成交叉口
      const rpc::OpendriveGenerationParameters& params,  // OpenDRIVE生成参数
      const geom::Vector3D& minpos,  // 最小位置
      const geom::Vector3D& maxpos,  // 最大位置
      std::map<road::Lane::LaneType, std::vector<std::unique_ptr<geom::Mesh>>>*  // 输出交叉口网格列表
      juntion_out_mesh_list) const;

    void GenerateSingleJunction(const carla::geom::MeshFactory& mesh_factory,  // 生成单个交叉口
      const JuncId Id,  // 交叉口ID
      std::map<road::Lane::LaneType, std::vector<std::unique_ptr<geom::Mesh>>>*  // 输出交叉口网格列表
      junction_out_mesh_list) const;

    // 返回在指定位置之间的交叉口ID列表
    std::vector<JuncId> FilterJunctionsByPosition(  // 根据位置过滤交叉口
      const geom::Vector3D& minpos,  // 最小位置
      const geom::Vector3D& maxpos) const;  // 最大位置

    // 返回在指定位置之间的道路ID列表
    std::vector<RoadId> FilterRoadsByPosition(  // 根据位置过滤道路
      const geom::Vector3D& minpos,  // 最小位置
      const geom::Vector3D& maxpos ) const;  // 最大位置

    std::unique_ptr<geom::Mesh> SDFToMesh(const road::Junction& jinput, const std::vector<geom::Vector3D>& sdfinput, int grid_cells_per_dim) const;  // 将SDF转换为网格
  };

} // namespace road
} // namespace carla
