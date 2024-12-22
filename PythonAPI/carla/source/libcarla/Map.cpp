// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.


//使用Boost.Python库将一些C++函数和类方法暴露给Python，以便在Python中使用

#include <carla/FileSystem.h>
#include <carla/PythonUtil.h>
#include <carla/client/Junction.h>
#include <carla/client/Map.h>
#include <carla/client/Waypoint.h>
#include <carla/road/element/LaneMarking.h>
#include <carla/client/Landmark.h>
#include <carla/road/SignalType.h>

#include <ostream>
#include <fstream>

//定义两个重载的输出流运算符（operator<<），用于将 Map 和 Waypoint 类型的对象以文本形式输出到标准输出流
namespace carla {
namespace client {

  std::ostream &operator<<(std::ostream &out, const Map &map) {
    out << "Map(name=" << map.GetName() << ')';
    return out;
  }//接受一个 std::ostream 引用和一个 Map 类型的常量引用,使用传入的输出流 out 输出 Map 对象的名称

  std::ostream &operator<<(std::ostream &out, const Waypoint &waypoint) {
    out << "Waypoint(" << waypoint.GetTransform() << ')';
    return out;
  }//它接受一个 std::ostream 引用和一个 Waypoint 类型的常量引用,用传入的输出流 out 输出 Waypoint 对象的变换信息

} // namespace client
} // namespace carla
 
// 定义一个静态函数，用于将 OpenDRIVE 地图数据保存到磁盘
static void SaveOpenDriveToDisk(const carla::client::Map &self, std::string path) {
  // 释放全局解释器锁（GIL），以便在Python环境中进行多线程操作
  carla::PythonUtil::ReleaseGIL unlock;
  // 检查传入的 path 字符串是否为空
  if (path.empty()) {
    // 如果 path 为空，则将其设置为地图对象 self 的名称
    path = self.GetName();
  }
  // 使用 ValidateFilePath 函数验证文件路径，并确保文件扩展名为 .xodr
  carla::FileSystem::ValidateFilePath(path, ".xodr");
  // 打开指定文件
  std::ofstream out(path);
  // 将地图对象 self 中的 OpenDRIVE 数据写入文件
  out << self.GetOpenDrive() << std::endl;
}
 
// 定义一个静态函数，用于获取地图的拓扑结构
static auto GetTopology(const carla::client::Map &self) {
  // 引入 boost::python 命名空间，用于与 Python 交互
  namespace py = boost::python;
  // 获取地图的拓扑结构
  auto topology = self.GetTopology();
  // 创建一个 Python 列表，用于存储结果
  py::list result;
  // 遍历拓扑结构，并将每个键值对转换为 Python 元组后添加到结果列表中
  for (auto &&pair : topology) {
    result.append(py::make_tuple(pair.first, pair.second));
  }
  // 返回结果列表
  return result;
}
 
// 定义一个静态函数，用于获取交叉路口的车道点
static auto GetJunctionWaypoints(const carla::client::Junction &self, const carla::road::Lane::LaneType lane_type) {
  // 引入 boost::python 命名空间，用于与 Python 交互
  namespace py = boost::python;
  // 获取交叉路口的车道点
  auto topology = self.GetWaypoints(lane_type);
  // 创建一个 Python 列表，用于存储结果
  py::list result;
  // 遍历车道点，并将每个键值对转换为 Python 元组后添加到结果列表中
  for (auto &pair : topology) {
    result.append(py::make_tuple(pair.first, pair.second));
  }
  // 返回结果列表
  return result;
}

// 定义一个静态函数，用于获取地标的车道有效性
static auto GetLaneValidities(const carla::client::Landmark &self){
  // 引入 boost::python 命名空间，用于与 Python 交互
  namespace py = boost::python;  
  // 获取车道有效性信息
  auto &validities = self.GetValidities();
  // 创建一个 Python 列表，用于存储结果
  py::list result;
  // 遍历车道有效性信息，并将每个有效性信息转换为 Python 元组后添加到结果列表中
  for(auto &validity : validities) {
    result.append(py::make_tuple(validity._from_lane, validity._to_lane));
  }
  // 返回结果列表
  return result;
}
 
