// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/StringUtil.h" // 引入字符串工具库
#include "carla/road/MapBuilder.h" // 引入地图构建器
#include "carla/road/element/RoadInfoElevation.h" // 引入道路高度信息类
#include "carla/road/element/RoadInfoGeometry.h" // 引入道路几何信息类
#include "carla/road/element/RoadInfoLaneAccess.h" // 引入车道访问信息类
#include "carla/road/element/RoadInfoLaneBorder.h" // 引入车道边界信息类
#include "carla/road/element/RoadInfoLaneHeight.h" // 引入车道高度信息类
#include "carla/road/element/RoadInfoLaneMaterial.h" // 引入车道材料信息类
#include "carla/road/element/RoadInfoLaneOffset.h" // 引入车道偏移信息类
#include "carla/road/element/RoadInfoLaneRule.h" // 引入车道规则信息类
#include "carla/road/element/RoadInfoLaneVisibility.h" // 引入车道可见性信息类
#include "carla/road/element/RoadInfoLaneWidth.h" // 引入车道宽度信息类
#include "carla/road/element/RoadInfoMarkRecord.h" // 引入道路标记记录类
#include "carla/road/element/RoadInfoMarkTypeLine.h" // 引入道路标记类型线类
#include "carla/road/element/RoadInfoSpeed.h" // 引入道路速度信息类
#include "carla/road/element/RoadInfoSignal.h" // 引入道路信号信息类
#include "carla/road/element/RoadInfoVisitor.h" // 引入道路信息访问者类
#include "carla/road/element/RoadInfoCrosswalk.h" // 引入人行横道信息类
#include "carla/road/InformationSet.h" // 引入信息集合类
#include "carla/road/Signal.h" // 引入信号类
#include "carla/road/SignalType.h" // 引入信号类型类

#include <iterator> // 引入迭代器相关库
#include <memory> // 引入智能指针库
#include <algorithm> // 引入算法库

using namespace carla::road::element; // 使用carla::road::element命名空间

namespace carla {
namespace road {

  boost::optional<Map> MapBuilder::Build() {

    CreatePointersBetweenRoadSegments(); // 创建路段之间的指针
    RemoveZeroLaneValiditySignalReferences(); // 移除无效车道信号引用

    for (auto &&info : _temp_road_info_container) { // 遍历临时道路信息容器
      DEBUG_ASSERT(info.first != nullptr); // 确保道路信息不为空
      info.first->_info = InformationSet(std::move(info.second)); // 移动并设置道路信息
    }

    for (auto &&info : _temp_lane_info_container) { // 遍历临时车道信息容器
      DEBUG_ASSERT(info.first != nullptr); // 确保车道信息不为空
      info.first->_info = InformationSet(std::move(info.second)); // 移动并设置车道信息
    }

    // compute transform requires the roads to have the RoadInfo
    SolveSignalReferencesAndTransforms(); // 解决信号引用和变换

    SolveControllerAndJuntionReferences(); // 解决控制器和交叉口引用

    // remove temporal already used information
    _temp_road_info_container.clear(); // 清空临时道路信息容器
    _temp_lane_info_container.clear(); // 清空临时车道信息容器

    // _map_data is a member of MapBuilder so you must especify if
    // you want to keep it (will return copy -> Map(const Map &))
    // or move it (will return move -> Map(Map &&))
    Map map(std::move(_map_data)); // 移动并创建地图对象
    CreateJunctionBoundingBoxes(map); // 创建交叉口的边界框
    ComputeJunctionRoadConflicts(map); // 计算交叉口道路冲突
    CheckSignalsOnRoads(map); // 检查道路上的信号

    return map; // 返回构建的地图
  }

  // called from profiles parser
  void MapBuilder::AddRoadElevationProfile(
      Road *road, // 道路指针
      const double s, // 路段位置
      const double a, // 高度参数a
      const double b, // 高度参数b
      const double c, // 高度参数c
      const double d) { // 高度参数d
    DEBUG_ASSERT(road != nullptr); // 确保道路不为空
    auto elevation = std::make_unique<RoadInfoElevation>(s, a, b, c, d); // 创建道路高度信息
    _temp_road_info_container[road].emplace_back(std::move(elevation)); // 添加到临时道路信息容器
  }

  void MapBuilder::AddRoadObjectCrosswalk(
      Road *road, // 道路指针
      const std::string name, // 人行横道名称
      const double s, // 路段位置
      const double t, // 路段横坐标
      const double zOffset, // 高度偏移
      const double hdg, // 偏航角
      const double pitch, // 俯仰角
      const double roll, // 翻滚角
      const std::string orientation, // 定位方向
      const double width, // 宽度
      const double length, // 长度
      const std::vector<road::element::CrosswalkPoint> points) { // 人行横道点集
    DEBUG_ASSERT(road != nullptr); // 确保道路不为空
    auto cross = std::make_unique<RoadInfoCrosswalk>(s, name, t, zOffset, hdg, pitch, roll, std::move(orientation), width, length, std::move(points)); // 创建人行横道信息
    _temp_road_info_container[road].emplace_back(std::move(cross)); // 添加到临时道路信息容器
  }

  // called from lane parser
  void MapBuilder::CreateLaneAccess(
      Lane *lane, // 车道指针
      const double s, // 位置
      const std::string restriction) { // 限制条件
    DEBUG_ASSERT(lane != nullptr); // 确保车道不为空
    _temp_lane_info_container[lane].emplace_back(std::make_unique<RoadInfoLaneAccess>(s, restriction)); // 创建车道访问信息并添加到临时车道信息容器
  }

