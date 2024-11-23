
#include "carla/trafficmanager/Constants.h"

#include "carla/trafficmanager/LocalizationStage.h"

namespace carla {
namespace traffic_manager {

using namespace constants::PathBufferUpdate;
using namespace constants::LaneChange;
using namespace constants::WaypointSelection;
using namespace constants::SpeedThreshold;
using namespace constants::Collision;
using namespace constants::MotionPlan;

LocalizationStage::LocalizationStage(
  const std::vector<ActorId> &vehicle_id_list,
  BufferMap &buffer_map,
  const SimulationState &simulation_state,
  TrackTraffic &track_traffic,
  const LocalMapPtr &local_map,
  Parameters &parameters,
  std::vector<ActorId>& marked_for_removal,
  LocalizationFrame &output_array,
  RandomGenerator &random_device)
    : vehicle_id_list(vehicle_id_list),
    buffer_map(buffer_map),
    simulation_state(simulation_state),
    track_traffic(track_traffic),
    local_map(local_map),
    parameters(parameters),
    marked_for_removal(marked_for_removal),
    output_array(output_array),
    random_device(random_device){}

void LocalizationStage::Update(const unsigned long index) {

  const ActorId actor_id = vehicle_id_list.at(index);
  const cg::Location vehicle_location = simulation_state.GetLocation(actor_id);
  const cg::Vector3D heading_vector = simulation_state.GetHeading(actor_id);
  const cg::Vector3D vehicle_velocity_vector = simulation_state.GetVelocity(actor_id);
  const float vehicle_speed = vehicle_velocity_vector.Length();

  // 速度相关的航点视野长度
  float horizon_length = std::max(vehicle_speed * HORIZON_RATE, MINIMUM_HORIZON_LENGTH);
  if (vehicle_speed > HIGHWAY_SPEED) {
    horizon_length = std::max(vehicle_speed * HIGH_SPEED_HORIZON_RATE, MINIMUM_HORIZON_LENGTH);
  }
  const float horizon_square = SQUARE(horizon_length);

  if (buffer_map.find(actor_id) == buffer_map.end()) {
    buffer_map.insert({actor_id, Buffer()});
  }
  Buffer &waypoint_buffer = buffer_map.at(actor_id);

  // 如果车辆离缓冲区里的第一个路点过远则清理路点缓冲区。
  if (!waypoint_buffer.empty() &&
      cg::Math::DistanceSquared(waypoint_buffer.front()->GetLocation(),
                                vehicle_location) > SQUARE(MAX_START_DISTANCE)) {

    auto number_of_pops = waypoint_buffer.size();
    for (uint64_t j = 0u; j < number_of_pops; ++j) {
      PopWaypoint(actor_id, track_traffic, waypoint_buffer);
    }
  }

  bool is_at_junction_entrance = false;
  if (!waypoint_buffer.empty()) {
    // 清除已通过的航点
    float dot_product = DeviationDotProduct(vehicle_location, heading_vector, waypoint_buffer.front()->GetLocation());
    while (dot_product <= 0.0f && !waypoint_buffer.empty()) {
      PopWaypoint(actor_id, track_traffic, waypoint_buffer);
      if (!waypoint_buffer.empty()) {
        dot_product = DeviationDotProduct(vehicle_location, heading_vector, waypoint_buffer.front()->GetLocation());
      }
    }

    if (!waypoint_buffer.empty()) {
      // 确定车辆是否在交叉路口入口处
      SimpleWaypointPtr look_ahead_point = GetTargetWaypoint(waypoint_buffer, JUNCTION_LOOK_AHEAD).first;
      SimpleWaypointPtr front_waypoint = waypoint_buffer.front();
      bool front_waypoint_junction = front_waypoint->CheckJunction();
      is_at_junction_entrance = !front_waypoint_junction && look_ahead_point->CheckJunction();
      if (!is_at_junction_entrance) {
        std::vector<SimpleWaypointPtr> last_passed_waypoints = front_waypoint->GetPreviousWaypoint();
        if (last_passed_waypoints.size() == 1) {
          is_at_junction_entrance = !last_passed_waypoints.front()->CheckJunction() && front_waypoint_junction;
        }
      }
      if (is_at_junction_entrance
          //Town03中的环岛例外情况
          && local_map->GetMapName() == "Carla/Maps/Town03"
          && vehicle_location.SquaredLength() < SQUARE(30)) {
        is_at_junction_entrance = false;
      }
    }

    // 清除缓冲区前端太远的航点，但如果已经到达一个路口，则不要清除
    while (!is_at_junction_entrance
           && !waypoint_buffer.empty()
           && waypoint_buffer.back()->DistanceSquared(waypoint_buffer.front()) > horizon_square + horizon_square
           && !waypoint_buffer.back()->CheckJunction()) {
      PopWaypoint(actor_id, track_traffic, waypoint_buffer, false);
    }
  }

  // 如果缓冲区为空，则进行初始化
  if (waypoint_buffer.empty()) {
    SimpleWaypointPtr closest_waypoint = local_map->GetWaypoint(vehicle_location);
    PushWaypoint(actor_id, track_traffic, waypoint_buffer, closest_waypoint);
  }

  // 分配变道
  const ChangeLaneInfo lane_change_info = parameters.GetForceLaneChange(actor_id);
  bool force_lane_change = lane_change_info.change_lane;
  bool lane_change_direction = lane_change_info.direction;

  //应用保持右侧规则和随机变道参数
  if (!force_lane_change && vehicle_speed > MIN_LANE_CHANGE_SPEED){
    const float perc_keep_right = parameters.GetKeepRightPercentage(actor_id);
    const float perc_random_leftlanechange = parameters.GetRandomLeftLaneChangePercentage(actor_id);
    const float perc_random_rightlanechange = parameters.GetRandomRightLaneChangePercentage(actor_id);
    const bool is_keep_right = perc_keep_right > random_device.next();
    const bool is_random_left_change = perc_random_leftlanechange >= random_device.next();
    const bool is_random_right_change = perc_random_rightlanechange >= random_device.next();

    //确定应应用的参数
    if (is_keep_right || is_random_right_change) {
      force_lane_change = true;
      lane_change_direction = true;
    }
    if (is_random_left_change) {
      if (!force_lane_change) {
        force_lane_change = true;
        lane_change_direction = false;
      } else {
        // 左右车道变更都是强制性的。请在其中选择一个
        lane_change_direction = FIFTYPERC > random_device.next();
      }
    }
  }

  const SimpleWaypointPtr front_waypoint = waypoint_buffer.front();
  const float lane_change_distance = SQUARE(std::max(10.0f * vehicle_speed, INTER_LANE_CHANGE_DISTANCE));

  bool recently_not_executed_lane_change = last_lane_change_swpt.find(actor_id) == last_lane_change_swpt.end();
  bool done_with_previous_lane_change = true;
  if (!recently_not_executed_lane_change) {
    float distance_frm_previous = cg::Math::DistanceSquared(last_lane_change_swpt.at(actor_id)->GetLocation(), vehicle_location);
    done_with_previous_lane_change = distance_frm_previous > lane_change_distance;
    if (done_with_previous_lane_change) last_lane_change_swpt.erase(actor_id);
  }
  bool auto_or_force_lane_change = parameters.GetAutoLaneChange(actor_id) || force_lane_change;
  bool front_waypoint_not_junction = !front_waypoint->CheckJunction();

  if (auto_or_force_lane_change
      && front_waypoint_not_junction
      && (recently_not_executed_lane_change || done_with_previous_lane_change)) {

    SimpleWaypointPtr change_over_point = AssignLaneChange(actor_id, vehicle_location, vehicle_speed,
                                                           force_lane_change, lane_change_direction);

    if (change_over_point != nullptr) {
      if (last_lane_change_swpt.find(actor_id) != last_lane_change_swpt.end()) {
        last_lane_change_swpt.at(actor_id) = change_over_point;
      } else {
        last_lane_change_swpt.insert({actor_id, change_over_point});
      }
      auto number_of_pops = waypoint_buffer.size();
      for (uint64_t j = 0u; j < number_of_pops; ++j) {
        PopWaypoint(actor_id, track_traffic, waypoint_buffer);
      }
      PushWaypoint(actor_id, track_traffic, waypoint_buffer, change_over_point);
    }
  }

  Path imported_path = parameters.GetCustomPath(actor_id);
  Route imported_actions = parameters.GetImportedRoute(actor_id);
  // 我们实际上是在导入一个路径
  if (!imported_path.empty()) {

    ImportPath(imported_path, waypoint_buffer, actor_id, horizon_square);

  } else if (!imported_actions.empty()) {

    ImportRoute(imported_actions, waypoint_buffer, actor_id, horizon_square);

  }

  // 通过随机选择航点填充缓冲区
  else {
    while (waypoint_buffer.back()->DistanceSquared(waypoint_buffer.front()) <= horizon_square) {
      SimpleWaypointPtr furthest_waypoint = waypoint_buffer.back();
      std::vector<SimpleWaypointPtr> next_waypoints = furthest_waypoint->GetNextWaypoint();
      uint64_t selection_index = 0u;
      // 伪随机路径选择，如果发现多个选择
      if (next_waypoints.size() > 1) {
        double r_sample = random_device.next();
        selection_index = static_cast<uint64_t>(r_sample*next_waypoints.size()*0.01);
      } else if (next_waypoints.size() == 0) {
        if (!parameters.GetOSMMode()) {
          std::cout << "This map has dead-end roads, please change the set_open_street_map parameter to true" << std::endl;
        }
        marked_for_removal.push_back(actor_id);
        break;
      }
      SimpleWaypointPtr next_wp_selection = next_waypoints.at(selection_index);
      PushWaypoint(actor_id, track_traffic, waypoint_buffer, next_wp_selection);
      if (next_wp_selection->GetId() == waypoint_buffer.front()->GetId()){
        // 发现了一个环，停止。不要使用零距离，因为可能有两个航点在同一位置
        break;
      }
    }
  }
  ExtendAndFindSafeSpace(actor_id, is_at_junction_entrance, waypoint_buffer);

  // 编辑输出数组
  LocalizationData &output = output_array.at(index);
  output.is_at_junction_entrance = is_at_junction_entrance;

  if (is_at_junction_entrance) {
    const SimpleWaypointPair &safe_space_end_points = vehicles_at_junction_entrance.at(actor_id);
    output.junction_end_point = safe_space_end_points.first;
    output.safe_point = safe_space_end_points.second;
  } else {
    output.junction_end_point = nullptr;
    output.safe_point = nullptr;
  }

  // 更新参与者的测地线网格位置
  track_traffic.UpdateGridPosition(actor_id, waypoint_buffer);
}

void LocalizationStage::ExtendAndFindSafeSpace(const ActorId actor_id,
                                               const bool is_at_junction_entrance,
                                               Buffer &waypoint_buffer) {

  SimpleWaypointPtr junction_end_point = nullptr;
  SimpleWaypointPtr safe_point_after_junction = nullptr;

  if (is_at_junction_entrance
      && vehicles_at_junction_entrance.find(actor_id) == vehicles_at_junction_entrance.end()) {

    bool entered_junction = false;
    bool past_junction = false;
    bool safe_point_found = false;
    SimpleWaypointPtr current_waypoint = nullptr;
    SimpleWaypointPtr junction_begin_point = nullptr;
    float safe_distance_squared = SQUARE(SAFE_DISTANCE_AFTER_JUNCTION);

    // 扫描现有缓冲点
    for (unsigned long i = 0u; i < waypoint_buffer.size() && !safe_point_found; ++i) {
      current_waypoint = waypoint_buffer.at(i);
      if (!entered_junction && current_waypoint->CheckJunction()) {
        entered_junction = true;
        junction_begin_point = current_waypoint;
      }
      if (entered_junction && !past_junction && !current_waypoint->CheckJunction()) {
        past_junction = true;
        junction_end_point = current_waypoint;
      }
      if (past_junction && junction_end_point->DistanceSquared(current_waypoint) > safe_distance_squared) {
        safe_point_found = true;
        safe_point_after_junction = current_waypoint;
      }
    }

    // 如果未找到安全点，则扩展缓冲区
    if (!safe_point_found) {
      bool abort = false;

      while (!past_junction && !abort) {
        NodeList next_waypoints = current_waypoint->GetNextWaypoint();
        if (!next_waypoints.empty()) {
          current_waypoint = next_waypoints.front();
          PushWaypoint(actor_id, track_traffic, waypoint_buffer, current_waypoint);
          if (!current_waypoint->CheckJunction()) {
            past_junction = true;
            junction_end_point = current_waypoint;
          }
        } else {
          abort = true;
        }
      }

      while (!safe_point_found && !abort) {
        std::vector<SimpleWaypointPtr> next_waypoints = current_waypoint->GetNextWaypoint();
        if ((junction_end_point->DistanceSquared(current_waypoint) > safe_distance_squared)
            || next_waypoints.size() > 1
            || current_waypoint->CheckJunction()) {

          safe_point_found = true;
          safe_point_after_junction = current_waypoint;
        } else {
          if (!next_waypoints.empty()) {
            current_waypoint = next_waypoints.front();
            PushWaypoint(actor_id, track_traffic, waypoint_buffer, current_waypoint);
          } else {
            abort = true;
          }
        }
      }
    }

    if (junction_end_point != nullptr &&
        safe_point_after_junction != nullptr &&
        junction_begin_point->DistanceSquared(junction_end_point) < SQUARE(MIN_JUNCTION_LENGTH)) {

      junction_end_point = nullptr;
      safe_point_after_junction = nullptr;
    }

    vehicles_at_junction_entrance.insert({actor_id, {junction_end_point, safe_point_after_junction}});
  }
  else if (!is_at_junction_entrance
           && vehicles_at_junction_entrance.find(actor_id) != vehicles_at_junction_entrance.end()) {

    vehicles_at_junction_entrance.erase(actor_id);
  }
}

void LocalizationStage::RemoveActor(ActorId actor_id) {
    last_lane_change_swpt.erase(actor_id);
    vehicles_at_junction.erase(actor_id);
}

void LocalizationStage::Reset() {
  last_lane_change_swpt.clear();
  vehicles_at_junction.clear();
}

SimpleWaypointPtr LocalizationStage::AssignLaneChange(const ActorId actor_id,
                                                      const cg::Location vehicle_location,
                                                      const float vehicle_speed,
                                                      bool force, bool direction) {

  // 航点表示航点缓冲区的新起点
  // 由于车道变更。如果车道变更不可行，则保持为空指针
  SimpleWaypointPtr change_over_point = nullptr;

  // 获取当前车辆的航点缓冲区
  const Buffer &waypoint_buffer = buffer_map.at(actor_id);

  // 检查缓冲区是否不为空
  if (!waypoint_buffer.empty()) {
    // 获取当前最近航点的左右航点
    const SimpleWaypointPtr &current_waypoint = waypoint_buffer.front();
    const SimpleWaypointPtr left_waypoint = current_waypoint->GetLeftWaypoint();
    const SimpleWaypointPtr right_waypoint = current_waypoint->GetRightWaypoint();

    // 检索与当前车辆重叠路径点缓冲区的车辆
    const auto blocking_vehicles = track_traffic.GetOverlappingVehicles(actor_id);

    // 查找车道内即时障碍物，并检查是否有障碍物距离过近，无法进行变道
    bool obstacle_too_close = false;
    float minimum_squared_distance = std::numeric_limits<float>::infinity();
    ActorId obstacle_actor_id = 0u;
    for (auto i = blocking_vehicles.begin();
         i != blocking_vehicles.end() && !obstacle_too_close && !force;
         ++i) {
      const ActorId &other_actor_id = *i;
      // 在缓冲区地图中查找车辆，并检查其缓冲区是否不为空
      if (buffer_map.find(other_actor_id) != buffer_map.end() && !buffer_map.at(other_actor_id).empty()) {
        const Buffer &other_buffer = buffer_map.at(other_actor_id);
        const SimpleWaypointPtr &other_current_waypoint = other_buffer.front();
        const cg::Location other_location = other_current_waypoint->GetLocation();

        const cg::Vector3D reference_heading = current_waypoint->GetForwardVector();
        cg::Vector3D reference_to_other = other_location - current_waypoint->GetLocation();
        const cg::Vector3D other_heading = other_current_waypoint->GetForwardVector();

        WaypointPtr current_raw_waypoint = current_waypoint->GetWaypoint();
        WaypointPtr other_current_raw_waypoint = other_current_waypoint->GetWaypoint();
        // 检查两辆车是否都不在交叉路口
        //检查另一辆车是否在当前车辆的前方
        // 检查两辆车的航向之间是否存在可接受的角偏差
        if (!current_waypoint->CheckJunction()
            && !other_current_waypoint->CheckJunction()
            && other_current_raw_waypoint->GetRoadId() == current_raw_waypoint->GetRoadId()
            && other_current_raw_waypoint->GetLaneId() == current_raw_waypoint->GetLaneId()
            && cg::Math::Dot(reference_heading, reference_to_other) > 0.0f
            && cg::Math::Dot(reference_heading, other_heading) > MAXIMUM_LANE_OBSTACLE_CURVATURE) {
          float squared_distance = cg::Math::DistanceSquared(vehicle_location, other_location);
          // 如果障碍物太近，则中止
          if (squared_distance > SQUARE(MINIMUM_LANE_CHANGE_DISTANCE)) {
            // 如果新车辆更靠近就记住
            if (squared_distance < minimum_squared_distance && squared_distance < SQUARE(MAXIMUM_LANE_OBSTACLE_DISTANCE)) {
              minimum_squared_distance = squared_distance;
              obstacle_actor_id = other_actor_id;
            }
          } else {
            obstacle_too_close = true;
          }
        }
      }
    }

    // 如果发现有效的即时障碍
    if (!obstacle_too_close && obstacle_actor_id != 0u && !force) {
      const Buffer &other_buffer = buffer_map.at(obstacle_actor_id);
      const SimpleWaypointPtr &other_current_waypoint = other_buffer.front();
      const auto other_neighbouring_lanes = {other_current_waypoint->GetLeftWaypoint(),
                                             other_current_waypoint->GetRightWaypoint()};

      // 反映障碍物附近相邻车道是否畅通的标志
      bool distant_left_lane_free = false;
      bool distant_right_lane_free = false;

      // 检查阻碍车辆附近的相邻车道是否没有其他车辆
      bool left_right = true;
      for (auto &candidate_lane_wp : other_neighbouring_lanes) {
        if (candidate_lane_wp != nullptr &&
            track_traffic.GetPassingVehicles(candidate_lane_wp->GetId()).size() == 0) {

          if (left_right)
            distant_left_lane_free = true;
          else
            distant_right_lane_free = true;
        }
        left_right = !left_right;
      }

      //基于障碍物附近哪些车道是空闲的，
      // 找到没有车辆通过的变更点
      if (distant_right_lane_free && right_waypoint != nullptr
          && track_traffic.GetPassingVehicles(right_waypoint->GetId()).size() == 0) {
        change_over_point = right_waypoint;
      } else if (distant_left_lane_free && left_waypoint != nullptr
               && track_traffic.GetPassingVehicles(left_waypoint->GetId()).size() == 0) {
        change_over_point = left_waypoint;
      }
    } else if (force) {
      if (direction && right_waypoint != nullptr) {
        change_over_point = right_waypoint;
      } else if (!direction && left_waypoint != nullptr) {
        change_over_point = left_waypoint;
      }
    }

    if (change_over_point != nullptr) {
      const float change_over_distance = cg::Math::Clamp(1.5f * vehicle_speed, MIN_WPT_DISTANCE, MAX_WPT_DISTANCE);
      const SimpleWaypointPtr starting_point = change_over_point;
      while (change_over_point->DistanceSquared(starting_point) < SQUARE(change_over_distance) &&
             !change_over_point->CheckJunction()) {
        change_over_point = change_over_point->GetNextWaypoint().front();
      }
    }
  }

  return change_over_point;
}

void LocalizationStage::ImportPath(Path &imported_path, Buffer &waypoint_buffer, const ActorId actor_id, const float horizon_square) {
    // Remove the waypoints already added to the path, except for the first.
    if (parameters.GetUploadPath(actor_id)) {
      auto number_of_pops = waypoint_buffer.size();
      for (uint64_t j = 0u; j < number_of_pops - 1; ++j) {
        PopWaypoint(actor_id, track_traffic, waypoint_buffer, false);
      }
      // We have successfully imported the path. Remove it from the list of paths to be imported.
      parameters.RemoveUploadPath(actor_id, false);
    }

    // Get the latest imported waypoint. and find its closest waypoint in TM's InMemoryMap.
    cg::Location latest_imported = imported_path.front();
    SimpleWaypointPtr imported = local_map->GetWaypoint(latest_imported);

    // We need to generate a path compatible with TM's waypoints.
    while (!imported_path.empty() && waypoint_buffer.back()->DistanceSquared(waypoint_buffer.front()) <= horizon_square) {
      // Get the latest point we added to the list. If starting, this will be the one referred to the vehicle's location.
      SimpleWaypointPtr latest_waypoint = waypoint_buffer.back();

      // Try to link the latest_waypoint to the imported waypoint.
      std::vector<SimpleWaypointPtr> next_waypoints = latest_waypoint->GetNextWaypoint();
      uint64_t selection_index = 0u;

      // Choose correct path.
      if (next_waypoints.size() > 1) {
        const float imported_road_id = imported->GetWaypoint()->GetRoadId();
        float min_distance = std::numeric_limits<float>::infinity();
        for (uint64_t k = 0u; k < next_waypoints.size(); ++k) {
          SimpleWaypointPtr junction_end_point = next_waypoints.at(k);
          while (!junction_end_point->CheckJunction()) {
            junction_end_point = junction_end_point->GetNextWaypoint().front();
          }
          while (junction_end_point->CheckJunction()) {
            junction_end_point = junction_end_point->GetNextWaypoint().front();
          }
          while (next_waypoints.at(k)->DistanceSquared(junction_end_point) < 50.0f) {
            junction_end_point = junction_end_point->GetNextWaypoint().front();
          }
          float jep_road_id = junction_end_point->GetWaypoint()->GetRoadId();
          if (jep_road_id == imported_road_id) {
            selection_index = k;
            break;
          }
          float distance = junction_end_point->DistanceSquared(imported);
          if (distance < min_distance) {
            min_distance = distance;
            selection_index = k;
          }
        }
      } else if (next_waypoints.size() == 0) {
        if (!parameters.GetOSMMode()) {
          std::cout << "This map has dead-end roads, please change the set_open_street_map parameter to true" << std::endl;
        }
        marked_for_removal.push_back(actor_id);
        break;
      }
      SimpleWaypointPtr next_wp_selection = next_waypoints.at(selection_index);

      // Remove the imported waypoint from the path if it's close to the last one.
      if (next_wp_selection->DistanceSquared(imported) < 30.0f) {
        imported_path.erase(imported_path.begin());
        std::vector<SimpleWaypointPtr> possible_waypoints = next_wp_selection->GetNextWaypoint();
        if (std::find(possible_waypoints.begin(), possible_waypoints.end(), imported) != possible_waypoints.end()) {
          // If the lane is changing, only push the new waypoint
          PushWaypoint(actor_id, track_traffic, waypoint_buffer, next_wp_selection);
        }
        PushWaypoint(actor_id, track_traffic, waypoint_buffer, imported);
        latest_imported = imported_path.front();
        imported = local_map->GetWaypoint(latest_imported);
      } else {
        PushWaypoint(actor_id, track_traffic, waypoint_buffer, next_wp_selection);
      }
    }
    if (imported_path.empty()) {
      // Once we are done, check if we can clear the structure.
      parameters.RemoveUploadPath(actor_id, true);
    } else {
      // Otherwise, update the structure with the waypoints that we still need to import.
      parameters.UpdateUploadPath(actor_id, imported_path);
    }
}

void LocalizationStage::ImportRoute(Route &imported_actions, Buffer &waypoint_buffer, const ActorId actor_id, const float horizon_square) {

    if (parameters.GetUploadRoute(actor_id)) {
      auto number_of_pops = waypoint_buffer.size();
      for (uint64_t j = 0u; j < number_of_pops - 1; ++j) {
        PopWaypoint(actor_id, track_traffic, waypoint_buffer, false);
      }
      // We have successfully imported the route. Remove it from the list of routes to be imported.
      parameters.RemoveImportedRoute(actor_id, false);
    }

    RoadOption next_road_option = static_cast<RoadOption>(imported_actions.front());
    while (!imported_actions.empty() && waypoint_buffer.back()->DistanceSquared(waypoint_buffer.front()) <= horizon_square) {
      // Get the latest point we added to the list. If starting, this will be the one referred to the vehicle's location.
      SimpleWaypointPtr latest_waypoint = waypoint_buffer.back();
      RoadOption latest_road_option = latest_waypoint->GetRoadOption();
      // Try to link the latest_waypoint to the correct next RouteOption.
      std::vector<SimpleWaypointPtr> next_waypoints = latest_waypoint->GetNextWaypoint();
      uint16_t selection_index = 0u;
      if (next_waypoints.size() > 1) {
        for (uint16_t i=0; i<next_waypoints.size(); ++i) {
          if (next_waypoints.at(i)->GetRoadOption() == next_road_option) {
            selection_index = i;
            break;
          } else {
            if (i == next_waypoints.size() - 1) {
              std::cout << "We couldn't find the RoadOption you were looking for. This route might diverge from the one expected." << std::endl;
            }
          }
        }
      } else if (next_waypoints.size() == 0) {
        if (!parameters.GetOSMMode()) {
          std::cout << "This map has dead-end roads, please change the set_open_street_map parameter to true" << std::endl;
        }
        marked_for_removal.push_back(actor_id);
        break;
      }

      SimpleWaypointPtr next_wp_selection = next_waypoints.at(selection_index);
      PushWaypoint(actor_id, track_traffic, waypoint_buffer, next_wp_selection);

      // If we are switching to a new RoadOption, it means the current one is already fully imported.
      if (latest_road_option != next_wp_selection->GetRoadOption() && next_road_option == next_wp_selection->GetRoadOption()) {
        imported_actions.erase(imported_actions.begin());
        next_road_option = static_cast<RoadOption>(imported_actions.front());
      }
    }
    if (imported_actions.empty()) {
      // Once we are done, check if we can clear the structure.
      parameters.RemoveImportedRoute(actor_id, true);
    } else {
      // Otherwise, update the structure with the waypoints that we still need to import.
      parameters.UpdateImportedRoute(actor_id, imported_actions);
    }
}

Action LocalizationStage::ComputeNextAction(const ActorId& actor_id) {
  auto waypoint_buffer = buffer_map.at(actor_id);
  auto next_action = std::make_pair(RoadOption::LaneFollow, waypoint_buffer.back()->GetWaypoint());
  bool is_lane_change = false;
  if (last_lane_change_swpt.find(actor_id) != last_lane_change_swpt.end()) {
    // A lane change is happening.
    is_lane_change = true;
    const cg::Vector3D heading_vector = simulation_state.GetHeading(actor_id);
    const cg::Vector3D relative_vector = simulation_state.GetLocation(actor_id) - last_lane_change_swpt.at(actor_id)->GetLocation();
    bool left_heading = (heading_vector.x * relative_vector.y - heading_vector.y * relative_vector.x) > 0.0f;
    if (left_heading) next_action = std::make_pair(RoadOption::ChangeLaneLeft, last_lane_change_swpt.at(actor_id)->GetWaypoint());
    else next_action = std::make_pair(RoadOption::ChangeLaneRight, last_lane_change_swpt.at(actor_id)->GetWaypoint());
  }
  for (auto &swpt : waypoint_buffer) {
    RoadOption road_opt = swpt->GetRoadOption();
    if (road_opt != RoadOption::LaneFollow) {
      if (!is_lane_change) {
        // No lane change in sight, we can assume this will be the next action.
        return std::make_pair(road_opt, swpt->GetWaypoint());
      } else {
        // A lane change will happen as well as another action, we need to figure out which one will happen first.
        cg::Location lane_change = last_lane_change_swpt.at(actor_id)->GetLocation();
        cg::Location actual_location = simulation_state.GetLocation(actor_id);
        auto distance_lane_change = cg::Math::DistanceSquared(actual_location, lane_change);
        auto distance_other_action = cg::Math::DistanceSquared(actual_location, swpt->GetLocation());
        if (distance_lane_change < distance_other_action) return next_action;
        else return std::make_pair(road_opt, swpt->GetWaypoint());
      }
    }
  }
  return next_action;
}

ActionBuffer LocalizationStage::ComputeActionBuffer(const ActorId& actor_id) {

  auto waypoint_buffer = buffer_map.at(actor_id);
  ActionBuffer action_buffer;
  Action lane_change;
  bool is_lane_change = false;
  SimpleWaypointPtr buffer_front = waypoint_buffer.front();
  RoadOption last_road_opt = buffer_front->GetRoadOption();
  action_buffer.push_back(std::make_pair(last_road_opt, buffer_front->GetWaypoint()));
  if (last_lane_change_swpt.find(actor_id) != last_lane_change_swpt.end()) {
    // A lane change is happening.
    is_lane_change = true;
    const cg::Vector3D heading_vector = simulation_state.GetHeading(actor_id);
    const cg::Vector3D relative_vector = simulation_state.GetLocation(actor_id) - last_lane_change_swpt.at(actor_id)->GetLocation();
    bool left_heading = (heading_vector.x * relative_vector.y - heading_vector.y * relative_vector.x) > 0.0f;
    if (left_heading) lane_change = std::make_pair(RoadOption::ChangeLaneLeft, last_lane_change_swpt.at(actor_id)->GetWaypoint());
    else lane_change = std::make_pair(RoadOption::ChangeLaneRight, last_lane_change_swpt.at(actor_id)->GetWaypoint());
  }
  for (auto &wpt : waypoint_buffer) {
    RoadOption current_road_opt = wpt->GetRoadOption();
    if (current_road_opt != last_road_opt) {
      action_buffer.push_back(std::make_pair(current_road_opt, wpt->GetWaypoint()));
      last_road_opt = current_road_opt;
    }
  }
  if (is_lane_change) {
    // Insert the lane change action in the appropriate part of the action buffer.
    auto distance_lane_change = cg::Math::DistanceSquared(waypoint_buffer.front()->GetLocation(), lane_change.second->GetTransform().location);
    for (uint16_t i = 0; i < action_buffer.size(); ++i) {
      auto distance_action = cg::Math::DistanceSquared(waypoint_buffer.front()->GetLocation(), waypoint_buffer.at(i)->GetLocation());
      // If the waypoint related to the next action is further away from the one of the lane change, insert lane change action here.
      // If we reached the end of the buffer, place the action at the end.
      if (i == action_buffer.size()-1) {
        action_buffer.push_back(lane_change);
        break;
      } else if (distance_action > distance_lane_change) {
        action_buffer.insert(action_buffer.begin()+i, lane_change);
        break;
      }
    }
  }
  return action_buffer;
}

} // namespace traffic_manager
} // namespace carla
