// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/client/Map.h"

#include "carla/client/Junction.h"
#include "carla/client/Waypoint.h"
#include "carla/opendrive/OpenDriveParser.h"
#include "carla/road/Map.h"
#include "carla/road/RoadTypes.h"
#include "carla/trafficmanager/InMemoryMap.h"

#include <sstream>
// 命名空间 carla
namespace carla {
// 命名空间 client
namespace client {
// 静态函数 MakeMap，根据输入的 opendrive 内容生成地图  
static auto MakeMap(const std::string &opendrive_contents) {
// 创建输入字符串流   
 auto stream = std::istringstream(opendrive_contents);
 // 调用 OpenDriveParser 类的 Load 函数加载地图，返回 boost::optional<carla::road::Map>
    auto map = opendrive::OpenDriveParser::Load(stream.str());
 // 如果 map 为空，抛出运行时异常    
    if (!map.has_value()) {
      throw_exception(std::runtime_error("failed to generate map"));
    }
// 移动 map 的值
    return std::move(*map);
  }
 // Map 类的构造函数，接受 rpc::MapInfo 和 xodr 内容
  Map::Map(rpc::MapInfo description, std::string xodr_content)
    : _description(std::move(description)),
      _map(MakeMap(xodr_content)){
// 存储 xodr 内容
    open_drive_file = xodr_content;
  }
// 另一个 Map 类的构造函数，接受名称和 xodr 内容
  Map::Map(std::string name, std::string xodr_content)
    : Map(rpc::MapInfo{
    std::move(name),
    std::vector<geom::Transform>{}}, xodr_content) {
    open_drive_file = xodr_content;
  }
// Map 类的析构函数，使用默认析构函数
  Map::~Map() = default;
// 获取与给定位置相关的 Waypoint 的函数
  SharedPtr<Waypoint> Map::GetWaypoint(
  const geom::Location &location,
  bool project_to_road,
  int32_t lane_type) const {
// 定义一个可选的 road::element::Waypoint 变量
    boost::optional<road::element::Waypoint> waypoint;
// 根据是否投影到道路选择不同的获取方式
    if (project_to_road) {
      waypoint = _map.GetClosestWaypointOnRoad(location, lane_type);
    } else {
      waypoint = _map.GetWaypoint(location, lane_type);
    }
 // 如果存在 waypoint，创建一个新的 Waypoint 并返回，否则返回 nullptr
    return waypoint.has_value() ?
    SharedPtr<Waypoint>(new Waypoint{shared_from_this(), *waypoint}) :
    nullptr;
  }
// 根据道路 ID、车道 ID 和 s 坐标获取 Waypoint 的函数
  SharedPtr<Waypoint> Map::GetWaypointXODR(
      carla::road::RoadId road_id,
      carla::road::LaneId lane_id,
      float s) const {
 // 定义一个可选的 road::element::Waypoint 变量
    boost::optional<road::element::Waypoint> waypoint;
// 调用 _map 的 GetWaypoint 函数获取 waypoint
    waypoint = _map.GetWaypoint(road_id, lane_id, s);
 // 如果存在 waypoint，创建一个新的 Waypoint 并返回，否则返回 nullptr
    return waypoint.has_value() ?
        SharedPtr<Waypoint>(new Waypoint{shared_from_this(), *waypoint}) :
        nullptr;
  }
// 获取地图拓扑结构的函数
  Map::TopologyList Map::GetTopology() const {
// 为简洁使用 re 作为 carla::road::element 的别名
    namespace re = carla::road::element;
// 为简洁使用 re 作为 carla::road::element 的别名
    std::unordered_map<re::Waypoint, SharedPtr<Waypoint>> waypoints;
 // 定义一个 lambda 函数，用于获取或创建 Waypoint
    auto get_or_make_waypoint = [&](const auto &waypoint) {
      auto it = waypoints.find(waypoint);
      if (it == waypoints.end()) {
        it = waypoints.emplace(
            waypoint,
            SharedPtr<Waypoint>(new Waypoint{shared_from_this(), waypoint})).first;
      }
      return it->second;
    };
 // 存储拓扑结构的列表
    TopologyList result;
 // 生成拓扑结构
    auto topology = _map.GenerateTopology();
 // 为结果预留空间
    result.reserve(topology.size());
 // 遍历拓扑结构中的元素，创建 Waypoint 并添加到结果中
    for (const auto &pair : topology) {
      result.emplace_back(
      get_or_make_waypoint(pair.first),
      get_or_make_waypoint(pair.second));
    }
    return result;
  }
// 生成间隔一定距离的 Waypoint 列表的函数
  std::vector<SharedPtr<Waypoint>> Map::GenerateWaypoints(double distance) const {
 // 存储结果的 Waypoint 向量
    std::vector<SharedPtr<Waypoint>> result;
// 生成 Waypoint 列表
    const auto waypoints = _map.GenerateWaypoints(distance);
// 为结果预留空间
    result.reserve(waypoints.size());
 // 遍历生成的 Waypoint，创建并添加到结果中
    for (const auto &waypoint : waypoints) {
      result.emplace_back(SharedPtr<Waypoint>(new Waypoint{shared_from_this(), waypoint}));
    }
    return result;
  }
// 计算穿越车道的函数
  std::vector<road::element::LaneMarking> Map::CalculateCrossedLanes(
  const geom::Location &origin,
  const geom::Location &destination) const {
 // 调用 _map 的 CalculateCrossedLanes 函数
    return _map.CalculateCrossedLanes(origin, destination);
  }
 // 获取地理参考的函数
  const geom::GeoLocation &Map::GetGeoReference() const {
// 调用 _map 的 GetGeoReference 函数
    return _map.GetGeoReference();
  }
 // 获取所有人行横道区域的函数
  std::vector<geom::Location> Map::GetAllCrosswalkZones() const {
// 调用 _map 的 GetAllCrosswalkZones 函数
    return _map.GetAllCrosswalkZones();
  }
// 获取与给定 Waypoint 相关的 Junction 的函数
  SharedPtr<Junction> Map::GetJunction(const Waypoint &waypoint) const {
// 获取 Junction 指针
    const road::Junction *juncptr = GetMap().GetJunction(waypoint.GetJunctionId());
// 创建新的 Junction 并返回
    auto junction = SharedPtr<Junction>(new Junction(shared_from_this(), juncptr));
    return junction;
  }
 // 获取 Junction 的 Waypoint 对的函数
  std::vector<std::pair<SharedPtr<Waypoint>, SharedPtr<Waypoint>>> Map::GetJunctionWaypoints(
      road::JuncId id,
      road::Lane::LaneType lane_type) const {
 // 存储结果的向量
    std::vector<std::pair<SharedPtr<Waypoint>, SharedPtr<Waypoint>>> result;
 // 获取 Junction 的 Waypoint 对
    auto junction_waypoints = GetMap().GetJunctionWaypoints(id, lane_type);
 // 遍历并创建新的 Waypoint 对添加到结果中
    for (auto &waypoint_pair : junction_waypoints) {
      result.emplace_back(
      std::make_pair(SharedPtr<Waypoint>(new Waypoint(shared_from_this(), waypoint_pair.first)),
      SharedPtr<Waypoint>(new Waypoint(shared_from_this(), waypoint_pair.second))));
    }
    return result;
  }
 // 获取所有地标的函数
  std::vector<SharedPtr<Landmark>> Map::GetAllLandmarks() const {
 // 存储结果的向量
    std::vector<SharedPtr<Landmark>> result;
 // 获取所有信号引用
    auto signal_references = _map.GetAllSignalReferences();
// 遍历信号引用，创建新的 Landmark 并添加到结果中
    for(auto* signal_reference : signal_references) {
      result.emplace_back(
          new Landmark(nullptr, shared_from_this(), signal_reference, 0));
    }
    return result;
  }
// 从 ID 获取地标的函数
  std::vector<SharedPtr<Landmark>> Map::GetLandmarksFromId(std::string id) const {
// 存储结果的向量
    std::vector<SharedPtr<Landmark>> result;
 // 获取所有信号引用
    auto signal_references = _map.GetAllSignalReferences();
// 遍历信号引用，找到符合 ID 的创建新的 Landmark 并添加到结果中
    for(auto* signal_reference : signal_references) {
      if(signal_reference->GetSignalId() == id) {
        result.emplace_back(
            new Landmark(nullptr, shared_from_this(), signal_reference, 0));
      }
    }
    return result;
  }
 // 从类型获取地标的函数
  std::vector<SharedPtr<Landmark>> Map::GetAllLandmarksOfType(std::string type) const {
 // 存储结果的向量
    std::vector<SharedPtr<Landmark>> result;
 // 获取所有信号引用
    auto signal_references = _map.GetAllSignalReferences();
 // 遍历信号引用，找到符合类型的创建新的 Landmark 并添加到结果中
    for(auto* signal_reference : signal_references) {
      if(signal_reference->GetSignal()->GetType() == type) {
        result.emplace_back(
            new Landmark(nullptr, shared_from_this(), signal_reference, 0));
      }
    }
    return result;
  }
// 获取地标组的函数
  std::vector<SharedPtr<Landmark>>
      Map::GetLandmarkGroup(const Landmark &landmark) const {
 // 存储结果的向量 
    std::vector<SharedPtr<Landmark>> result;
 // 获取信号的控制器
    auto &controllers = landmark._signal->GetSignal()->GetControllers();
// 遍历控制器和控制器中的信号，添加新的 Landmark 到结果中
    for (auto& controller_id : controllers) {
      const auto &controller = _map.GetControllers().at(controller_id);
      for(auto& signal_id : controller->GetSignals()) {
        auto& signal = _map.GetSignals().at(signal_id);
        auto new_landmarks = GetLandmarksFromId(signal->GetSignalId());
        result.insert(result.end(), new_landmarks.begin(), new_landmarks.end());
      }
    }
    return result;
  }
 // 烹饪内存中地图的函数
  void Map::CookInMemoryMap(const std::string& path) const {
// 调用 InMemoryMap 的 Cook 函数
    traffic_manager::InMemoryMap::Cook(shared_from_this(), path);
  }

}  // 结束 client 命名空间
} // 结束 carla 命名空间