  void MapBuilder::CreateLaneBorder(
      Lane *lane, // 车道指针
      const double s, // 位置
      const double a, // 边界参数a
      const double b, // 边界参数b
      const double c, // 边界参数c
      const double d) { // 边界参数d
    DEBUG_ASSERT(lane != nullptr); // 确保车道不为空
    _temp_lane_info_container[lane].emplace_back(std::make_unique<RoadInfoLaneBorder>(s, a, b, c, d)); // 创建车道边界信息并添加到临时车道信息容器
  }

  void MapBuilder::CreateLaneHeight(
      Lane *lane, // 车道指针
      const double s, // 位置
      const double inner, // 内部高度
      const double outer) { // 外部高度
    DEBUG_ASSERT(lane != nullptr); // 确保车道不为空
    _temp_lane_info_container[lane].emplace_back(std::make_unique<RoadInfoLaneHeight>(s, inner, outer)); // 创建车道高度信息并添加到临时车道信息容器
  }

  void MapBuilder::CreateLaneMaterial(
      Lane *lane, // 车道指针
      const double s, // 位置
      const std::string surface, // 表面材料
      const double friction, // 摩擦系数
      const double roughness) { // 粗糙度
    DEBUG_ASSERT(lane != nullptr); // 确保车道不为空
    _temp_lane_info_container[lane].emplace_back(std::make_unique<RoadInfoLaneMaterial>(s, surface, friction,
        roughness)); // 创建车道材料信息并添加到临时车道信息容器
  }

  void MapBuilder::CreateLaneRule(
      Lane *lane, // 车道指针
      const double s, // 位置
      const std::string value) { // 规则值
    DEBUG_ASSERT(lane != nullptr); // 确保车道不为空
    _temp_lane_info_container[lane].emplace_back(std::make_unique<RoadInfoLaneRule>(s, value)); // 创建车道规则信息并添加到临时车道信息容器
  }

  void MapBuilder::CreateLaneVisibility(
      Lane *lane, // 车道指针
      const double s, // 位置
      const double forward, // 前方可见性
      const double back, // 后方可见性
      const double left, // 左侧可见性
      const double right) { // 右侧可见性
    DEBUG_ASSERT(lane != nullptr); // 确保车道不为空
    _temp_lane_info_container[lane].emplace_back(std::make_unique<RoadInfoLaneVisibility>(s, forward, back,
        left, right)); // 创建车道可见性信息并添加到临时车道信息容器
  }

  void MapBuilder::CreateLaneWidth(
      Lane *lane, // 车道指针
      const double s, // 位置
      const double a, // 宽度参数a
      const double b, // 宽度参数b
      const double c, // 宽度参数c
      const double d) { // 宽度参数d
    DEBUG_ASSERT(lane != nullptr); // 确保车道不为空
    _temp_lane_info_container[lane].emplace_back(std::make_unique<RoadInfoLaneWidth>(s, a, b, c, d)); // 创建车道宽度信息并添加到临时车道信息容器
  }