// 定义一个静态函数，用于将位置转换为地理坐标
static carla::geom::GeoLocation ToGeolocation(
    const carla::client::Map &self,
    const carla::geom::Location &location) {
  // 使用地图的地理参考信息将位置转换为地理坐标
  return self.GetGeoReference().Transform(location);
}

void export_map() {
  using namespace boost::python;
  namespace cc = carla::client;
  namespace cr = carla::road;
  namespace cg = carla::geom;
  namespace cre = carla::road::element;

  // ===========================================================================
  // -- Enums ------------------------------------------------------------------
  // ===========================================================================

 // 定义了名为"LaneType"的枚举，用于表示车道类型
// 每个枚举值对应不同的车道类型，例如None、Driving等
// 在Python3中None是保留字，这里有相应的特殊处理
enum_<cr::Lane::LaneType>("LaneType")
  // 定义每个枚举值与其对应的名称
   .value("NONE", cr::Lane::LaneType::None) // None is reserved in Python3
   .value("Driving", cr::Lane::LaneType::Driving)
   .value("Stop", cr::Lane::LaneType::Stop)
   .value("Shoulder", cr::Lane::LaneType::Shoulder)
   .value("Biking", cr::Lane::LaneType::Biking)
   .value("Sidewalk", cr::Lane::LaneType::Sidewalk)
   .value("Border", cr::Lane::LaneType::Border)
   .value("Restricted", cr::Lane::LaneType::Restricted)
   .value("Parking", cr::Lane::LaneType::Parking)
   .value("Bidirectional", cr::Lane::LaneType::Bidirectional)
   .value("Median", cr::Lane::LaneType::Median)
   .value("Special1", cr::Lane::LaneType::Special1)
   .value("Special2", cr::Lane::LaneType::Special2)
   .value("Special3", cr::Lane::LaneType::Special3)
   .value("RoadWorks", cr::Lane::LaneType::RoadWorks)
   .value("Tram", cr::Lane::LaneType::Tram)
   .value("Rail", cr::Lane::LaneType::Rail)
   .value("Entry", cr::Lane::LaneType::Entry)
   .value("Exit", cr::Lane::LaneType::Exit)
   .value("OffRamp", cr::Lane::LaneType::OffRamp)
   .value("OnRamp", cr::Lane::LaneType::OnRamp)
   .value("Any", cr::Lane::LaneType::Any)
  ;

// 定义了名为"LaneChange"的枚举，用于表示车道变更相关的类型
// 例如可以是无变更（None）、向右变更（Right）、向左变更（Left）或者双向变更（Both）
enum_<cre::LaneMarking::LaneChange>("LaneChange")
   .value("NONE", cre::LaneMarking::LaneChange::None)
   .value("Right", cre::LaneMarking::LaneChange::Right)
   .value("Left", cre::LaneMarking::LaneChange::Left)
   .value("Both", cre::LaneMarking::LaneChange::Both)
  ;

// 定义了名为"LaneMarkingColor"的枚举，用于表示车道标线的颜色类型
// 涵盖了标准色（Standard）以及常见的如蓝色（Blue）、绿色（Green）等多种颜色
enum_<cre::LaneMarking::Color>("LaneMarkingColor")
   .value("Standard", cre::LaneMarking::Color::Standard)
   .value("Blue", cre::LaneMarking::Color::Blue)
   .value("Green", cre::LaneMarking::Color::Green)
   .value("Red", cre::LaneMarking::Color::Red)
   .value("White", cre::LaneMarking::Color::White)
   .value("Yellow", cre::LaneMarking::Color::Yellow)
   .value("Other", cre::LaneMarking::Color::Other)
  ;

// 定义了名为"LaneMarkingType"的枚举，用于表示车道标线的类型
// 包含了诸如无标线（None）、其他类型（Other）以及各种不同样式的实线、虚线组合等类型
enum_<cre::LaneMarking::Type>("LaneMarkingType")
   .value("NONE", cre::LaneMarking::Type::None)
   .value("Other", cre::LaneMarking::Type::Other)
   .value("Broken", cre::LaneMarking::Type::Broken)
   .value("Solid", cre::LaneMarking::Type::Solid)
   .value("SolidSolid", cre::LaneMarking::Type::SolidSolid)
   .value("SolidBroken", cre::LaneMarking::Type::SolidBroken)
   .value("BrokenSolid", cre::LaneMarking::Type::BrokenSolid)
   .value("BrokenBroken", cre::LaneMarking::Type::BrokenBroken)
   .value("BottsDots", cre::LaneMarking::Type::BottsDots)
   .value("Grass", cre::LaneMarking::Type::Grass)
   .value("Curb", cre::LaneMarking::Type::Curb)
  ;

// 定义了名为"LandmarkOrientation"的枚举，用于表示信号方向相关的类型
// 例如正向（Positive）、负向（Negative）或者双向（Both）
enum_<cr::SignalOrientation>("LandmarkOrientation")
   .value("Positive", cr::SignalOrientation::Positive)
   .value("Negative", cr::SignalOrientation::Negative)
   .value("Both", cr::SignalOrientation::Both)
  ;

// ===========================================================================
// -- Map相关的类定义，用于处理地图相关操作和属性
// ===========================================================================

// 定义了名为"Map"的类，该类不可复制，使用智能指针进行管理
// 提供了多种初始化以及获取地图相关信息、操作地图的方法
class_<cc::Map, boost::noncopyable, boost::shared_ptr<cc::Map>>("Map", no_init)
    // 使用给定的名称和OpenDRIVE内容初始化地图对象
   .def(init<std::string, std::string>((arg("name"), arg("xodr_content"))))
    // 获取地图名称的属性
   .add_property("name", CALL_RETURNING_COPY(cc::Map, GetName))
    // 获取推荐的生成点列表
   .def("get_spawn_points", CALL_RETURNING_LIST(cc::Map, GetRecommendedSpawnPoints))
    // 根据位置获取路点，可指定是否投影到道路以及车道类型（默认是驾驶车道）
   .def("get_waypoint", &cc::Map::GetWaypoint, (arg("location"), arg("project_to_road")=true, arg("lane_type")=cr::Lane::LaneType::Driving))
    // 根据道路ID、车道ID和距离获取路点（基于OpenDRIVE格式相关参数）
   .def("get_waypoint_xodr", &cc::Map::GetWaypointXODR, (arg("road_id"), arg("lane_id"), arg("s")))
    // 获取地图拓扑结构的相关方法（这里具体函数未给出完整定义，可能在别处实现）
   .def("get_topology", &GetTopology)
    // 按照给定距离生成路点列表
   .def("generate_waypoints", CALL_RETURNING_LIST_1(cc::Map, GenerateWaypoints, double), (args("distance")))
    // 将给定位置转换为地理位置信息（具体转换逻辑在对应函数中实现）
   .def("transform_to_geolocation", &ToGeolocation, (arg("location")))
    // 获取地图的OpenDRIVE表示（以副本形式返回）
   .def("to_opendrive", CALL_RETURNING_COPY(cc::Map, GetOpenDrive))
    // 将地图保存到磁盘，可指定保存路径（默认空路径可能有默认存储位置等逻辑）
   .def("save_to_disk", &SaveOpenDriveToDisk, (arg("path")=""))
    // 获取地图中所有的人行横道区域列表
   .def("get_crosswalks", CALL_RETURNING_LIST(cc::Map, GetAllCrosswalkZones))
    // 获取地图中所有的地标列表
   .def("get_all_landmarks", CALL_RETURNING_LIST(cc::Map, GetAllLandmarks))
    // 根据OpenDRIVE ID获取地标列表
   .def("get_all_landmarks_from_id", CALL_RETURNING_LIST_1(cc::Map, GetLandmarksFromId, std::string), (args("opendrive_id")))
    // 根据类型获取所有地标列表
   .def("get_all_landmarks_of_type", CALL_RETURNING_LIST_1(cc::Map, GetAllLandmarksOfType, std::string), (args("type")))
    // 根据给定地标获取地标组（具体地标组的概念和逻辑由对应函数确定）
   .def("get_landmark_group", CALL_RETURNING_LIST_1(cc::Map, GetLandmarkGroup, cc::Landmark), args("landmark"))
    // 在内存中处理地图（可能涉及加载、解析等操作，具体由对应函数实现，可指定路径）
   .def("cook_in_memory_map", &cc::Map::CookInMemoryMap, (arg("path")=""))
    // 用于输出类自身相关的字符串表示（具体输出格式由对应函数决定）
   .def(self_ns::str(self_ns::self))
  ;

// ===========================================================================
// -- 辅助对象相关的类定义，用于表示车道标线、路点、路口、地标等不同实体及其属性和操作
// ===========================================================================

// 定义了名为"LaneMarking"的类，用于表示车道标线相关信息
// 提供了获取车道标线类型、颜色、变更类型以及宽度等属性的接口
class_<cre::LaneMarking>("LaneMarking", no_init)
   .add_property("type", &cre::LaneMarking::type)
   .add_property("color", &cre::LaneMarking::color)
   .add_property("lane_change", &cre::LaneMarking::lane_change)
   .add_property("width", &cre::LaneMarking::width)
  ;

// 定义了名为"Waypoint"的类，用于表示路点相关信息，不可复制，使用智能指针管理
// 提供了众多获取路点相关属性（如ID、变换信息、所在车道宽度等）的属性接口
// 以及获取前后路点、获取特定距离内路点、获取相关地标等操作的方法接口
class_<cc::Waypoint, boost::noncopyable, boost::shared_ptr<cc::Waypoint>>("Waypoint", no_init)
   .add_property("id", &cc::Waypoint::GetId)
   .add_property("transform", CALL_RETURNING_COPY(cc::Waypoint, GetTransform))
   .add_property("is_intersection", &cc::Waypoint::IsJunction) // deprecated
   .add_property("is_junction", &cc::Waypoint::IsJunction)
   .add_property("lane_width", &cc::Waypoint::GetLaneWidth)
   .add_property("road_id", &cc::Waypoint::GetRoadId)
   .add_property("section_id", &cc::Waypoint::GetSectionId)
   .add_property("lane_id", &cc::Waypoint::GetLaneId)
   .add_property("s", &cc::Waypoint::GetDistance)
   .add_property("junction_id", &cc::Waypoint::GetJunctionId)
   .add_property("lane_change", &cc::Waypoint::GetLaneChange)
   .add_property("lane_type", &cc::Waypoint::GetType)
   .add_property("right_lane_marking", CALL_RETURNING_OPTIONAL(cc::Waypoint, GetRightLaneMarking))
   .add_property("left_lane_marking", CALL_RETURNING_OPTIONAL(cc::Waypoint, GetLeftLaneMarking))
   .def("next", CALL_RETURNING_LIST_1(cc::Waypoint, GetNext, double), (args("distance")))
   .def("previous", CALL_RETURNING_LIST_1(cc::Waypoint, GetPrevious, double), (args("distance")))
   .def("next_until_lane_end", CALL_RETURNING_LIST_1(cc::Waypoint, GetNextUntilLaneEnd, double), (args("distance")))
   .def("previous_until_lane_start", CALL_RETURNING_LIST_1(cc::Waypoint, GetPreviousUntilLaneStart, double), (args("distance")))
   .def("get_right_lane", &cc::Waypoint::GetRight)
   .def("get_left_lane", &cc::Waypoint::GetLeft)
   .def("get_junction", &cc::Waypoint::GetJunction)
   .def("get_landmarks", CALL_RETURNING_LIST_2(cc::Waypoint, GetAllLandmarksInDistance, double, bool), (arg("distance"), arg("stop_at_junction")=false))
   .def("get_landmarks_of_type", CALL_RETURNING_LIST_3(cc::Waypoint, GetLandmarksOfTypeInDistance, double, std::string, bool), (arg("distance"), arg("type"), arg("stop_at_junction")=false))
   .def(self_ns::str(self_ns::self))
  ;

// 定义了名为"Junction"的类，用于表示路口相关信息，不可复制，使用智能指针管理
// 提供了获取路口ID和边界框属性的接口，以及获取路口相关路点的方法
class_<cc::Junction, boost::noncopyable, boost::shared_ptr<cc::Junction>>("Junction", no_init)
   .add_property("id", &cc::Junction::GetId)
   .add_property("bounding_box", &cc::Junction::GetBoundingBox)
   .def("get_waypoints", &GetJunctionWaypoints)
  ;

// 定义了名为"LandmarkType"（别名为"SignalType"）的类，用于表示地标类型相关的静态属性
// 包含了众多不同意义的地标类型，如危险标识、车道合并标识、各种交通限制标识等
class_<cr::SignalType>("LandmarkType", no_init)
   .add_static_property("Danger", &cr::SignalType::Danger)
   .add_static_property("LanesMerging", &cr::SignalType::LanesMerging)
   .add_static_property("CautionPedestrian", &cr::SignalType::CautionPedestrian)
   .add_static_property("CautionBicycle", &cr::SignalType::CautionBicycle)
   .add_static_property("LevelCrossing", &cr::SignalType::LevelCrossing)
   .add_static_property("StopSign", &cr::SignalType::StopSign)
   .add_static_property("YieldSign", &cr::SignalType::YieldSign)
   .add_static_property("MandatoryTurnDirection", &cr::SignalType::MandatoryTurnDirection)
   .add_static_property("MandatoryLeftRightDirection", &cr::SignalType::MandatoryLeftRightDirection)
   .add_static_property("TwoChoiceTurnDirection", &cr::SignalType::TwoChoiceTurnDirection)
   .add_static_property("Roundabout", &cr::SignalType::Roundabout)
   .add_static_property("PassRightLeft", &cr::SignalType::PassRightLeft)
   .add_static_property("AccessForbidden", &cr::SignalType::AccessForbidden)
   .add_static_property("AccessForbiddenMotorvehicles", &cr::SignalType::AccessForbiddenMotorvehicles)
   .add_static_property("AccessForbiddenTrucks", &cr::SignalType::AccessForbiddenTrucks)
   .add_static_property("AccessForbiddenBicycle", &cr::SignalType::AccessForbiddenBicycle)
   .add_static_property("AccessForbiddenWeight", &cr::SignalType::AccessForbiddenWeight)
   .add_static_property("AccessForbiddenWidth", &cr::SignalType::AccessForbiddenWidth)
   .add_static_property("AccessForbiddenHeight", &cr::SignalType::AccessForbiddenHeight)
   .add_static_property("AccessForbiddenWrongDirection", &cr::SignalType::AccessForbiddenWrongDirection)
   .add_static_property("ForbiddenUTurn", &cr::SignalType::ForbiddenUTurn)
   .add_static_property("MaximumSpeed", &cr::SignalType::MaximumSpeed)
   .add_static_property("ForbiddenOvertakingMotorvehicles", &cr::SignalType::ForbiddenOvertakingMotorvehicles)
   .add_static_property("ForbiddenOvertakingTrucks", &cr::SignalType::ForbiddenOvertakingTrucks)
   .add_static_property("AbsoluteNoStop", &cr::SignalType::AbsoluteNoStop)
   .add_static_property("RestrictedStop", &cr::SignalType::RestrictedStop)
   .add_static_property("HasWayNextIntersection", &cr::SignalType::HasWayNextIntersection)
   .add_static_property("PriorityWay", &cr::SignalType::PriorityWay)
   .add_static_property("PriorityWayEnd", &cr::SignalType::PriorityWayEnd)
   .add_static_property("CityBegin", &cr::SignalType::CityBegin)
   .add_static_property("CityEnd", &cr::SignalType::CityEnd)
   .add_static_property("Highway", &cr::SignalType::Highway)
   .add_static_property("DeadEnd", &cr::SignalType::DeadEnd)
   .add_static_property("RecomendedSpeed", &cr::SignalType::RecomendedSpeed)
   .add_static_property("RecomendedSpeedEnd", &cr::SignalType::RecomendedSpeedEnd)
  ;
