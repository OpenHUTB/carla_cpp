
#pragma once

#include "carla/road/RoadTypes.h"
#include "carla/rpc/ActorId.h"

#include "carla/trafficmanager/SimpleWaypoint.h"

namespace carla {
namespace traffic_manager {

using ActorId = carla::ActorId;
using ActorIdSet = std::unordered_set<ActorId>;
using SimpleWaypointPtr = std::shared_ptr<SimpleWaypoint>;
using Buffer = std::deque<SimpleWaypointPtr>;
using GeoGridId = carla::road::JuncId;

// 此类用于跟踪所有角色的航点占用情况
class TrackTraffic {

private:
    /// 用于跟踪车辆间重叠航点的结构
    using WaypointOverlap = std::unordered_map<uint64_t, ActorIdSet>;
    WaypointOverlap waypoint_overlap_tracker;

    /// 用于跟踪车辆所占用航点的结构
    using WaypointIdSet = std::unordered_set<uint64_t>;
    using WaypointOccupancyMap = std::unordered_map<ActorId, WaypointIdSet>;
    WaypointOccupancyMap waypoint_occupied;

    /// 参与者路径所占据的测地线网格
    std::unordered_map<ActorId, std::unordered_set<GeoGridId>> actor_to_grids;
    /// 参与者当前经过的网格
    std::unordered_map<GeoGridId, ActorIdSet> grid_to_actors;
    /// 当前英雄位置
    cg::Location hero_location = cg::Location(0,0,0);


public:
    TrackTraffic();

    /// 更新、移除和检索经过某一路点车辆的方法
    void UpdatePassingVehicle(uint64_t waypoint_id, ActorId actor_id);
    void RemovePassingVehicle(uint64_t waypoint_id, ActorId actor_id);
    ActorIdSet GetPassingVehicles(uint64_t waypoint_id) const;

    void UpdateGridPosition(const ActorId actor_id, const Buffer &buffer);
    void UpdateUnregisteredGridPosition(const ActorId actor_id,
                                        const std::vector<SimpleWaypointPtr> waypoints);

    ActorIdSet GetOverlappingVehicles(ActorId actor_id) const;
    bool IsGeoGridFree(const GeoGridId geogrid_id) const;
    void AddTakenGrid(const GeoGridId geogrid_id, const ActorId actor_id);

    void SetHeroLocation(const cg::Location location);
    cg::Location GetHeroLocation() const;


    /// 从跟踪中删除参与者数据的方法
    void DeleteActor(ActorId actor_id);

    void Clear();
};

} // namespace traffic_manager
} // namespace carla