  void MapBuilder::CreateRoadMark(
      Lane *lane, // 车道指针
      const int road_mark_id, // 道路标记ID
      const double s, // 位置
      const std::string type, // 标记类型
      const std::string weight, // 标记重量
      const std::string color, // 标记颜色
      const std::string material, // 标记材料
      const double width, // 标记宽度
      std::string lane_change, // 车道变更类型
      const double height, // 标记高度
      const std::string type_name, // 类型名称
      const double type_width) { // 类型宽度
    DEBUG_ASSERT(lane != nullptr); // 确保车道不为空
    RoadInfoMarkRecord::LaneChange lc; // 定义车道变更类型

    StringUtil::ToLower(lane_change); // 将车道变更类型转换为小写

    if (lane_change == "increase") { // 如果是增加
      lc = RoadInfoMarkRecord::LaneChange::Increase; // 设置为增加
    } else if (lane_change == "decrease") { // 如果是减少
      lc = RoadInfoMarkRecord::LaneChange::Decrease; // 设置为减少
    } else if (lane_change == "none") { // 如果没有
      lc = RoadInfoMarkRecord::LaneChange::None; // 设置为无
    } else { // 其他情况
      lc = RoadInfoMarkRecord::LaneChange::Both; // 设置为双向
    }
    _temp_lane_info_container[lane].emplace_back(std::make_unique<RoadInfoMarkRecord>(s, road_mark_id, type,
        weight, color,
        material, width, lc, height, type_name, type_width)); // 创建道路标记记录并添加到临时车道信息容器
  }
  void MapBuilder::CreateRoadMarkTypeLine(
      Lane *lane,  // 道路车道指针
      const int road_mark_id,  // 道路标记ID
      const double length,  // 标记线长度
      const double space,  // 标记线间距
      const double tOffset,  // 垂直偏移量
      const double s,  // 纵向位置
      const std::string rule,  // 规则字符串
      const double width) {  // 标记线宽度
    DEBUG_ASSERT(lane != nullptr);  // 确保车道指针不为空
    auto it = MakeRoadInfoIterator<RoadInfoMarkRecord>(_temp_lane_info_container[lane]);  // 创建道路信息迭代器
    for (; !it.IsAtEnd(); ++it) {  // 遍历道路信息
      if (it->GetRoadMarkId() == road_mark_id) {  // 如果找到匹配的道路标记ID
        it->GetLines().emplace_back(std::make_unique<RoadInfoMarkTypeLine>(s, road_mark_id, length, space,
            tOffset, rule, width));  // 添加新的标记线对象
        break;  // 跳出循环
      }
    }
}

void MapBuilder::CreateLaneSpeed(
      Lane *lane,  // 道路车道指针
      const double s,  // 纵向位置
      const double max,  // 最大速度
      const std::string /*unit*/) {  // 单位（未使用）
    DEBUG_ASSERT(lane != nullptr);  // 确保车道指针不为空
    _temp_lane_info_container[lane].emplace_back(std::make_unique<RoadInfoSpeed>(s, max));  // 添加车道速度信息
}

element::RoadInfoSignal* MapBuilder::AddSignal(
      Road* road,  // 道路指针
      const SignId signal_id,  // 信号ID
      const double s,  // 纵向位置
      const double t,  // 横向位置
      const std::string name,  // 信号名称
      const std::string dynamic,  // 动态信息
      const std::string orientation,  // 方向
      const double zOffset,  // 高度偏移量
      const std::string country,  // 国家
      const std::string type,  // 信号类型
      const std::string subtype,  // 信号子类型
      const double value,  // 信号值
      const std::string unit,  // 单位
      const double height,  // 信号高度
      const double width,  // 信号宽度
      const std::string text,  // 文字内容
      const double hOffset,  // 水平偏移量
      const double pitch,  // 俯仰角
      const double roll) {  // 横滚角
    _temp_signal_container[signal_id] = std::make_unique<Signal>(  // 创建信号对象并存储
        road->GetId(),  // 获取道路ID
        signal_id,  // 信号ID
        s,  // 纵向位置
        t,  // 横向位置
        name,  // 信号名称
        dynamic,  // 动态信息
        orientation,  // 方向
        zOffset,  // 高度偏移量
        country,  // 国家
        type,  // 信号类型
        subtype,  // 信号子类型
        value,  // 信号值
        unit,  // 单位
        height,  // 信号高度
        width,  // 信号宽度
        text,  // 文字内容
        hOffset,  // 水平偏移量
        pitch,  // 俯仰角
        roll);  // 横滚角

    return AddSignalReference(road, signal_id, s, t, orientation);  // 添加信号引用并返回
}

void MapBuilder::AddSignalPositionInertial(
      const SignId signal_id,  // 信号ID
      const double x,  // X坐标
      const double y,  // Y坐标
      const double z,  // Z坐标
      const double hdg,  // 航向角
      const double pitch,  // 俯仰角
      const double roll) {  // 横滚角
    std::unique_ptr<Signal> &signal = _temp_signal_container[signal_id];  // 获取信号对象引用
    signal->_using_inertial_position = true;  // 设置为使用惯性位置
    geom::Location location = geom::Location(x, -y, z);  // 创建位置对象，注意Y轴取反
    signal->_transform = geom::Transform(location, geom::Rotation(  // 创建变换对象
        geom::Math::ToDegrees(static_cast<float>(pitch)),  // 俯仰角转换为度
        geom::Math::ToDegrees(static_cast<float>(-hdg)),  // 航向角转换为度并取反
        geom::Math::ToDegrees(static_cast<float>(roll))));  // 横滚角转换为度
}

void MapBuilder::AddSignalPositionRoad(
      const SignId signal_id,  // 信号ID
      const RoadId road_id,  // 道路ID
      const double s,  // 纵向位置
      const double t,  // 横向位置
      const double zOffset,  // 高度偏移量
      const double hOffset,  // 水平偏移量
      const double pitch,  // 俯仰角
      const double roll) {  // 横滚角
    std::unique_ptr<Signal> &signal = _temp_signal_container[signal_id];  // 获取信号对象引用
    signal->_road_id = road_id;  // 设置道路ID
    signal->_s = s;  // 设置纵向位置
    signal->_t = t;  // 设置横向位置
    signal->_zOffset = zOffset;  // 设置高度偏移量
    signal->_hOffset = hOffset;  // 设置水平偏移量
    signal->_pitch = pitch;  // 设置俯仰角
    signal->_roll = roll;  // 设置横滚角
}

    element::RoadInfoSignal* MapBuilder::AddSignalReference(
        Road* road,
        const SignId signal_id,
        const double s_position,
        const double t_position,
        const std::string signal_reference_orientation) {

      const double epsilon = 0.00001; // 定义一个极小值，用于比较
      RELEASE_ASSERT(s_position >= 0.0); // 确保s_position为非负数
      // 防止s_position等于道路长度
      double fixed_s = geom::Math::Clamp(s_position, 0.0, road->GetLength() - epsilon);
      // 在临时容器中创建一个RoadInfoSignal对象并存储
      _temp_road_info_container[road].emplace_back(std::make_unique<element::RoadInfoSignal>(
          signal_id, road->GetId(), fixed_s, t_position, signal_reference_orientation));
      // 获取刚刚添加的信号引用
      auto road_info_signal = static_cast<element::RoadInfoSignal*>(
          _temp_road_info_container[road].back().get());
      // 将信号引用添加到临时信号参考容器中
      _temp_signal_reference_container.emplace_back(road_info_signal);
      return road_info_signal; // 返回信号引用
    }

    void MapBuilder::AddValidityToSignalReference(
        element::RoadInfoSignal* signal_reference,
        const LaneId from_lane,
        const LaneId to_lane) {
      // 为信号引用添加有效性信息
      signal_reference->_validities.emplace_back(LaneValidity(from_lane, to_lane));
    }

    void MapBuilder::AddDependencyToSignal(
        const SignId signal_id,
        const std::string dependency_id,
        const std::string dependency_type) {
      // 为信号添加依赖关系
      _temp_signal_container[signal_id]->_dependencies.emplace_back(
          SignalDependency(dependency_id, dependency_type));
    }

