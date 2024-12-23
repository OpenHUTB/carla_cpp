// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.
// 包含头文件
#include "carla/client/TrafficLight.h" // TrafficLight类的定义文件
// 提供了与模拟器交互和演员列表相关的功能
#include "carla/client/detail/Simulator.h"
#include "carla/client/ActorList.h"

#include <unordered_map>
#include <unordered_set>

namespace carla {
namespace client {
  // 设置交通信号灯的当前状态（如红灯、黄灯、绿灯）
// 接收一个rpc::TrafficLightState 类型的参数，表示要设置的交通信号灯的状态
  void TrafficLight::SetState(rpc::TrafficLightState state) {
    GetEpisode().Lock()->SetTrafficLightState(*this, state);
  }

  //获取交通信号灯的当前状态

  rpc::TrafficLightState TrafficLight::GetState() const {
    return GetEpisode().Lock()->GetActorSnapshot(*this).state.traffic_light_data.state;
  }
  // 设置绿灯持续时间
// 接收一个浮点数参数，表示绿灯持续时间
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
 
  // 重置交通信号灯组
  void TrafficLight::ResetGroup() {
    // 调用当前交通信号灯所属的交通信号灯组的重置方法
    GetEpisode().Lock()->ResetTrafficLightGroup(*this);
  }

  // 获取交通信号灯影响的车道的路点
  std::vector<SharedPtr<Waypoint>> TrafficLight::GetAffectedLaneWaypoints() const {
    std::vector<SharedPtr<Waypoint>> result; // 存储受影响车道的路点结果
    SharedPtr<Map> carla_map = GetEpisode().Lock()->GetCurrentMap(); // 获取当前的地图对象
    std::vector<SharedPtr<Landmark>> landmarks = carla_map->GetLandmarksFromId(GetOpenDRIVEID()); // 获取与交通信号灯相关的地标
    for (auto& landmark : landmarks) {
      // 遍历地标，检查其影响的车道范围
      for (const road::LaneValidity& validity : landmark->GetValidities()) {
        // 如果车道范围是从小到大的
        if (validity._from_lane < validity._to_lane) {
          for (int lane_id = validity._from_lane; lane_id <= validity._to_lane; ++lane_id) {
            if(lane_id == 0) continue; // 跳过中心车道
            result.emplace_back(
                carla_map->GetWaypointXODR(
                landmark->GetRoadId(), lane_id, static_cast<float>(landmark->GetS()))); // 获取车道路点
          }
        } else { // 车道范围是从大到小的
          for (int lane_id = validity._from_lane; lane_id >= validity._to_lane; --lane_id) {
            if(lane_id == 0) continue; // 跳过中心车道
            result.emplace_back(
                carla_map->GetWaypointXODR(
                landmark->GetRoadId(), lane_id, static_cast<float>(landmark->GetS()))); // 获取车道路点
          }
        }
      }
    }
    return result; //返回受影响的车道路点
  }

  // 获取交通信号灯的边界框方法
  std::vector<geom::BoundingBox> TrafficLight::GetLightBoxes() const {
    return GetEpisode().Lock()->GetLightBoxes(*this);
  }

  // 获取交通信号灯的OpenDRIVE标识符
  road::SignId TrafficLight::GetOpenDRIVEID() const {
    //　通过交通信号灯的快照数据获取其OpenDRIVE的标识符
    return GetEpisode().Lock()->GetActorSnapshot(*this).state.traffic_light_data.sign_id;
  }

  std::vector<SharedPtr<Waypoint>> TrafficLight::GetStopWaypoints() const {
    // 定义一个向量用于存储结果（停止点的路标）
    std::vector<SharedPtr<Waypoint>> result;
    // 获取当前地图的指针
    SharedPtr<Map> carla_map = GetEpisode().Lock()->GetCurrentMap();
    //获取交通信号灯的触发范围
    geom::BoundingBox box = GetTriggerVolume();
    // 获取交通信号灯的全局变换
    geom::Transform transform = GetTransform();
    // 提取Bounding Box 的中心位置
    geom::Location box_position = box.location;
    // 将 Bounding Box 的位置转换为全局坐标
    transform.TransformPoint(box_position);
    // 获取信号灯面朝的方向向量
    geom::Vector3D right_direction = transform.GetForwardVector();
    // 定义遍历 Bounding Box 沿x轴的范围：从最小x值到最大x值
    float min_x = -0.9f*box.extent.x; // x的最小范围
    float max_x = 0.9f*box.extent.x; // x的最大范围
    float current_x = min_x; //初始化当前x值为最小范围
    // 定义一个哈希表，用于记录已经访问过的道路和车道ID
    std::unordered_map<road::RoadId, std::unordered_set<road::LaneId>> road_lanes_map;
    // 遍历Bounding Box 沿x轴的范围
    while (current_x < max_x) {
      // 根据当前x值计算查询点的全局位置
      geom::Location query_point = box_position + geom::Location(right_direction*current_x);
      // 获取查询点对应的路标
      SharedPtr<Waypoint> waypoint = carla_map->GetWaypoint(query_point);
      // 检查当前的路标是否已经被处理过
      if (road_lanes_map[waypoint->GetRoadId()].count(waypoint->GetLaneId()) == 0) {
        // 如果未处理过，则将道路ID和车道ID记录到哈希表中
        road_lanes_map[waypoint->GetRoadId()].insert(waypoint->GetLaneId());
        // 将路标加入结果列表
        result.emplace_back(waypoint);
      }
      // 将当前x值增加，继续处理下一个点
      current_x += 1.f;
    }
    return result; // 返回结果列表
  }

} // namespace client
} // namespace carla
