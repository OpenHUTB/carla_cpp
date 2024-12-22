// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/AtomicList.h"
#include "carla/client/detail/EpisodeState.h"
#include "carla/geom/BoundingBox.h"

// 使用几何库相关功能
#include "carla/geom/Location.h"
#include "carla/geom/Transform.h"
#include "carla/nav/WalkerManager.h" 

// 使用远程过程调用相关功能
#include "carla/rpc/ActorId.h"

#include <recast/Recast.h>
// 包含Recast/Detour导航网格生成和查询库的头文件
 
// Recast库的主头文件，提供生成导航网格所需的基本函数和数据结构
#include <recast/DetourCrowd.h>
// 提供处理大量角色（crowd）同时导航的功能，确保它们不会相互碰撞或阻塞
#include <recast/DetourNavMesh.h>
// 定义导航网格（NavMesh）的数据结构和接口，表示环境中的可通行区域
#include <recast/DetourNavMeshBuilder.h>
// 包含构建导航网格的实用函数，从高度图或紧凑高度图构建导航网格
#include <recast/DetourNavMeshQuery.h>
// 提供查询导航网格的接口，如查找路径、获取多边形信息等
#include <recast/DetourCommon.h>
// 可能包含Recast/Detour库中使用的通用定义、枚举和数据结构

namespace carla {
// 定义命名空间carla，它是CARLA自动驾驶仿真平台的命名空间
namespace nav {
// 在carla命名空间内定义子命名空间nav，用于与导航相关的功能
// 在这里，您可能会定义与导航网格生成和查询相关的函数、类或数据结构
// 例如，您可能会实现一个函数来加载高度图并生成导航网格
// 或者实现一个类来管理导航网格的查询和路径规划

  // 导航区域
  // 参考：https://openhutb.github.io/carla_doc/tuto_M_generate_pedestrian_navigation/
  enum NavAreas {
    CARLA_AREA_BLOCK = 0,
    CARLA_AREA_SIDEWALK,  // 人行道
    CARLA_AREA_CROSSWALK, // 人行横道：如果找不到地面，行人将在这些网格上行走作为第二种选择。
    CARLA_AREA_ROAD,      // 马路：行人只能通过这些网格过马路。
    CARLA_AREA_GRASS      // 草地：行人不会在此网格上行走，除非您指定一定比例的行人这样做。
  };

  enum SamplePolyFlags
// 定义一个枚举类型SamplePolyFlags，用于表示导航网格中多边形（Poly）的不同类型或属性
  {
    CARLA_TYPE_NONE       = 0x01,
 // 没有任何特殊类型的标志，可能表示一个默认或未知的类型
    CARLA_TYPE_SIDEWALK   = 0x02,
// 表示人行道，通常用于行人行走的区域
    CARLA_TYPE_CROSSWALK  = 0x04,
// 表示斑马线，是行人过马路时专用的区域
    CARLA_TYPE_ROAD       = 0x08,
// 表示道路，通常用于车辆行驶
    CARLA_TYPE_GRASS      = 0x10,
// 表示草地，可能允许行人行走，但通常不允许车辆进入
    CARLA_TYPE_ALL        = 0xffff,
// 一个特殊的标志，表示上述所有类型的组合，常用于需要匹配所有类型的情况
// 0xffff即二进制的1111 1111 1111 1111，代表所有位都被设置为1

    CARLA_TYPE_WALKABLE   = CARLA_TYPE_SIDEWALK | CARLA_TYPE_CROSSWALK | CARLA_TYPE_GRASS | CARLA_TYPE_ROAD,
 // 一个方便的组合标志，表示所有可通行的类型，包括人行道、斑马线、草地和道路
 // 但不包括没有任何特殊类型的标志（CARLA_TYPE_NONE）
  };

  /// 向人群发送有关车辆的信息的结构体
  struct VehicleCollisionInfo {
    carla::rpc::ActorId id;
    carla::geom::Transform transform;
    carla::geom::BoundingBox bounding;
  };

  /// 管理行人导航，使用 Recast & Detour 库进行低层计算。
  ///
  /// 该类从服务器获取地图的二进制内容，这是查找路径所必需的。然后，这个类可以添加或删除行人，并为每个行人设置目标步行点。
  class Navigation : private NonCopyable {

  public:

    Navigation();
    ~Navigation();