    // 构建道路对象
    carla::road::Road *MapBuilder::AddRoad(
        const RoadId road_id,
        const std::string name,
        const double length,
        const JuncId junction_id,
        const RoadId predecessor,
        const RoadId successor) {

      // 添加道路
      auto road = &(_map_data._roads.emplace(road_id, Road()).first->second);

      // 设置道路数据
      road->_map_data = &_map_data; // 设置地图数据指针
      road->_id = road_id; // 设置道路ID
      road->_name = name; // 设置道路名称
      road->_length = length; // 设置道路长度
      road->_junction_id = junction_id; // 设置交叉口ID
      (junction_id != -1) ? road->_is_junction = true : road->_is_junction = false; // 判断是否为交叉口
      road->_successor = successor; // 设置后继道路
      road->_predecessor = predecessor; // 设置前驱道路

      return road; // 返回道路对象
  }

  carla::road::LaneSection *MapBuilder::AddRoadSection(
      Road *road,
      const SectionId id,
      const double s) {
    DEBUG_ASSERT(road != nullptr); // 确保道路不为空
    carla::road::LaneSection &sec = road->_lane_sections.Emplace(id, s); // 在道路中添加车道段
    sec._road = road; // 设置车道段所属道路
    return &sec; // 返回车道段对象
  }

  carla::road::Lane *MapBuilder::AddRoadSectionLane(
      carla::road::LaneSection *section,
      const int32_t lane_id,
      const uint32_t lane_type,
      const bool lane_level,
      const int32_t predecessor,
      const int32_t successor) {
    DEBUG_ASSERT(section != nullptr); // 确保车道段不为空

    // 添加车道
    auto *lane = &((section->_lanes.emplace(lane_id, Lane()).first)->second);

    // 设置车道数据
    lane->_id = lane_id; // 设置车道ID
    lane->_lane_section = section; // 设置所属车道段
    lane->_level = lane_level; // 设置车道等级
    lane->_type = static_cast<carla::road::Lane::LaneType>(lane_type); // 设置车道类型
    lane->_successor = successor; // 设置后继车道
    lane->_predecessor = predecessor; // 设置前驱车道

    return lane; // 返回车道对象
  }

