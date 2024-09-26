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
#include <recast/DetourCrowd.h>
#include <recast/DetourNavMesh.h>
#include <recast/DetourNavMeshBuilder.h>
#include <recast/DetourNavMeshQuery.h>
#include <recast/DetourCommon.h>

namespace carla {
namespace nav {

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
  {
    CARLA_TYPE_NONE       = 0x01,
    CARLA_TYPE_SIDEWALK   = 0x02,
    CARLA_TYPE_CROSSWALK  = 0x04,
    CARLA_TYPE_ROAD       = 0x08,
    CARLA_TYPE_GRASS      = 0x10,
    CARLA_TYPE_ALL        = 0xffff,

    CARLA_TYPE_WALKABLE   = CARLA_TYPE_SIDEWALK | CARLA_TYPE_CROSSWALK | CARLA_TYPE_GRASS | CARLA_TYPE_ROAD,
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
    std::vector<carla::geom::Location> &path, std::vector<unsigned char> &area);
    bool GetAgentRoute(ActorId id, carla::geom::Location from, carla::geom::Location to,
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
