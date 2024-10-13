// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once // 确保头文件只被包含一次，防止重复定义。

#include "carla/Memory.h"
#include "carla/NonCopyable.h"
#include "carla/road/element/LaneMarking.h"
#include "carla/road/Lane.h"
#include "carla/road/Map.h"
#include "carla/road/RoadTypes.h"
#include "carla/rpc/MapInfo.h"
#include "Landmark.h"

#include <string>

namespace carla {
namespace geom { class GeoLocation; } // 定义几何位置类
namespace client {

  class Waypoint; // 声明路点类
  class Junction; // 声明交叉口类

  class Map
    : public EnableSharedFromThis<Map>, 
      private NonCopyable {
  public:

    explicit Map(rpc::MapInfo description, std::string xodr_content); // 构造函数，从RPC地图信息和OpenDRIVE内容创建地图


    explicit Map(std::string name, std::string xodr_content);  // 构造函数，从名称和OpenDRIVE内容创建地图

    ~Map();

    const std::string &GetName() const { // 获取地图名称
      return _description.name;
    }

    const road::Map &GetMap() const { // 获取道路地图
      return _map;
    }

    const std::string &GetOpenDrive() const { // 获取OpenDRIVE文件内容
      return open_drive_file;
    }

    const std::vector<geom::Transform> &GetRecommendedSpawnPoints() const { // 获取推荐生成点
      return _description.recommended_spawn_points;
    }

    SharedPtr<Waypoint> GetWaypoint( // 获取路点，可以选择是否投影到道路上
        const geom::Location &location,
        bool project_to_road = true,
        int32_t lane_type = static_cast<uint32_t>(road::Lane::LaneType::Driving)) const;

    SharedPtr<Waypoint> GetWaypointXODR( // 根据OpenDRIVE ID获取路点
      carla::road::RoadId road_id,
      carla::road::LaneId lane_id,
      float s) const;

    using TopologyList = std::vector<std::pair<SharedPtr<Waypoint>, SharedPtr<Waypoint>>>;

    TopologyList GetTopology() const;

    std::vector<SharedPtr<Waypoint>> GenerateWaypoints(double distance) const;  // 根据距离生成路点

    std::vector<road::element::LaneMarking> CalculateCrossedLanes( // 计算交叉车道
        const geom::Location &origin,
        const geom::Location &destination) const;

    const geom::GeoLocation &GetGeoReference() const;  // 获取地理参考

    std::vector<geom::Location> GetAllCrosswalkZones() const;  // 获取所有斑马线区域

    SharedPtr<Junction> GetJunction(const Waypoint &waypoint) const; // 获取指定路点所属的路口对象

    /// 返回路口中每条车道的起始和结束路点
    /// 
    std::vector<std::pair<SharedPtr<Waypoint>, SharedPtr<Waypoint>>> GetJunctionWaypoints(
        road::JuncId id, road::Lane::LaneType type) const;

    /// 获取地图中所有的地标
    std::vector<SharedPtr<Landmark>> GetAllLandmarks() const;

    ///根据特定的OpenDRIVE ID获取地图中的所有地标
    std::vector<SharedPtr<Landmark>> GetLandmarksFromId(std::string id) const;

    /// 根据特定类型获取地图中的所有地标
    std::vector<SharedPtr<Landmark>> GetAllLandmarksOfType(std::string type) const;

    ///获取与指定地标在同一组的所有地标
    std::vector<SharedPtr<Landmark>> GetLandmarkGroup(const Landmark &landmark) const;

    /// 制作交通管理器使用的内存地图
    void CookInMemoryMap(const std::string& path) const;

  private:

    std::string open_drive_file;

    const rpc::MapInfo _description; // 描述地图信息的RPC对象

    const road::Map _map; // 道路地图的内部表示
  };

} // namespace client
} // namespace carla
