// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/road/Map.h" // 引入地图模块
#include "carla/road/element/RoadInfoCrosswalk.h" // 引入人行横道信息模块
#include "carla/road/element/RoadInfoSignal.h" // 引入交通信号信息模块

#include <boost/optional.hpp> // 引入可选类型模块

#include <map> // 引入映射容器模块

namespace carla {
namespace road {

  class MapBuilder {
  public:

    boost::optional<Map> Build(); // 构建地图并返回一个可选的地图对象

    // 从道路解析器调用
    carla::road::Road *AddRoad(
        const RoadId road_id, // 道路ID
        const std::string name, // 道路名称
        const double length, // 道路长度
        const JuncId junction_id, // 交叉口ID
        const RoadId predecessor, // 前驱道路ID
        const RoadId successor); // 后继道路ID

    carla::road::LaneSection *AddRoadSection(
        carla::road::Road *road, // 指向道路的指针
        const SectionId id, // 道路段ID
        const double s); // 位置参数

    carla::road::Lane *AddRoadSectionLane(
        carla::road::LaneSection *section, // 道路段指针
        const LaneId lane_id, // 车道ID
        const uint32_t lane_type, // 车道类型
        const bool lane_level, // 是否平坦
        const LaneId predecessor, // 前驱车道ID
        const LaneId successor); // 后继车道ID

    // 从几何解析器调用
    void AddRoadGeometryLine(
        carla::road::Road *road, // 指向道路的指针
        const double s, // 位置参数
        const double x, // x坐标
        const double y, // y坐标
        const double hdg, // 航向角
        const double length); // 线段长度

    void AddRoadGeometryArc(
        carla::road::Road *road, // 指向道路的指针
        const double s, // 位置参数
        const double x, // x坐标
        const double y, // y坐标
        const double hdg, // 航向角
        const double length, // 弧长
        const double curvature); // 曲率

    void AddRoadGeometrySpiral(
        carla::road::Road *road, // 指向道路的指针
        const double s, // 位置参数
        const double x, // x坐标
        const double y, // y坐标
        const double hdg, // 航向角
        const double length, // 螺旋长度
        const double curvStart, // 起始曲率
        const double curvEnd); // 结束曲率

    void AddRoadGeometryPoly3(
        carla::road::Road *road, // 指向道路的指针
        const double s, // 位置参数
        const double x, // x坐标
        const double y, // y坐标
        const double hdg, // 航向角
        const double length, // 多项式长度
        const double a, // 多项式系数a
        const double b, // 多项式系数b
        const double c, // 多项式系数c
        const double d); // 多项式系数d

    void AddRoadGeometryParamPoly3(
        carla::road::Road *road, // 指向道路的指针
        const double s, // 位置参数
        const double x, // x坐标
        const double y, // y坐标
        const double hdg, // 航向角
        const double length, // 多项式长度
        const double aU, // 多项式系数a (U方向)
        const double bU, // 多项式系数b (U方向)
        const double cU, // 多项式系数c (U方向)
        const double dU, // 多项式系数d (U方向)
        const double aV, // 多项式系数a (V方向)
        const double bV, // 多项式系数b (V方向)
        const double cV, // 多项式系数c (V方向)
        const double dV, // 多项式系数d (V方向)
        const std::string p_range); // 参数范围字符串

    // 从轮廓解析器调用
    void AddRoadElevationProfile(
        Road *road, // 指向道路的指针
        const double s, // 位置参数
        const double a, // 高程参数a
        const double b, // 高程参数b
        const double c, // 高程参数c
        const double d); // 高程参数d

    void AddRoadObjectCrosswalk(
        Road *road, // 指向道路的指针
        const std::string name, // 人行横道名称
        const double s, // 位置参数
        const double t, // 横向位置
        const double zOffset, // 垂直偏移
        const double hdg, // 航向角
        const double pitch, // 倾斜角
        const double roll, // 翻滚角
        const std::string orientation, // 定位方向
        const double width, // 宽度
        const double length, // 长度
        const std::vector<road::element::CrosswalkPoint> points); // 人行横道点集

    // void AddRoadLateralSuperElevation(
    //     Road* road,
    //     const double s,
    //     const double a,
    //     const double b,
    //     const double c,
    //     const double d);

    // void AddRoadLateralCrossfall(
    //     Road* road,
    //     const double s,
    //     const double a,
    //     const double b,
    //     const double c,
    //     const double d,
    //     const std::string side);

    // void AddRoadLateralShape(
    //     Road* road,
    //     const double s,
    //     const double a,
    //     const double b,
    //     const double c,
    //     const double d,
    //     const double t);

    // 信号方法   
 element::RoadInfoSignal* AddSignal(
        Road* road,
        const SignId signal_id,
        const double s,
        const double t,
        const std::string name,
        const std::string dynamic,
        const std::string orientation,
        const double zOffset,
        const std::string country,
        const std::string type,
        const std::string subtype,
        const double value,
        const std::string unit,
        const double height,
        const double width,
        const std::string text,
        const double hOffset,
        const double pitch,
        const double roll);

    void AddSignalPositionInertial(
        const SignId signal_id,
        const double x,
        const double y,
        const double z,
        const double hdg,
        const double pitch,
        const double roll);

    void AddSignalPositionRoad(
        const SignId signal_id,
        const RoadId road_id,
        const double s,
        const double t,
        const double zOffset,
        const double hOffset,
        const double pitch,
        const double roll);

    element::RoadInfoSignal* AddSignalReference(
        Road* road,
        const SignId signal_id,
        const double s_position,
        const double t_position,
        const std::string signal_reference_orientation);

