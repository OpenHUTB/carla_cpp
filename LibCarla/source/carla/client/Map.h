// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once // 确保头文件只被包含一次，防止重复定义。
/// @cond DoxygenSuppress

// 包含CARLA内存管理相关的头文件
#include "carla/Memory.h"
// 包含CARLA不可复制类的头文件，用于防止对象被复制
#include "carla/NonCopyable.h"
// 包含CARLA道路元素中的车道标记（LaneMarking）的头文件
#include "carla/road/element/LaneMarking.h"
// 包含CARLA道路（Lane）的头文件
#include "carla/road/Lane.h"
、// 包含CARLA地图（Map）的头文件
#include "carla/road/Map.h"
// 包含CARLA道路类型（RoadTypes）的头文件
#include "carla/road/RoadTypes.h"
// 包含CARLA RPC地图信息（MapInfo）的头文件
#include "carla/rpc/MapInfo.h"
// 包含地标（Landmark）的头文件
#include "Landmark.h"

#include <string>
/**
 * @namespace carla::client
 * @brief CARLA仿真框架中的客户端命名空间，包含与地图交互的类。
 */
namespace carla {
namespace geom { 
    /**
     * @class GeoLocation
     * @brief 定义几何位置类，用于表示地图上的地理位置。
     */class GeoLocation; } 
namespace client {
    /**
         * @class Waypoint
         * @brief 声明路点类，表示地图上的一个具体点，通常与道路网络中的某个位置相关联。
         */
  class Waypoint; 
  /**
     * @class Junction
     * @brief 声明交叉口类，表示地图上的一个交叉口或道路交汇点。
     */
  class Junction;  
  /**
     * @class Map
     * @brief 地图类，用于表示和操作CARLA仿真中的地图。
     *
     * 该类提供了丰富的接口来查询地图信息、处理路点和地标、以及获取地图的拓扑结构等。
     * 它继承自EnableSharedFromThis以支持智能指针的共享，并私有继承了NonCopyable以防止对象被复制。
     */
  class Map
    : public EnableSharedFromThis<Map>, 
      private NonCopyable {
  public:
      /**
               * @brief 构造函数，从RPC地图信息和OpenDRIVE内容创建地图。
               *
               * @param description 描述地图信息的RPC对象。
               * @param xodr_content 包含OpenDRIVE地图数据的字符串。
               */
    explicit Map(rpc::MapInfo description, std::string xodr_content);   
    /**
         * @brief 构造函数，从名称和OpenDRIVE内容创建地图。
         *
         * @param name 地图的名称。
         * @param xodr_content 包含OpenDRIVE地图数据的字符串。
         */
    explicit Map(std::string name, std::string xodr_content);   
    /**
         * @brief 析构函数。
         */
    ~Map();
    /**
         * @brief 获取地图名称。
         *
         * @return 返回地图名称的常量引用。
         */
    const std::string &GetName() const { 
      return _description.name;
    }
    /**
         * @brief 获取道路地图。
         *
         * @return 返回道路地图的常量引用。
         */
    const road::Map &GetMap() const {  
      return _map;
    }
    /**
         * @brief 获取OpenDRIVE文件内容。
         *
         * @return 返回包含OpenDRIVE文件内容的字符串的常量引用。
         */
    const std::string &GetOpenDrive() const {  
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
