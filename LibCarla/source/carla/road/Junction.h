// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once  // 确保该头文件只被包含一次

#include "carla/geom/BoundingBox.h"  // 引入BoundingBox类的定义
#include "carla/NonCopyable.h"  // 引入不可拷贝类的定义
#include "carla/road/RoadTypes.h"  // 引入道路相关类型的定义

#include <unordered_map>  // 引入无序映射的头文件
#include <unordered_set>  // 引入无序集合的头文件
#include <vector>  // 引入动态数组的头文件
#include <string>  // 引入字符串的头文件

namespace carla {  // 定义carla命名空间
namespace road {  // 定义road命名空间

  class MapBuilder;  // 前向声明MapBuilder类

  // 定义Junction类，表示交叉口
  class Junction : private MovableNonCopyable {
  public:

    // 定义LaneLink结构体，表示车道之间的连接
    struct LaneLink {
      LaneId from;  // 源车道ID
      LaneId to;    // 目标车道ID
    };

    // 定义Connection结构体，表示道路连接信息
    struct Connection {
      ConId id;  // 连接ID
      RoadId incoming_road;  // 进入道路ID
      RoadId connecting_road;  // 连接道路ID
      std::vector<LaneLink> lane_links;  // 车道链接的列表

      // 添加车道链接
      void AddLaneLink(LaneId from, LaneId to) {
        lane_links.push_back(LaneLink{from, to});  // 将车道链接添加到列表中
      }

      // Connection构造函数
      Connection(ConId id, RoadId incoming_road, RoadId connecting_road)
        : id(id),
          incoming_road(incoming_road),
          connecting_road(connecting_road),
          lane_links() {}  // 初始化连接信息
    };

    // Junction构造函数
    Junction(const JuncId id, const std::string name)
      : _id(id),
        _name(name),
        _connections() {}  // 初始化交叉口ID、名称和连接列表

    // 获取交叉口ID
    JuncId GetId() const {
      return _id;  // 返回交叉口ID
    }

    // 获取指定ID的连接
    Connection *GetConnection(ConId id) {
      const auto search = _connections.find(id);  // 在连接中查找指定ID
      if (search != _connections.end()) {  // 如果找到
        return &search->second;  // 返回对应的连接
      }
      return nullptr;  // 未找到，返回空指针
    }

    // 获取所有连接的引用
    std::unordered_map<ConId, Connection> &GetConnections() {
      return _connections;  // 返回连接的引用
    }

    // 获取所有连接的副本
    std::unordered_map<ConId, Connection> GetConnections() const {
      return _connections;  // 返回连接的副本
    }

    // 获取边界框
    carla::geom::BoundingBox GetBoundingBox() const {
      return _bounding_box;  // 返回交叉口的边界框
    }

    // 检查指定道路是否有冲突
    bool RoadHasConflicts(RoadId road_id) const {
      return _road_conflicts.count(road_id) > 0;  // 返回是否有冲突
    }

    // 获取指定道路的冲突列表
    const std::unordered_set<RoadId> & GetConflictsOfRoad(RoadId road_id) const {
      return _road_conflicts.at(road_id);  // 返回指定道路的冲突道路集
    }

    // 获取所有控制器的集合
    const std::set<ContId>& GetControllers() const {
      return _controllers;  // 返回控制器的集合
    }

  private:

    friend MapBuilder;  // 声明MapBuilder为友元类，可以访问私有成员

    JuncId _id;  // 交叉口ID

    std::string _name;  // 交叉口名称

    std::unordered_map<ConId, Connection> _connections;  // 存储连接的无序映射

    std::set<ContId> _controllers;  // 控制器的集合

    std::unordered_map<RoadId, std::unordered_set<RoadId>> _road_conflicts;  // 道路冲突的映射

    carla::geom::BoundingBox _bounding_box;  // 交叉口的边界框
  };

} // namespace road
} // namespace carla