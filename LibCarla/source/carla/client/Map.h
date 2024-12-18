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
// 包含CARLA地图（Map）的头文件
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
    /**
         * @brief 获取推荐生成点。
         *
         * @return 返回推荐生成点（`geom::Transform`对象的向量）的常量引用。
         */
    const std::vector<geom::Transform> &GetRecommendedSpawnPoints() const {  
      return _description.recommended_spawn_points;
    }
    /**
         * @brief 获取路点，可以选择是否投影到道路上。
         *
         * @param location 地理位置。
         * @param project_to_road 是否将位置投影到最近的道路上（默认为true）。
         * @param lane_type 车道类型（默认为驾驶车道）。
         * @return 返回指向路点对象的智能指针。
         */
    SharedPtr<Waypoint> GetWaypoint(  
        const geom::Location &location,
        bool project_to_road = true,
        int32_t lane_type = static_cast<uint32_t>(road::Lane::LaneType::Driving)) const;
    /**
         * @brief 根据OpenDRIVE ID获取路点。
         *
         * @param road_id 道路ID。
         * @param lane_id 车道ID。
         * @param s 沿车道的距离。
         * @return 返回指向路点对象的智能指针。
         */
    SharedPtr<Waypoint> GetWaypointXODR(  
      carla::road::RoadId road_id,
      carla::road::LaneId lane_id,
      float s) const;
    /**
         * @brief 拓扑结构列表的类型定义。
         */
    using TopologyList = std::vector<std::pair<SharedPtr<Waypoint>, SharedPtr<Waypoint>>>;
    /**
        * @brief 获取地图的拓扑结构。
        *
        * @return 返回拓扑结构列表。
        */
    TopologyList GetTopology() const;
    /**
         * @brief 根据距离生成路点。
         *
         * @param distance 距离。
         * @return 返回生成的路点（智能指针的向量）。
         */
    std::vector<SharedPtr<Waypoint>> GenerateWaypoints(double distance) const;   
    /**
         * @brief 计算从起点到终点所跨越的车道。
         *
         * @param origin 起点位置。
         * @param destination 终点位置。
         * @return 返回跨越的车道标记的向量。
         */
    std::vector<road::element::LaneMarking> CalculateCrossedLanes(  
        const geom::Location &origin,
        const geom::Location &destination) const;
    /**
         * @brief 获取地理参考。
         *
         * @return 返回地理参考的常量引用。
         */
    const geom::GeoLocation &GetGeoReference() const;  
    /**
         * @brief 获取所有斑马线区域。
         *
         * @return 返回斑马线区域位置（`geom::Location`对象的向量）的常量引用。
         */
    std::vector<geom::Location> GetAllCrosswalkZones() const;   
    /**
         * @brief 获取指定路点所属的路口对象。
         *
         * @param waypoint 路点对象。
         * @return 返回指向路口对象的智能指针。
         */
    SharedPtr<Junction> GetJunction(const Waypoint &waypoint) const;  

    /**
         * @brief 返回路口中每条车道的起始和结束路点。
         *
         * @param id 路口ID。
         * @param type 车道类型。
         * @return 返回路口车道路点的对（智能指针）的向量。
         */
    std::vector<std::pair<SharedPtr<Waypoint>, SharedPtr<Waypoint>>> GetJunctionWaypoints(
        road::JuncId id, road::Lane::LaneType type) const;

    /**
         * @brief 获取地图中所有的地标。
         *
         * @return 返回所有地标（智能指针的向量）。
         */
    std::vector<SharedPtr<Landmark>> GetAllLandmarks() const;

    /**
         * @brief 根据特定的OpenDRIVE ID获取地图中的所有地标。
         *
         * @param id OpenDRIVE ID。
         * @return 返回与ID匹配的地标（智能指针的向量）。
         */
    std::vector<SharedPtr<Landmark>> GetLandmarksFromId(std::string id) const;

    /**
          * @brief 根据特定类型获取地图中的所有地标。
          *
          * @param type 地标类型。
          * @return 返回与类型匹配的地标（智能指针的向量）。
          */
    std::vector<SharedPtr<Landmark>> GetAllLandmarksOfType(std::string type) const;

    /**
          * @brief 获取与指定地标在同一组的所有地标。
          *
          * @param landmark 指定地标对象。
          * @return 返回与指定地标在同一组的地标（智能指针的向量）。
          */
    std::vector<SharedPtr<Landmark>> GetLandmarkGroup(const Landmark &landmark) const;

    /**
          * @brief 制作交通管理器使用的内存地图。
          *
          * @param path 地图存储路径。
          */
    void CookInMemoryMap(const std::string& path) const;

  private:

    std::string open_drive_file;// 包含OpenDRIVE文件内容的字符串

    const rpc::MapInfo _description; // 描述地图信息的RPC对象

    const road::Map _map; // 道路地图的内部表示
  };

} // namespace client
} // namespace carla