  void MapBuilder::AddRoadGeometryLine(
      Road *road,
      const double s,
      const double x,
      const double y,
      const double hdg,
      const double length) {
    DEBUG_ASSERT(road != nullptr); // 确保道路不为空
    const geom::Location location(static_cast<float>(x), static_cast<float>(y), 0.0f); // 创建位置对象
    // 创建几何线对象
    auto line_geometry = std::make_unique<GeometryLine>(
        s,
        length,
        hdg,
        location);

    _// 将新的道路几何信息添加到临时道路信息容器中
_temp_road_info_container[road].emplace_back(std::unique_ptr<RoadInfo>(new RoadInfoGeometry(s,
        std::move(line_geometry))));
  }

// 创建道路速度信息
void MapBuilder::CreateRoadSpeed(
      Road *road,
      const double s,
      const std::string /*type*/,
      const double max,
      const std::string /*unit*/) {
    DEBUG_ASSERT(road != nullptr); // 确保道路指针不为nullptr
    // 将新的速度信息添加到临时道路信息容器中
    _temp_road_info_container[road].emplace_back(std::make_unique<RoadInfoSpeed>(s, max));
  }

// 创建道路段偏移信息
void MapBuilder::CreateSectionOffset(
      Road *road,
      const double s,
      const double a,
      const double b,
      const double c,
      const double d) {
    DEBUG_ASSERT(road != nullptr); // 确保道路指针不为nullptr
    // 将新的车道偏移信息添加到临时道路信息容器中
    _temp_road_info_container[road].emplace_back(std::make_unique<RoadInfoLaneOffset>(s, a, b, c, d));
  }

// 添加道路几何弧形信息
void MapBuilder::AddRoadGeometryArc(
      Road *road,
      const double s,
      const double x,
      const double y,
      const double hdg,
      const double length,
      const double curvature) {
    DEBUG_ASSERT(road != nullptr); // 确保道路指针不为nullptr
    // 创建位置对象
    const geom::Location location(static_cast<float>(x), static_cast<float>(y), 0.0f);
    // 创建弧形几何对象
    auto arc_geometry = std::make_unique<GeometryArc>(
        s,
        length,
        hdg,
        location,
        curvature);

    // 将新的道路几何信息（弧形）添加到临时道路信息容器中
    _temp_road_info_container[road].emplace_back(std::unique_ptr<RoadInfo>(new RoadInfoGeometry(s,
        std::move(arc_geometry))));
  }

// 添加道路几何螺旋形信息
void MapBuilder::AddRoadGeometrySpiral(
      Road * road,
      const double s,
      const double x,
      const double y,
      const double hdg,
      const double length,
      const double curvStart,
      const double curvEnd) {
    // throw_exception(std::runtime_error("geometry spiral not supported"));
    DEBUG_ASSERT(road != nullptr); // 确保道路指针不为nullptr
    // 创建位置对象
    const geom::Location location(static_cast<float>(x), static_cast<float>(y), 0.0f);
    // 创建螺旋几何对象
    auto spiral_geometry = std::make_unique<GeometrySpiral>(
        s,
        length,
        hdg,
        location,
        curvStart,
        curvEnd);

    // 将新的道路几何信息（螺旋）添加到临时道路信息容器中
    _temp_road_info_container[road].emplace_back(std::unique_ptr<RoadInfo>(new RoadInfoGeometry(s,
        std::move(spiral_geometry))));
  }

// 添加道路几何三次多项式信息
void MapBuilder::AddRoadGeometryPoly3(
      Road * road,
      const double s,
      const double x,
      const double y,
      const double hdg,
      const double length,
      const double a,
      const double b,
      const double c,
      const double d) {
    // throw_exception(std::runtime_error("geometry poly3 not supported"));
   DEBUG_ASSERT(road != nullptr);  // 确保 road 指针不为空
const geom::Location location(static_cast<float>(x), static_cast<float>(y), 0.0f);  // 创建一个位置对象，坐标为 (x, y)，高度为 0.0f
auto poly3_geometry = std::make_unique<GeometryPoly3>(  // 创建一个 GeometryPoly3 对象的智能指针
    s,
    length,
    hdg,
    location,
    a,
    b,
    c,
    d);
_temp_road_info_container[road].emplace_back(std::unique_ptr<RoadInfo>(new RoadInfoGeometry(s,  // 将 RoadInfoGeometry 对象添加到临时道路信息容器中
    std::move(poly3_geometry))));  // 移动 poly3_geometry 智能指针

void MapBuilder::AddRoadGeometryParamPoly3(  // 定义一个函数，用于添加参数化的三次曲线道路几何信息
    Road * road,  // 道路指针
    const double s,  // 曲线长度
    const double x,  // x 坐标
    const double y,  // y 坐标
    const double hdg,  // 航向角
    const double length,  // 道路长度
    const double aU,  // 参数 U 的系数 a
    const double bU,  // 参数 U 的系数 b
    const double cU,  // 参数 U 的系数 c
    const double dU,  // 参数 U 的系数 d
    const double aV,  // 参数 V 的系数 a
    const double bV,  // 参数 V 的系数 b
    const double cV,  // 参数 V 的系数 c
    const double dV,  // 参数 V 的系数 d
    const std::string p_range) {  // 参数范围类型
    //throw_exception(std::runtime_error("geometry poly3 not supported"));  // 抛出异常，表示不支持三次曲线几何（已注释）
    bool arcLength;  // 声明一个布尔变量，用于判断弧长
    if(p_range == "arcLength"){  // 如果参数范围是弧长
      arcLength = true;  // 设置 arcLength 为 true
    }else{  // 否则
      arcLength = false;  // 设置 arcLength 为 false
    }
    DEBUG_ASSERT(road != nullptr);  // 确保 road 指针不为空
    const geom::Location location(static_cast<float>(x), static_cast<float>(y), 0.0f);  // 创建一个位置对象
    auto parampoly3_geometry = std::make_unique<GeometryParamPoly3>(  // 创建一个 GeometryParamPoly3 对象的智能指针
        s,
        length,
        hdg,
        location,
        aU,
        bU,
        cU,
        dU,
        aV,
        bV,
        cV,
        dV,
        arcLength);  // 将 arcLength 作为参数传入
    _temp_road_info_container[road].emplace_back(std::unique_ptr<RoadInfo>(new RoadInfoGeometry(s,  // 将 RoadInfoGeometry 对象添加到临时道路信息容器中
        std::move(parampoly3_geometry))));  // 移动 parampoly3_geometry 智能指针

void MapBuilder::AddJunction(const int32_t id, const std::string name) {  // 定义一个函数，用于添加交叉口
    _map_data.GetJunctions().emplace(id, Junction(id, name));  // 在地图数据中添加交叉口
}

void MapBuilder::AddConnection(  // 定义一个函数，用于添加交叉口连接
    const JuncId junction_id,  // 交叉口 ID
    const ConId connection_id,  // 连接 ID
    const RoadId incoming_road,  // 输入道路 ID
    const RoadId connecting_road) {  // 连接道路 ID
    DEBUG_ASSERT(_map_data.GetJunction(junction_id) != nullptr);  // 确保交叉口存在
    _map_data.GetJunction(junction_id)->GetConnections().emplace(connection_id,  // 在交叉口中添加连接
        Junction::Connection(connection_id, incoming_road, connecting_road));  // 创建连接对象并添加
}

void MapBuilder::AddLaneLink(  // 定义一个函数，用于添加车道链接
    const JuncId junction_id,  // 交叉口 ID
    const ConId connection_id,  // 连接 ID
    const LaneId from,  // 起始车道 ID
    const LaneId to) {  // 目标车道 ID
    DEBUG_ASSERT(_map_data.GetJunction(junction_id) != nullptr);  // 确保交叉口存在
    _map_data.GetJunction(junction_id)->GetConnection(connection_id)->AddLaneLink(from, to);  // 添加车道链接
}

void MapBuilder::AddJunctionController(  // 定义一个函数，用于添加交叉口控制器
    const JuncId junction_id,  // 交叉口 ID
    std::set<road::ContId>&& controllers) {  // 控制器集合
    DEBUG_ASSERT(_map_data.GetJunction(junction_id) != nullptr);  // 确保交叉口存在
    _map_data.GetJunction(junction_id)->_controllers = std::move(controllers);  // 移动控制器集合到交叉口
}

  Lane *MapBuilder::GetLane(
      const RoadId road_id,
      const LaneId lane_id,
      const double s) {
    return &_map_data.GetRoad(road_id).GetLaneByDistance(s, lane_id);
  }

  Road *MapBuilder::GetRoad(
      const RoadId road_id) {
    return &_map_data.GetRoad(road_id);
  }

  // return the pointer to a lane object
  Lane *MapBuilder::GetEdgeLanePointer(RoadId road_id, bool from_start, LaneId lane_id) {

    if (!_map_data.ContainsRoad(road_id)) {
      return nullptr;
    }
    Road &road = _map_data.GetRoad(road_id);

    // get the lane section
    LaneSection *section;
    if (from_start) {
      section = road.GetStartSection(lane_id);
    } else {
      section = road.GetEndSection(lane_id);
    }

    // get the lane
    DEBUG_ASSERT(section != nullptr);
    return section->GetLane(lane_id);
  }

  // return a list of pointers to all lanes from a lane (using road and junction
  // info)
  std::vector<Lane *> MapBuilder::GetLaneNext(
      RoadId road_id,
      SectionId section_id,
      LaneId lane_id) {
    std::vector<Lane *> result;

    if (!_map_data.ContainsRoad(road_id)) {
      return result;
    }
    Road &road = _map_data.GetRoad(road_id);

    // get the section
    LaneSection &section = road._lane_sections.GetById(section_id);

    // get the lane
    Lane *lane = section.GetLane(lane_id);
    DEBUG_ASSERT(lane != nullptr);

    // successor and predecessor (road and lane)
    LaneId next;
    RoadId next_road;
    if (lane_id <= 0) {
      next_road = road.GetSuccessor();
      next = lane->GetSuccessor();
    } else {
      next_road = road.GetPredecessor();
      next = lane->GetPredecessor();
    }

    // check to see if next is a road or a junction
    bool next_is_junction = !_map_data.ContainsRoad(next_road);
    double s = section.GetDistance();

    // check if we are in a lane section in the middle
    if ((lane_id > 0 && s > 0) ||
        (lane_id <= 0 && road._lane_sections.upper_bound(s) != road._lane_sections.end())) {
      // check if lane has a next link (if not, it deads in the middle section)
      if (next != 0 || (lane_id == 0 && next == 0)) {
        // change to next / prev section
        if (lane_id <= 0) {
          result.push_back(road.GetNextLane(s, next));
        } else {
          result.push_back(road.GetPrevLane(s, next));
        }
      }
    } else if (!next_is_junction) {
      // change to another road / junction
      if (next != 0 || (lane_id == 0 && next == 0)) {
        // single road
        result.push_back(GetEdgeLanePointer(next_road, (next <= 0), next));
      }
    } else {
      // several roads (junction)

      /// @todo Is it correct to use a road id as section id? (NS: I just added
      /// this cast to avoid compiler warnings).
      auto next_road_as_junction = static_cast<JuncId>(next_road);
      auto options = GetJunctionLanes(next_road_as_junction, road_id, lane_id);
      for (auto opt : options) {
        result.push_back(GetEdgeLanePointer(opt.first, (opt.second <= 0), opt.second));
      }
    }

    return result;
  }

  std::vector<std::pair<RoadId, LaneId>> MapBuilder::GetJunctionLanes(
      JuncId junction_id,
      RoadId road_id,
      LaneId lane_id) {
    std::vector<std::pair<RoadId, LaneId>> result;

    // get the junction
    Junction *junction = _map_data.GetJunction(junction_id);
    if (junction == nullptr) {
      return result;
    }

    // check all connections
    for (auto con : junction->_connections) {
      // only connections for our road
      if (con.second.incoming_road == road_id) {
        // for center lane it is always next lane id 0, we don't need to search
        // because it is not in the junction
        if (lane_id == 0) {
          result.push_back(std::make_pair(con.second.connecting_road, 0));
        } else {
          // check all lane links
          for (auto link : con.second.lane_links) {
            // is our lane id ?
            if (link.from == lane_id) {
              // add as option
              result.push_back(std::make_pair(con.second.connecting_road, link.to));
            }
          }
        }
      }
    }

    return result;
  }

  // assign pointers to the next lanes
  void MapBuilder::CreatePointersBetweenRoadSegments(void) {
    // process each lane to define its nexts
    for (auto &road : _map_data._roads) {
      for (auto &section : road.second._lane_sections) {
        for (auto &lane : section.second._lanes) {

          // assign the next lane pointers
          lane.second._next_lanes = GetLaneNext(road.first, section.second._id, lane.first);

          // add to each lane found, this as its predecessor
          for (auto next_lane : lane.second._next_lanes) {
            // add as previous
            DEBUG_ASSERT(next_lane != nullptr);
            next_lane->_prev_lanes.push_back(&lane.second);
          }

        }
      }
    }

    // process each lane to define its nexts
    for (auto &road : _map_data._roads) {
      for (auto &section : road.second._lane_sections) {
        for (auto &lane : section.second._lanes) {

          // add next roads
          for (auto next_lane : lane.second._next_lanes) {
            DEBUG_ASSERT(next_lane != nullptr);
            // avoid same road
            if (next_lane->GetRoad() != &road.second) {
              if (std::find(road.second._nexts.begin(), road.second._nexts.end(),
                  next_lane->GetRoad()) == road.second._nexts.end()) {
                road.second._nexts.push_back(next_lane->GetRoad());
              }
            }
          }

          // add prev roads
          for (auto prev_lane : lane.second._prev_lanes) {
            DEBUG_ASSERT(prev_lane != nullptr);
            // avoid same road
            if (prev_lane->GetRoad() != &road.second) {
              if (std::find(road.second._prevs.begin(), road.second._prevs.end(),
                  prev_lane->GetRoad()) == road.second._prevs.end()) {
                road.second._prevs.push_back(prev_lane->GetRoad());
              }
            }
          }

        }
      }
    }
  }

  geom::Transform MapBuilder::ComputeSignalTransform(std::unique_ptr<Signal> &signal, MapData &data) {
    DirectedPoint point = data.GetRoad(signal->_road_id).GetDirectedPointInNoLaneOffset(signal->_s);
    point.ApplyLateralOffset(static_cast<float>(-signal->_t));
    point.location.y *= -1; // Unreal Y axis hack
    point.location.z += static_cast<float>(signal->_zOffset);
    geom::Transform transform(point.location, geom::Rotation(
        geom::Math::ToDegrees(static_cast<float>(signal->_pitch)),
        geom::Math::ToDegrees(static_cast<float>(-(point.tangent + signal->_hOffset))),
        geom::Math::ToDegrees(static_cast<float>(signal->_roll))));
    return transform;
  }

  void MapBuilder::SolveSignalReferencesAndTransforms() {
    for(auto signal_reference : _temp_signal_reference_container){
      signal_reference->_signal =
          _temp_signal_container[signal_reference->_signal_id].get();
    }

    for(auto& signal_pair : _temp_signal_container) {
      auto& signal = signal_pair.second;
      if (signal->_using_inertial_position) {
        continue;
      }
      auto transform = ComputeSignalTransform(signal, _map_data);
      if (SignalType::IsTrafficLight(signal->GetType())) {
        transform.location = transform.location +
            geom::Location(transform.GetForwardVector()*0.25);
      }
      signal->_transform = transform;
    }

    _map_data._signals = std::move(_temp_signal_container);

    GenerateDefaultValiditiesForSignalReferences();
  }

  void MapBuilder::SolveControllerAndJuntionReferences() {
    for(const auto& junction : _map_data._junctions) {
      for(const auto& controller : junction.second._controllers) {
        auto it = _map_data._controllers.find(controller);
        if(it != _map_data._controllers.end()){
          if( it->second != nullptr ){
            it->second->_junctions.insert(junction.first);
            for(const auto & signal : it->second->_signals) {
              auto signal_it = _map_data._signals.find(signal);
              if( signal_it->second != nullptr ){
                signal_it->second->_controllers.insert(controller);
              }
            }
          }
        }
      }
    }
  }

  void MapBuilder::CreateJunctionBoundingBoxes(Map &map) {
    for (auto &junctionpair : map._data.GetJunctions()) {
      auto* junction = map.GetJunction(junctionpair.first);
      auto waypoints = map.GetJunctionWaypoints(junction->GetId(), Lane::LaneType::Any);
      const int number_intervals = 10;

      float minx = std::numeric_limits<float>::max();
      float miny = std::numeric_limits<float>::max();
      float minz = std::numeric_limits<float>::max();
      float maxx = -std::numeric_limits<float>::max();
      float maxy = -std::numeric_limits<float>::max();
      float maxz = -std::numeric_limits<float>::max();

      auto get_min_max = [&](geom::Location position) {
        if (position.x < minx) {
          minx = position.x;
        }
        if (position.y < miny) {
          miny = position.y;
        }
        if (position.z < minz) {
          minz = position.z;
        }

        if (position.x > maxx) {
          maxx = position.x;
        }
        if (position.y > maxy) {
          maxy = position.y;
        }
        if (position.z > maxz) {
          maxz = position.z;
        }
      };

      for (auto &waypoint_p : waypoints) {
        auto &waypoint_start = waypoint_p.first;
        auto &waypoint_end = waypoint_p.second;
        double interval = (waypoint_end.s - waypoint_start.s) / static_cast<double>(number_intervals);
        auto next_wp = waypoint_end;
        auto location = map.ComputeTransform(next_wp).location;

        get_min_max(location);

        next_wp = waypoint_start;
        location = map.ComputeTransform(next_wp).location;

        get_min_max(location);

        for (int i = 0; i < number_intervals; ++i) {
          if (interval < std::numeric_limits<double>::epsilon())
            break;
          auto next = map.GetNext(next_wp, interval);
          if(next.size()){
            next_wp = next.back();
          }

          location = map.ComputeTransform(next_wp).location;
          get_min_max(location);
        }
      }
      carla::geom::Location location(0.5f * (maxx + minx), 0.5f * (maxy + miny), 0.5f * (maxz + minz));
      carla::geom::Vector3D extent(0.5f * (maxx - minx), 0.5f * (maxy - miny), 0.5f * (maxz - minz));

      junction->_bounding_box = carla::geom::BoundingBox(location, extent);
    }
  }

void MapBuilder::CreateController(
  const ContId controller_id,
  const std::string controller_name,
  const uint32_t controller_sequence,
  const std::set<road::SignId>&& signals) {

    // Add the Controller to MapData
    auto controller_pair = _map_data._controllers.emplace(
      std::make_pair(
          controller_id,
          std::make_unique<Controller>(controller_id, controller_name, controller_sequence)));

    DEBUG_ASSERT(controller_pair.first != _map_data._controllers.end());
    DEBUG_ASSERT(controller_pair.first->second);

    // Add the signals owned by the controller
    controller_pair.first->second->_signals = std::move(signals);

    // Add ContId to the signal owned by this Controller
    auto& signals_map = _map_data._signals;
    for(auto signal: signals) {
      auto it = signals_map.find(signal);
      if(it != signals_map.end()) {
        it->second->_controllers.insert(signal);
      }
    }
}

  void MapBuilder::ComputeJunctionRoadConflicts(Map &map) {
    for (auto &junctionpair : map._data.GetJunctions()) {
      auto& junction = junctionpair.second;
      junction._road_conflicts = (map.ComputeJunctionConflicts(junction.GetId()));
    }
  }

  void MapBuilder::GenerateDefaultValiditiesForSignalReferences() {
    for (auto * signal_reference : _temp_signal_reference_container) {
      if (signal_reference->_validities.size() == 0) {
        Road* road = GetRoad(signal_reference->GetRoadId());
        auto lanes = road->GetLanesByDistance(signal_reference->GetS());
        switch (signal_reference->GetOrientation()) {
          case SignalOrientation::Positive: {
            LaneId min_lane = 1;
            LaneId max_lane = 0;
            for (const auto* lane : lanes) {
              auto lane_id = lane->GetId();
              if(lane_id > max_lane) {
                max_lane = lane_id;
              }
            }
            if(min_lane <= max_lane) {
              AddValidityToSignalReference(signal_reference, min_lane, max_lane);
            }
            break;
          }
          case SignalOrientation::Negative: {
            LaneId min_lane = 0;
            LaneId max_lane = -1;
            for (const auto* lane : lanes) {
              auto lane_id = lane->GetId();
              if(lane_id < min_lane) {
                min_lane = lane_id;
              }
            }
            if(min_lane <= max_lane) {
              AddValidityToSignalReference(signal_reference, min_lane, max_lane);
            }
            break;
          }
          case SignalOrientation::Both: {
            // Get positive lanes
            LaneId min_lane = 1;
            LaneId max_lane = 0;
            for (const auto* lane : lanes) {
              auto lane_id = lane->GetId();
              if(lane_id > max_lane) {
                max_lane = lane_id;
              }
            }
            if(min_lane <= max_lane) {
              AddValidityToSignalReference(signal_reference, min_lane, max_lane);
            }

            // get negative lanes
            min_lane = 0;
            max_lane = -1;
            for (const auto* lane : lanes) {
              auto lane_id = lane->GetId();
              if(lane_id < min_lane) {
                min_lane = lane_id;
              }
            }
            if(min_lane <= max_lane) {
              AddValidityToSignalReference(signal_reference, min_lane, max_lane);
            }
            break;
          }
        }
      }
    }
  }

  void MapBuilder::RemoveZeroLaneValiditySignalReferences() {
    std::vector<element::RoadInfoSignal*> elements_to_remove;
    for (auto * signal_reference : _temp_signal_reference_container) {
      bool should_remove = true;
      for (auto & lane_validity : signal_reference->_validities) {
        if ( (lane_validity._from_lane != 0) ||
             (lane_validity._to_lane != 0)) {
          should_remove = false;
          break;
        }
      }
      if (signal_reference->_validities.size() == 0) {
        should_remove = false;
      }
      if (should_remove) {
        elements_to_remove.push_back(signal_reference);
      }
    }
    for (auto* element : elements_to_remove) {
      auto road_id = element->GetRoadId();
      auto& road_info = _temp_road_info_container[GetRoad(road_id)];
      road_info.erase(std::remove_if(road_info.begin(), road_info.end(),
          [=] (auto& info_ptr) {
            return (info_ptr.get() == element);
          }), road_info.end());
      _temp_signal_reference_container.erase(std::remove(_temp_signal_reference_container.begin(),
          _temp_signal_reference_container.end(), element),
          _temp_signal_reference_container.end());
    }
  }

  void MapBuilder::CheckSignalsOnRoads(Map &map) {
    for (auto& signal_pair : map._data._signals) {
      auto& signal = signal_pair.second;
      auto signal_position = signal->GetTransform().location;
      auto signal_rotation = signal->GetTransform().rotation;
      auto closest_waypoint_to_signal =
          map.GetClosestWaypointOnRoad(signal_position,
          static_cast<int32_t>(carla::road::Lane::LaneType::Shoulder) |  static_cast<int32_t>(carla::road::Lane::LaneType::Driving));
      // workarround to not move stencil stop
      if (
          signal->GetName().find("Stencil_STOP") != std::string::npos ||
          signal->GetName().find("STATIC") != std::string::npos ||
          signal->_using_inertial_position) {
        continue;
      }
      if(closest_waypoint_to_signal) {
        auto road_transform = map.ComputeTransform(closest_waypoint_to_signal.get());
        auto distance_to_road = (road_transform.location -signal_position).Length();
        double lane_width = map.GetLaneWidth(closest_waypoint_to_signal.get());
        int displacement_direction = 1;
        int iter = 0;
        int MaxIter = 10;
        // Displaces signal until it finds a suitable spot
        while(distance_to_road < (lane_width * 0.7) && iter < MaxIter && displacement_direction != 0) {
          if(iter == 0) {
            log_debug("Traffic sign",
                signal->GetSignalId(),
                "overlaps a driving lane. Moving out of the road...");
          }

          auto right_waypoint = map.GetRight(closest_waypoint_to_signal.get());
          auto right_lane_type = (right_waypoint) ? map.GetLaneType(right_waypoint.get()) : carla::road::Lane::LaneType::None;

          auto left_waypoint = map.GetLeft(closest_waypoint_to_signal.get());
          auto left_lane_type = (left_waypoint) ? map.GetLaneType(left_waypoint.get()) : carla::road::Lane::LaneType::None;

          if (right_lane_type != carla::road::Lane::LaneType::Driving) {
            displacement_direction = 1;
          } else if (left_lane_type != carla::road::Lane::LaneType::Driving) {
            displacement_direction = -1;
          } else {
            displacement_direction = 0;
          }

          geom::Vector3D displacement = 1.f*(road_transform.GetRightVector()) *
              static_cast<float>(abs(lane_width))*0.2f;
          signal_position += (displacement * displacement_direction);
          signal_rotation = road_transform.rotation;
          closest_waypoint_to_signal =
              map.GetClosestWaypointOnRoad(signal_position,
              static_cast<int32_t>(carla::road::Lane::LaneType::Shoulder) |  static_cast<int32_t>(carla::road::Lane::LaneType::Driving));
          distance_to_road =
              (map.ComputeTransform(closest_waypoint_to_signal.get()).location -
              signal_position).Length();
          lane_width = map.GetLaneWidth(closest_waypoint_to_signal.get());
          iter++;
        }
        if(iter == MaxIter) {
          log_debug("Failed to find suitable place for signal.");
        } else {
          // Only perform the displacement if a good location has been found
          signal->_transform.location = signal_position;
          signal->_transform.rotation = signal_rotation;
        }
      }
    }
  }

} // namespace road
} // namespace carla
