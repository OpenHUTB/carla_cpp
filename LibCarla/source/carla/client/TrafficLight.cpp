// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/client/TrafficLight.h"
#include "carla/client/detail/Simulator.h"
#include "carla/client/ActorList.h"

#include <unordered_map>
#include <unordered_set>

namespace carla {
namespace client {
  // 设置交通信号灯的当前状态（如红灯、黄灯、绿灯）
  void TrafficLight::SetState(rpc::TrafficLightState state) {
    GetEpisode().Lock()->SetTrafficLightState(*this, state);
  }

  //获取交通信号灯的当前状态
  rpc::TrafficLightState TrafficLight::GetState() const {
    return GetEpisode().Lock()->GetActorSnapshot(*this).state.traffic_light_data.state;
  }
  // 设置绿灯持续时间
  void TrafficLight::SetGreenTime(float green_time) {
    GetEpisode().Lock()->SetTrafficLightGreenTime(*this, green_time);
  }

  // 获取绿灯持续时间
  float TrafficLight::GetGreenTime() const {
    return GetEpisode().Lock()->GetActorSnapshot(*this).state.traffic_light_data.green_time;
  }

  //设置黄灯持续时间
  void TrafficLight::SetYellowTime(float yellow_time) {
    GetEpisode().Lock()->SetTrafficLightYellowTime(*this, yellow_time);
  }

  //获取黄灯持续时间
  float TrafficLight::GetYellowTime() const {
    return GetEpisode().Lock()->GetActorSnapshot(*this).state.traffic_light_data.yellow_time;
  }

  // 设置红灯持续时间
  void TrafficLight::SetRedTime(float red_time) {
    GetEpisode().Lock()->SetTrafficLightRedTime(*this, red_time);
  }

  // 获取红灯持续时间
  float TrafficLight::GetRedTime() const {
    return GetEpisode().Lock()->GetActorSnapshot(*this).state.traffic_light_data.red_time;
  }

  // 获取当前信号灯周期内已过去的时间
  float TrafficLight::GetElapsedTime() const {
    return GetEpisode().Lock()->GetActorSnapshot(*this).state.traffic_light_data.elapsed_time;
  }

  // 冻结或解冻信号灯的状态
  void TrafficLight::Freeze(bool freeze) {
    // 冻结或解冻所有信号灯
    GetEpisode().Lock()->FreezeAllTrafficLights(freeze);
  }

  //检查信号灯是否被冻结
  bool TrafficLight::IsFrozen() const {
    return GetEpisode().Lock()->GetActorSnapshot(*this).state.traffic_light_data.time_is_frozen;
  }

  // 获取当前信号灯的杆编号
  uint32_t TrafficLight::GetPoleIndex()
  {
    return GetEpisode().Lock()->GetActorSnapshot(*this).state.traffic_light_data.pole_index;
  }

  // 获取属于同一组的所有信号灯
  std::vector<SharedPtr<TrafficLight>> TrafficLight::GetGroupTrafficLights() {
    std::vector<SharedPtr<TrafficLight>> result;
    //获取同组的信号灯ID列表
    auto ids = GetEpisode().Lock()->GetGroupTrafficLights(*this);
    for (auto id : ids) {
      // 查找每个ID对应的参与者，并转换为TrafficLight类型，加入结果列表
      SharedPtr<Actor> actor = GetWorld().GetActors()->Find(id);
      result.push_back(boost::static_pointer_cast<TrafficLight>(actor));
    }
    return result;
  }

  void TrafficLight::ResetGroup() {
    GetEpisode().Lock()->ResetTrafficLightGroup(*this);
  }

  std::vector<SharedPtr<Waypoint>> TrafficLight::GetAffectedLaneWaypoints() const {
    std::vector<SharedPtr<Waypoint>> result;
    SharedPtr<Map> carla_map = GetEpisode().Lock()->GetCurrentMap();
    std::vector<SharedPtr<Landmark>> landmarks = carla_map->GetLandmarksFromId(GetOpenDRIVEID());
    for (auto& landmark : landmarks) {
      for (const road::LaneValidity& validity : landmark->GetValidities()) {
        if (validity._from_lane < validity._to_lane) {
          for (int lane_id = validity._from_lane; lane_id <= validity._to_lane; ++lane_id) {
            if(lane_id == 0) continue;
            result.emplace_back(
                carla_map->GetWaypointXODR(
                landmark->GetRoadId(), lane_id, static_cast<float>(landmark->GetS())));
          }
        } else {
          for (int lane_id = validity._from_lane; lane_id >= validity._to_lane; --lane_id) {
            if(lane_id == 0) continue;
            result.emplace_back(
                carla_map->GetWaypointXODR(
                landmark->GetRoadId(), lane_id, static_cast<float>(landmark->GetS())));
          }
        }
      }
    }
    return result;
  }

  std::vector<geom::BoundingBox> TrafficLight::GetLightBoxes() const {
    return GetEpisode().Lock()->GetLightBoxes(*this);
  }

  road::SignId TrafficLight::GetOpenDRIVEID() const {
    return GetEpisode().Lock()->GetActorSnapshot(*this).state.traffic_light_data.sign_id;
  }

  std::vector<SharedPtr<Waypoint>> TrafficLight::GetStopWaypoints() const {
    std::vector<SharedPtr<Waypoint>> result;
    SharedPtr<Map> carla_map = GetEpisode().Lock()->GetCurrentMap();
    geom::BoundingBox box = GetTriggerVolume();
    geom::Transform transform = GetTransform();
    geom::Location box_position = box.location;
    transform.TransformPoint(box_position);
    geom::Vector3D right_direction = transform.GetForwardVector();
    float min_x = -0.9f*box.extent.x;
    float max_x = 0.9f*box.extent.x;
    float current_x = min_x;
    std::unordered_map<road::RoadId, std::unordered_set<road::LaneId>> road_lanes_map;
    while (current_x < max_x) {
      geom::Location query_point = box_position + geom::Location(right_direction*current_x);
      SharedPtr<Waypoint> waypoint = carla_map->GetWaypoint(query_point);
      if (road_lanes_map[waypoint->GetRoadId()].count(waypoint->GetLaneId()) == 0) {
        road_lanes_map[waypoint->GetRoadId()].insert(waypoint->GetLaneId());
        result.emplace_back(waypoint);
      }
      current_x += 1.f;
    }
    return result;
  }

} // namespace client
} // namespace carla
