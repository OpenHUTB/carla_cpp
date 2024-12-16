
#include "carla/trafficmanager/Constants.h"

#include "carla/trafficmanager/TrackTraffic.h"

namespace carla {
namespace traffic_manager {
// 使用命名空间中的常量
using constants::TrackTraffic::BUFFER_STEP_THROUGH;
using constants::TrackTraffic::INV_BUFFER_STEP_THROUGH;
// TrackTraffic类的构造函数
TrackTraffic::TrackTraffic() {}

void TrackTraffic::UpdateUnregisteredGridPosition(const ActorId actor_id,
                                                  const std::vector<SimpleWaypointPtr> waypoints) {
// 删除指定参与者的现有信息
    DeleteActor(actor_id);

    std::unordered_set<GeoGridId> current_grids;
    // Step through waypoints and update grid list for actor and actor list for grids.
    for (auto &waypoint : waypoints) {
    	 // 更新经过的车辆信息
        UpdatePassingVehicle(waypoint->GetId(), actor_id);
// 获取路点的地理网格 ID
        GeoGridId ggid = waypoint->GetGeodesicGridId();
        // 将网格 ID 插入当前网格集合
        current_grids.insert(ggid);
// 如果网格到参与者的映射中存在该网格 ID
        if (grid_to_actors.find(ggid) != grid_to_actors.end()) {
        	// 获取对应网格的参与者集合
            ActorIdSet &actor_ids = grid_to_actors.at(ggid);
            // 如果参与者 ID 不在集合中，插入参与者 ID
            if (actor_ids.find(actor_id) == actor_ids.end()) {
                actor_ids.insert(actor_id);
            }
        } else {
        	// 如果映射中不存在该网格 ID，创建新的映射关系
            grid_to_actors.insert({ggid, {actor_id}});
        }
    }
// 将参与者 ID 和当前网格集合插入参与者到网格的映射中
    actor_to_grids.insert({actor_id, current_grids});
}

void TrackTraffic::UpdateGridPosition(const ActorId actor_id, const Buffer &buffer) {
	// 如果缓冲区不为空
    if (!buffer.empty()) {

        // Clear current actor from all grids containing itself.
        if (actor_to_grids.find(actor_id) != actor_to_grids.end()) {
            std::unordered_set<GeoGridId> &current_grids = actor_to_grids.at(actor_id);
            // 遍历当前参与者所在的网格集合
            for (auto &grid_id : current_grids) {
            	// 如果网格到参与者的映射中存在该网格 ID
                if (grid_to_actors.find(grid_id) != grid_to_actors.end()) {
                	// 获取对应网格的参与者集合
                    ActorIdSet &actor_ids = grid_to_actors.at(grid_id);
                    // 从集合中删除该参与者 ID
                    actor_ids.erase(actor_id);
                }
            }
// 从参与者到网格的映射中删除该参与者
            actor_to_grids.erase(actor_id);
        }

        // Step through buffer and update grid list for actor and actor list for grids.
        std::unordered_set<GeoGridId> current_grids;
        uint64_t buffer_size = buffer.size();
        // 遍历缓冲区中的路点
        for (uint64_t i = 0u; i <= buffer_size - 1u; ++i) {
            auto waypoint = buffer.at(i);
            GeoGridId ggid = waypoint->GetGeodesicGridId();
            current_grids.insert(ggid);
            // Add grid entry if not present.
            if (grid_to_actors.find(ggid) == grid_to_actors.end()) {
                grid_to_actors.insert({ggid, {}});
            }
// 获取对应网格的参与者集合
            ActorIdSet &actor_ids = grid_to_actors.at(ggid);
            // 如果参与者 ID 不在集合中，插入参与者 ID
            if (actor_ids.find(actor_id) == actor_ids.end()) {
                actor_ids.insert(actor_id);
            }
        }
// 将参与者 ID 和当前网格集合插入参与者到网格的映射中
        actor_to_grids.insert({actor_id, current_grids});
    }
}


bool TrackTraffic::IsGeoGridFree(const GeoGridId geogrid_id) const {
    if (grid_to_actors.find(geogrid_id) != grid_to_actors.end()) {
        return grid_to_actors.at(geogrid_id).empty();
    }
    return true;
}

void TrackTraffic::AddTakenGrid(const GeoGridId geogrid_id, const ActorId actor_id) {
	// 如果网格到参与者的映射中不存在该网格 ID
    if (grid_to_actors.find(geogrid_id) == grid_to_actors.end()) {
    	// 创建新的映射关系（网格 ID 到包含该演员 ID 的集合）
        grid_to_actors.insert({geogrid_id, {actor_id}});
    }
}


void TrackTraffic::SetHeroLocation(const cg::Location _location) {
    hero_location = _location;
}

cg::Location TrackTraffic::GetHeroLocation() const {
    return hero_location;
}

ActorIdSet TrackTraffic::GetOverlappingVehicles(ActorId actor_id) const {
    ActorIdSet actor_id_set;
// 如果参与者在参与者到网格的映射中
    if (actor_to_grids.find(actor_id) != actor_to_grids.end()) {
        const std::unordered_set<GeoGridId> &grid_ids = actor_to_grids.at(actor_id);
        // 遍历参与者所在的网格集合
        for (auto &grid_id : grid_ids) {
        	// 如果网格到参与者的映射中存在该网格 ID
            if (grid_to_actors.find(grid_id) != grid_to_actors.end()) {
            	 // 获取对应网格的参与者集合
                const ActorIdSet &actor_ids = grid_to_actors.at(grid_id);
                 // 将集合中的参与者 ID 插入结果集合
                actor_id_set.insert(actor_ids.begin(), actor_ids.end());
            }
        }
    }

    return actor_id_set;
}

void TrackTraffic::DeleteActor(ActorId actor_id) {
	// 如果参与者在参与者到网格的映射中
    if (actor_to_grids.find(actor_id) != actor_to_grids.end()) {
        std::unordered_set<GeoGridId> &grid_ids = actor_to_grids.at(actor_id);
        // 遍历参与者所在的网格集合
        for (auto &grid_id : grid_ids) {
        	// 如果网格到参与者的映射中存在该网格 ID
            if (grid_to_actors.find(grid_id) != grid_to_actors.end()) {
            	// 获取对应网格的参与者集合
                ActorIdSet &actor_ids = grid_to_actors.at(grid_id);
                // 从集合中删除该参与者 ID
                actor_ids.erase(actor_id);
                }
            }
            // 从参与者到网格的映射中删除该参与者
        actor_to_grids.erase(actor_id);
    }
// 如果参与者在路点占用的映射中
    if (waypoint_occupied.find(actor_id) != waypoint_occupied.end()) {
        WaypointIdSet waypoint_id_set = waypoint_occupied.at(actor_id);
        // 遍历参与者占用的路点 ID 集合
        for (const uint64_t &waypoint_id : waypoint_id_set) {
        	// 移除经过的车辆
            RemovePassingVehicle(waypoint_id, actor_id);
        }
    }
}

void TrackTraffic::UpdatePassingVehicle(uint64_t waypoint_id, ActorId actor_id) {
	 // 如果路点重叠追踪器中存在该路点 ID
    if (waypoint_overlap_tracker.find(waypoint_id) != waypoint_overlap_tracker.end()) {
    	// 获取对应路点的参与者集合
        ActorIdSet &actor_id_set = waypoint_overlap_tracker.at(waypoint_id);
        // 如果参与者 ID 不在集合中，插入参与者 ID
        if (actor_id_set.find(actor_id) == actor_id_set.end()) {
            actor_id_set.insert(actor_id);
        }
    } else {
    	// 如果路点重叠追踪器中不存在该路点 ID，创建新的映射关系
        waypoint_overlap_tracker.insert({waypoint_id, {actor_id}});
    }
// 如果参与者在路点占用的映射中
    if (waypoint_occupied.find(actor_id) != waypoint_occupied.end()) {
    	// 获取对应参与者的路点 ID 集合
        WaypointIdSet &waypoint_id_set = waypoint_occupied.at(actor_id);
        // 如果路点 ID 不在集合中，插入路点 ID
        if (waypoint_id_set.find(waypoint_id) == waypoint_id_set.end()) {
            waypoint_id_set.insert(waypoint_id);
        }
    } else {
    	// 如果参与者不在路点占用的映射中，创建新的映射关系
        waypoint_occupied.insert({actor_id, {waypoint_id}});
    }
}

void TrackTraffic::RemovePassingVehicle(uint64_t waypoint_id, ActorId actor_id) {
	// 如果路点重叠追踪器中存在该路点 ID
    if (waypoint_overlap_tracker.find(waypoint_id) != waypoint_overlap_tracker.end()) {
        ActorIdSet &actor_id_set = waypoint_overlap_tracker.at(waypoint_id);
        actor_id_set.erase(actor_id);
// 如果集合为空，从路点重叠追踪器中删除该路点 ID
        if (actor_id_set.size() == 0) {
            waypoint_overlap_tracker.erase(waypoint_id);
        }
    }
// 如果参与者在路点占用的映射中
    if (waypoint_occupied.find(actor_id) != waypoint_occupied.end()) {
        WaypointIdSet &waypoint_id_set = waypoint_occupied.at(actor_id);
        waypoint_id_set.erase(waypoint_id);
// 如果集合为空，从参与者占用的映射中删除该参与者 ID
        if (waypoint_id_set.size() == 0) {
            waypoint_occupied.erase(actor_id);
        }
    }
}

ActorIdSet TrackTraffic::GetPassingVehicles(uint64_t waypoint_id) const {

    if (waypoint_overlap_tracker.find(waypoint_id) != waypoint_overlap_tracker.end()) {
        return waypoint_overlap_tracker.at(waypoint_id);
    } else {
        return ActorIdSet();
    }
}
// 清空所有数据结构
void TrackTraffic::Clear() {
    waypoint_overlap_tracker.clear();
    waypoint_occupied.clear();
    actor_to_grids.clear();
    grid_to_actors.clear();
}

} // namespace traffic_manager
} // namespace carla
