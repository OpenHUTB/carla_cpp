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
 element::RoadInfoSignal* AddSignal( // 添加信号的函数
        Road* road, // 道路指针
        const SignId signal_id, // 信号标识符
        const double s, // 在道路上的位置（s坐标）
        const double t, // 在道路上的横向位置（t坐标）
        const std::string name, // 信号名称
        const std::string dynamic, // 动态属性
        const std::string orientation, // 方向
        const double zOffset, // Z轴偏移
        const std::string country, // 国家
        const std::string type, // 信号类型
        const std::string subtype, // 信号子类型
        const double value, // 信号值
        const std::string unit, // 单位
        const double height, // 高度
        const double width, // 宽度
        const std::string text, // 显示文本
        const double hOffset, // 水平偏移
        const double pitch, // 倾斜角度
        const double roll); // 翻滚角度

void AddSignalPositionInertial( // 添加惯性信号位置的函数
        const SignId signal_id, // 信号标识符
        const double x, // X坐标
        const double y, // Y坐标
        const double z, // Z坐标
        const double hdg, // 航向角
        const double pitch, // 倾斜角度
        const double roll); // 翻滚角度

void AddSignalPositionRoad( // 添加道路信号位置的函数
        const SignId signal_id, // 信号标识符
        const RoadId road_id, // 道路标识符
        const double s, // 在道路上的位置（s坐标）
        const double t, // 在道路上的横向位置（t坐标）
        const double zOffset, // Z轴偏移
        const double hOffset, // 水平偏移
        const double pitch, // 倾斜角度
        const double roll); // 翻滚角度

element::RoadInfoSignal* AddSignalReference( // 添加信号参考的函数
        Road* road, // 道路指针
        const SignId signal_id, // 信号标识符
        const double s_position, // 信号参考位置（s坐标）
        const double t_position, // 信号参考位置（t坐标）
        const std::string signal_reference_orientation); // 信号参考方向

void AddValidityToSignalReference( // 向信号参考添加有效性的函数
        element::RoadInfoSignal* signal_reference, // 信号参考指针
        const LaneId from_lane, // 起始车道标识符
        const LaneId to_lane); // 结束车道标识符

void AddDependencyToSignal( // 向信号添加依赖关系的函数
        const SignId signal_id, // 信号标识符
        const std::string dependency_id, // 依赖标识符
        const std::string dependency_type); // 依赖类型

// 从交叉口解析器调用
void AddJunction( // 添加交叉口的函数
        const JuncId id, // 交叉口标识符
        const std::string name); // 交叉口名称

void AddConnection( // 添加连接的函数
        const JuncId junction_id, // 交叉口标识符
        const ConId connection_id, // 连接标识符
        const RoadId incoming_road, // 进入道路标识符
        const RoadId connecting_road); // 连接道路标识符

void AddLaneLink( // 添加车道链接的函数
        const JuncId junction_id, // 交叉口标识符
        const ConId connection_id, // 连接标识符
        const LaneId from, // 起始车道标识符
        const LaneId to); // 目标车道标识符

void AddJunctionController( // 添加交叉口控制器的函数
        const JuncId junction_id, // 交叉口标识符
        std::set<ContId>&& controllers); // 控制器集合

void AddRoadSection( // 添加道路段的函数
        const RoadId road_id, // 道路标识符
        const SectionId section_index, // 道路段索引
        const double s, // 在道路段上的位置（s坐标）
        const double a, // 参数a
        const double b, // 参数b
        const double c, // 参数c
        const double d); // 参数d

void SetRoadLaneLink( // 设置道路车道链接的函数
        const RoadId road_id, // 道路标识符
        const SectionId section_index, // 道路段索引
        const LaneId lane_id, // 车道标识符
        const Lane::LaneType lane_type, // 车道类型
        const bool lane_level, // 是否为车道级别
        const LaneId predecessor, // 前驱车道标识符
        const LaneId successor); // 后继车道标识符

// 从车道解析器调用
  void CreateLaneAccess( // 创建车道通行权限
        Lane *lane, // 车道指针
        const double s, // 在车道上的位置（s坐标）
        const std::string restriction); // 限制类型

void CreateLaneBorder( // 创建车道边界
        Lane *lane, // 车道指针
        const double s, // 在车道上的位置（s坐标）
        const double a, // 参数a
        const double b, // 参数b
        const double c, // 参数c
        const double d); // 参数d

void CreateLaneHeight( // 创建车道高度
        Lane *lane, // 车道指针
        const double s, // 在车道上的位置（s坐标）
        const double inner, // 内部高度
        const double outer); // 外部高度

void CreateLaneMaterial( // 创建车道材料属性
        Lane *lane, // 车道指针
        const double s, // 在车道上的位置（s坐标）
        const std::string surface, // 表面类型
        const double friction, // 摩擦系数
        const double roughness); // 粗糙度

void CreateSectionOffset( // 创建道路段偏移
        Road *road, // 道路指针
        const double s, // 在道路段上的位置（s坐标）
        const double a, // 参数a
        const double b, // 参数b
        const double c, // 参数c
        const double d); // 参数d

void CreateLaneRule( // 创建车道规则
        Lane *lane, // 车道指针
        const double s, // 在车道上的位置（s坐标）
        const std::string value); // 规则值

void CreateLaneVisibility( // 创建车道可见性
        Lane *lane, // 车道指针
        const double s, // 在车道上的位置（s坐标）
        const double forward, // 前方可见距离
        const double back, // 后方可见距离
        const double left, // 左侧可见距离
        const double right); // 右侧可见距离