    /// 从磁盘中加载导航数据
    bool Load(const std::string &filename);
    /// 从内存中加载导航数据
    bool Load(std::vector<uint8_t> content);
    /// 返回从一个位置到另一个位置的路径点
    bool GetPath(carla::geom::Location from, carla::geom::Location to, dtQueryFilter * filter,
// GetPath 函数注释
//
// 功能：在给定的起点和终点之间查询导航路径。
//
// 参数：
//   - from：起点位置，使用carla::geom::Location类型表示。
//   - to：终点位置，使用carla::geom::Location类型表示。
//   - filter：一个指向dtQueryFilter的指针，用于在查询过程中筛选导航网格中的多边形。
//             dtQueryFilter是Detour库中的一个结构体，用于定义查询的过滤条件。
//   - path：一个引用，指向存储查询结果的路径的std::vector<carla::geom::Location>。
//           查询成功后，该向量将包含从起点到终点的位置序列。
//   - area：一个引用，指向存储路径中每个位置所属区域的std::vector<unsigned char>。
//           每个元素代表路径中对应位置的多边形区域类型。
//
// 返回值：
//   - 如果成功找到路径，则返回true；否则返回false
    std::vector<carla::geom::Location> &path, std::vector<unsigned char> &area);
    bool GetAgentRoute(ActorId id, carla::geom::Location from, carla::geom::Location to,
 
// GetAgentRoute 函数注释
//
// 功能：为给定的代理（Agent，如自动驾驶车辆或行人）查询从起点到终点的路由。
//
// 参数：
//   - id：代理的唯一标识符，使用ActorId类型表示。在CARLA中，每个仿真对象（如车辆、行人等）都有一个唯一的ID。
//   - from：起点位置，使用carla::geom::Location类型表示。
//   - to：终点位置，使用carla::geom::Location类型表示。
//   - path：一个引用，指向存储查询结果的路径的std::vector<carla::geom::Location>。
//           查询成功后，该向量将包含从起点到终点的位置序列，这些位置是根据代理的类型和规则优化过的。
//   - area：一个引用，指向存储路径中每个位置所属区域的std::vector<unsigned char>。
//           每个元素代表路径中对应位置的多边形区域类型，这些信息可能用于进一步的决策或行为规划。
//
// 返回值：
//   - 如果成功找到路由，则返回true；否则返回false。
//
// 注意：
//   - 与GetPath函数相比，GetAgentRoute函数可能考虑了更多的因素，如代理的类型、尺寸、速度限制等，以生成更适合代理的路由。
//   - 在实际使用中，这些函数可能会依赖于CARLA仿真平台中的导航系统和地图数据来执行查询。
    std::vector<carla::geom::Location> &path, std::vector<unsigned char> &area);

    /// 引用模拟器来访问API函数
    void SetSimulator(std::weak_ptr<carla::client::detail::Simulator> simulator);
    /// 设置随机数种子
    void SetSeed(unsigned int seed);
    /// 创建人群对象
    void CreateCrowd(void);
    /// 创建新的行人
    bool AddWalker(ActorId id, carla::geom::Location from);
    /// 在人群中创造一辆新的车辆，让行人避开
    bool AddOrUpdateVehicle(VehicleCollisionInfo &vehicle);
    /// 移除代理
    bool RemoveAgent(ActorId id);
    /// 在人群中添加/更新/删除车辆
    bool UpdateVehicles(std::vector<VehicleCollisionInfo> vehicles);
    /// 设置新的最大速度
    bool SetWalkerMaxSpeed(ActorId id, float max_speed);
    /// 设置新的目标点以通过有事件的路线
    bool SetWalkerTarget(ActorId id, carla::geom::Location to);
    // 设置新的目标点，直接前往没有事件发生的地方
    bool SetWalkerDirectTarget(ActorId id, carla::geom::Location to);
    bool SetWalkerDirectTargetIndex(int index, carla::geom::Location to);
    /// 获取步行人当前变换
    bool GetWalkerTransform(ActorId id, carla::geom::Transform &trans);
    /// 获取行人的当前位置
    bool GetWalkerPosition(ActorId id, carla::geom::Location &location);
    /// 获取步行人速度
    float GetWalkerSpeed(ActorId id);
    /// 更新人群中的所有步行者
    void UpdateCrowd(const client::detail::EpisodeState &state);
    /// 获取导航的随机位置
    bool GetRandomLocation(carla::geom::Location &location, dtQueryFilter * filter = nullptr) const;
    /// 设置行人代理在路径跟随过程中穿过马路的概率
    void SetPedestriansCrossFactor(float percentage);
    /// 将人群中的代理设置为暂停
    void PauseAgent(ActorId id, bool pause);
    /// 如果代理在附近有车辆（作为邻居），则返回
    bool HasVehicleNear(ActorId id, float distance, carla::geom::Location direction);
    /// 让代理查看某个位置
    bool SetWalkerLookAt(ActorId id, carla::geom::Location location);
    /// 如果行人代理被车辆撞死，则返回
    bool IsWalkerAlive(ActorId id, bool &alive);

    dtCrowd *GetCrowd() { return _crowd; };

    /// 返回最后增量秒数
    double GetDeltaSeconds() { return _delta_seconds; };

  private:

    bool _ready { false };
    std::vector<uint8_t> _binary_mesh;
    double _delta_seconds { 0.0 };
    /// 网格
    dtNavMesh *_nav_mesh { nullptr };
    dtNavMeshQuery *_nav_query { nullptr };
    /// crowd
    dtCrowd *_crowd { nullptr };
    /// mapping Id
    std::unordered_map<ActorId, int> _mapped_walkers_id;
    std::unordered_map<ActorId, int> _mapped_vehicles_id;
    // 也可以通过索引进行映射
    std::unordered_map<int, ActorId> _mapped_by_index;
    /// 存储上一个节拍的行人偏航角
    std::unordered_map<ActorId, float> _yaw_walkers;
    /// 每隔一段时间保存每个参与者的位置，并检查是否有参与者被阻挡
    std::unordered_map<int, carla::geom::Vector3D> _walkers_blocked_position;
    double _time_to_unblock { 0.0 };

    /// 行人管理器负责带事件的路线规划
    WalkerManager _walker_manager;

    std::weak_ptr<carla::client::detail::Simulator> _simulator;
    
    mutable std::mutex _mutex;

    float _probability_crossing { 0.0f };

    /// 为代理分配过滤索引
    void SetAgentFilter(int agent_index, int filter_index);
  };

} // namespace nav
} // namespace carla