    void AddValidityToSignalReference(
        element::RoadInfoSignal* signal_reference,
        const LaneId from_lane,
        const LaneId to_lane);

    void AddDependencyToSignal(
        const SignId signal_id,
        const std::string dependency_id,
        const std::string dependency_type);

    // called from junction parser
    void AddJunction(
        const JuncId id,
        const std::string name);

    void AddConnection(
        const JuncId junction_id,
        const ConId connection_id,
        const RoadId incoming_road,
        const RoadId connecting_road);

    void AddLaneLink(
        const JuncId junction_id,
        const ConId connection_id,
        const LaneId from,
        const LaneId to);

    void AddJunctionController(
        const JuncId junction_id,
        std::set<ContId>&& controllers);

    void AddRoadSection(
        const RoadId road_id,
        const SectionId section_index,
        const double s,
        const double a,
        const double b,
        const double c,
        const double d);

    void SetRoadLaneLink(
        const RoadId road_id,
        const SectionId section_index,
        const LaneId lane_id,
        const Lane::LaneType lane_type,
        const bool lane_level,
        const LaneId predecessor,
        const LaneId successor);

    // called from lane parser
    void CreateLaneAccess(
        Lane *lane,
        const double s,
        const std::string restriction);

    void CreateLaneBorder(
        Lane *lane,
        const double s,
        const double a,
        const double b,
        const double c,
        const double d);

    void CreateLaneHeight(
        Lane *lane,
        const double s,
        const double inner,
        const double outer);

    void CreateLaneMaterial(
        Lane *lane,
        const double s,
        const std::string surface,
        const double friction,
        const double roughness);

    void CreateSectionOffset(
        Road *road,
        const double s,
        const double a,
        const double b,
        const double c,
        const double d);

    void CreateLaneRule(
        Lane *lane,
        const double s,
        const std::string value);

    void CreateLaneVisibility(
        Lane *lane,
        const double s,
        const double forward,
        const double back,
        const double left,
        const double right);

    void CreateLaneWidth(
        Lane *lane,
        const double s,
        const double a,
        const double b,
        const double c,
        const double d);

    void CreateRoadMark(
        Lane *lane,
        const int road_mark_id,
        const double s,
        const std::string type,
        const std::string weight,
        const std::string color,
        const std::string material,
        const double width,
        const std::string lane_change,
        const double height,
        const std::string type_name,
        const double type_width);

    void CreateRoadMarkTypeLine(
        Lane *lane,
        const int road_mark_id,
        const double length,
        const double space,
        const double tOffset,
        const double s,
        const std::string rule,
        const double width);

    void CreateRoadSpeed(
        Road *road,
        const double s,
        const std::string type,
        const double max,
        const std::string unit);

    void CreateLaneSpeed(
        Lane *lane,
        const double s,
        const double max,
        const std::string unit);

    Road *GetRoad(
        const RoadId road_id);

    Lane *GetLane(
        const RoadId road_id,
        const LaneId lane_id,
        const double s);

    // Called from ControllerParser
    void CreateController(
        const ContId controller_id,
        const std::string controller_name,
        const uint32_t controller_sequence,
        const std::set<road::SignId>&& signals
        );



    void SetGeoReference(const geom::GeoLocation &geo_reference) {
      _map_data._geo_reference = geo_reference;
    }

  private:

    MapData _map_data;

    /// Create the pointers between RoadSegments based on the ids.
    void CreatePointersBetweenRoadSegments();

    /// Create the bounding boxes of each junction
    void CreateJunctionBoundingBoxes(Map &map);

    geom::Transform ComputeSignalTransform(std::unique_ptr<Signal> &signal,  MapData &data);

    /// Solves the signal references in the road
    void SolveSignalReferencesAndTransforms();

    /// Solve the references between Controllers and Juntions
    void SolveControllerAndJuntionReferences();

    /// Compute the conflicts of the roads (intersecting roads)
    void ComputeJunctionRoadConflicts(Map &map);

    /// Generates a default validity field for signal references with missing validity record in OpenDRIVE
    void GenerateDefaultValiditiesForSignalReferences();

    /// Removes signal references with lane validity equal to [0,0]
    /// as they have no effect on any road
    void RemoveZeroLaneValiditySignalReferences();

    /// Checks signals overlapping driving lanes and emits a warning
    void CheckSignalsOnRoads(Map &map);

    /// Return the pointer to a lane object.
    Lane *GetEdgeLanePointer(RoadId road_id, bool from_start, LaneId lane_id);

    /// Return a list of pointers to all lanes from a lane (using road and
    /// junction info).
    std::vector<Lane *> GetLaneNext(
        RoadId road_id,
        SectionId section_id,
        LaneId lane_id);

    std::vector<std::pair<RoadId, LaneId>> GetJunctionLanes(
        JuncId junction_id,
        RoadId road_id,
        LaneId lane_id);

    /// Map to temporary store all the road and lane infos until the map is
    /// built, so they can be added all together.
    std::unordered_map<Road *, std::vector<std::unique_ptr<element::RoadInfo>>>
        _temp_road_info_container;

    std::unordered_map<Lane *, std::vector<std::unique_ptr<element::RoadInfo>>>
        _temp_lane_info_container;

    std::unordered_map<SignId, std::unique_ptr<Signal>>
        _temp_signal_container;

    std::vector<element::RoadInfoSignal*> _temp_signal_reference_container;

  };

} // namespace road
} // namespace carla