void CreateLaneWidth( // 创建车道宽度
        Lane *lane, // 车道指针
        const double s, // 在车道上的位置（s坐标）
        const double a, // 参数a
        const double b, // 参数b
        const double c, // 参数c
        const double d); // 参数d

void CreateRoadMark( // 创建道路标记
        Lane *lane, // 车道指针
        const int road_mark_id, // 道路标记标识符
        const double s, // 在车道上的位置（s坐标）
        const std::string type, // 标记类型
        const std::string weight, // 标记重量
        const std::string color, // 标记颜色
        const std::string material, // 标记材料
        const double width, // 标记宽度
        const std::string lane_change, // 车道变更信息
        const double height, // 标记高度
        const std::string type_name, // 标记类型名称
        const double type_width); // 标记类型宽度

void CreateRoadMarkTypeLine( // 创建道路标记类型线
        Lane *lane, // 车道指针
        const int road_mark_id, // 道路标记标识符
        const double length, // 线段长度
        const double space, // 线段间距
        const double tOffset, // 偏移量
        const double s, // 在车道上的位置（s坐标）
        const std::string rule, // 规则
        const double width); // 线宽

void CreateRoadSpeed( // 创建道路速度限制
        Road *road, // 道路指针
        const double s, // 在道路上的位置（s坐标）
        const std::string type, // 限速类型
        const double max, // 最大速度
        const std::string unit); // 单位

void CreateLaneSpeed( // 创建车道速度限制
        Lane *lane, // 车道指针
        const double s, // 在车道上的位置（s坐标）
        const double max, // 最大速度
        const std::string unit); // 单位

Road *GetRoad( // 获取道路对象
        const RoadId road_id); // 道路标识符

Lane *GetLane( // 获取车道对象
        const RoadId road_id, // 道路标识符
        const LaneId lane_id, // 车道标识符
        const double s); // 在车道上的位置（s坐标）

// 从控制器解析器调用
    void CreateController( // 创建控制器
        const ContId controller_id, // 控制器标识符
        const std::string controller_name, // 控制器名称
        const uint32_t controller_sequence, // 控制器序列号
        const std::set<road::SignId>&& signals // 控制器信号集合
        );

void SetGeoReference(const geom::GeoLocation &geo_reference) { // 设置地理参考
      _map_data._geo_reference = geo_reference; // 更新地图数据中的地理参考
}

private:

    MapData _map_data; // 地图数据

    /// Create the pointers between RoadSegments based on the ids. // 根据标识符创建道路段之间的指针
    void CreatePointersBetweenRoadSegments();

    /// Create the bounding boxes of each junction // 创建每个交叉口的边界框
    void CreateJunctionBoundingBoxes(Map &map);

    geom::Transform ComputeSignalTransform(std::unique_ptr<Signal> &signal,  MapData &data); // 计算信号变换

    /// Solves the signal references in the road // 解决道路中的信号引用
    void SolveSignalReferencesAndTransforms();

    /// Solve the references between Controllers and Juntions // 解决控制器和交叉口之间的引用
    void SolveControllerAndJuntionReferences();

    /// Compute the conflicts of the roads (intersecting roads) // 计算道路冲突（相交道路）
    void ComputeJunctionRoadConflicts(Map &map);

    /// Generates a default validity field for signal references with missing validity record in OpenDRIVE // 为缺少有效性记录的信号引用生成默认有效性字段
    void GenerateDefaultValiditiesForSignalReferences();

    /// Removes signal references with lane validity equal to [0,0] // 移除车道有效性等于[0,0]的信号引用
    /// as they have no effect on any road
    void RemoveZeroLaneValiditySignalReferences();

    /// Checks signals overlapping driving lanes and emits a warning // 检查重叠驾驶车道的信号并发出警告
    void CheckSignalsOnRoads(Map &map);

    /// Return the pointer to a lane object. // 返回车道对象的指针
    Lane *GetEdgeLanePointer(RoadId road_id, bool from_start, LaneId lane_id);

    /// Return a list of pointers to all lanes from a lane (using road and // 返回从某车道到所有车道的指针列表（使用道路和
    /// junction info). // 交叉口信息）
    std::vector<Lane *> GetLaneNext(
        RoadId road_id, // 道路标识符
        SectionId section_id, // 段落标识符
        LaneId lane_id); // 车道标识符

    std::vector<std::pair<RoadId, LaneId>> GetJunctionLanes( // 获取交叉口车道
        JuncId junction_id, // 交叉口标识符
        RoadId road_id, // 道路标识符
        LaneId lane_id); // 车道标识符

    /// Map to temporary store all the road and lane infos until the map is // 用于临时存储所有道路和车道信息，直到地图构建完成，因此可以一起添加
    /// built, so they can be added all together.
    std::unordered_map<Road *, std::vector<std::unique_ptr<element::RoadInfo>>>
        _temp_road_info_container; // 临时道路信息容器

    std::unordered_map<Lane *, std::vector<std::unique_ptr<element::RoadInfo>>>
        _temp_lane_info_container; // 临时车道信息容器

    std::unordered_map<SignId, std::unique_ptr<Signal>>
        _temp_signal_container; // 临时信号容器

    std::vector<element::RoadInfoSignal*> _temp_signal_reference_container; // 临时信号引用容器

}; // 类结束

} // namespace road
} // namespace carla