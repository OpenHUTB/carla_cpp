// Copyright (c) 2021 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <limits>

#include "carla/client/TrafficSign.h"
#include "carla/client/TrafficLight.h"
#include "carla/rpc/TrafficLightState.h"

#include "carla/trafficmanager/Constants.h"
#include "carla/trafficmanager/PIDController.h"

#include "carla/trafficmanager/MotionPlanStage.h"
// 定义carla空间下的traffic_manage空间
namespace carla {
namespace traffic_manager {
// 引入相关的元素
using namespace constants::MotionPlan;
using namespace constants::WaypointSelection;
using namespace constants::SpeedThreshold;
// 引入HybridMode相关的常量到当前作用域
using constants::HybridMode::HYBRID_MODE_DT;
using constants::HybridMode::HYBRID_MODE_DT_FL;
using constants::Collision::EPSILON;
//初始化类的各个成员变量
MotionPlanStage::MotionPlanStage(
  const std::vector<ActorId> &vehicle_id_list,// 车辆ID列表，标识车辆
  SimulationState &simulation_state,// 模拟状态对象
  const Parameters &parameters,//交通管理相关的参数
  const BufferMap &buffer_map,
  TrackTraffic &track_traffic,//跟踪交通状况
  const std::vector<float> &urban_longitudinal_parameters,//车辆纵向运动控制
  const std::vector<float> &highway_longitudinal_parameters,//纵向控制相关的参数列表
  const std::vector<float> &urban_lateral_parameters,//车辆横向运动控制相关操作
  const std::vector<float> &highway_lateral_parameters,//横向控制相关的参数列表
  const LocalizationFrame &localization_frame,//获取车辆的定位信息
  const CollisionFrame&collision_frame,//获取车辆周围的碰撞危险相关数据
  const TLFrame &tl_frame,//获取交通信号灯相关状态信息
  const cc::World &world,//获取世界的快照等全局信息
  ControlFrame &output_array,//输出最终的控制指令控制车辆的行为
  RandomGenerator &random_device,// 随机数生成器对象
 // 局部地图指针，用于在局部范围内进行路径规划
  const LocalMapPtr &local_map)
    : vehicle_id_list(vehicle_id_list),
    simulation_state(simulation_state),
    parameters(parameters),
    buffer_map(buffer_map),
    track_traffic(track_traffic),
    urban_longitudinal_parameters(urban_longitudinal_parameters),
    highway_longitudinal_parameters(highway_longitudinal_parameters),
    urban_lateral_parameters(urban_lateral_parameters),
    highway_lateral_parameters(highway_lateral_parameters),
    localization_frame(localization_frame),
    collision_frame(collision_frame),
    tl_frame(tl_frame),
    world(world),
    output_array(output_array),
    random_device(random_device),
    local_map(local_map) {}
//对车辆的运动规划进行更新
void MotionPlanStage::Update(const unsigned long index) {
  const ActorId actor_id = vehicle_id_list.at(index);// 获取指定索引对应的车辆ID
  const cg::Location vehicle_location = simulation_state.GetLocation(actor_id);//获取获取该车辆当前的位置信息
  const cg::Vector3D vehicle_velocity = simulation_state.GetVelocity(actor_id);//获取该车辆当前的速度向量信息
  const cg::Rotation vehicle_rotation = simulation_state.GetRotation(actor_id);//获取该车辆当前的旋转角度信息
  const float vehicle_speed = vehicle_velocity.Length();// 计算车辆当前的速度大小
  const cg::Vector3D vehicle_heading = simulation_state.GetHeading(actor_id);// 获取车辆当前的行驶方向向量
  const bool vehicle_physics_enabled = simulation_state.IsPhysicsEnabled(actor_id); // 获取该车辆的物理模拟是否启用的状态
  const float vehicle_speed_limit = simulation_state.GetSpeedLimit(actor_id);// 获取该车辆当前所处位置的速度限制信息
  const Buffer &waypoint_buffer = buffer_map.at(actor_id);// 获取与该车辆对应的路点缓冲区数据
  const LocalizationData &localization = localization_frame.at(index);// 获取与该车辆对应的定位数据
  const CollisionHazardData &collision_hazard = collision_frame.at(index); // 获取与该车辆对应的碰撞危险数据
  const bool &tl_hazard = tl_frame.at(index);// 获取与该车辆对应的交通信号灯危险状态
  current_timestamp = world.GetSnapshot().GetTimestamp();//记录更新操作发生的时间等用途
  StateEntry current_state;

  // 实例化传送变换为当前载具变换
  cg::Transform teleportation_transform = cg::Transform(vehicle_location, vehicle_rotation);

  // 从actor_id状态中获取英雄位置信息
  cg::Location hero_location = track_traffic.GetHeroLocation();
  bool is_hero_alive = hero_location != cg::Location(0, 0, 0);

  if (simulation_state.IsDormant(actor_id) && parameters.GetRespawnDormantVehicles() && is_hero_alive) {
    // 冲洗车辆的控制器状态
    current_state = {current_timestamp,
                    0.0f, 0.0f,
                    0.0f};

    // 如果表中不存在，则将条目添加到传送持续时间时钟表中
    if (teleportation_instance.find(actor_id) == teleportation_instance.end()) {
      teleportation_instance.insert({actor_id, current_timestamp});
    }

    // 获取传送载具的下限和上限
    float lower_bound = parameters.GetLowerBoundaryRespawnDormantVehicles();
    float upper_bound = parameters.GetUpperBoundaryRespawnDormantVehicles();
    float dilate_factor = (upper_bound-lower_bound)/100.0f;

    // 测量车辆自上次传送以来所经过的时间
    double elapsed_time = current_timestamp.elapsed_seconds - teleportation_instance.at(actor_id).elapsed_seconds;

    if (parameters.GetSynchronousMode() || elapsed_time > HYBRID_MODE_DT) {
      float random_sample = (static_cast<float>(random_device.next())*dilate_factor) + lower_bound;
      NodeList teleport_waypoint_list = local_map->GetWaypointsInDelta(hero_location, ATTEMPTS_TO_TELEPORT, random_sample);
      if (!teleport_waypoint_list.empty()) {
        for (auto &teleport_waypoint : teleport_waypoint_list) {
          GeoGridId geogrid_id = teleport_waypoint->GetGeodesicGridId();
          if (track_traffic.IsGeoGridFree(geogrid_id)) {
            teleportation_transform = teleport_waypoint->GetTransform();
            teleportation_transform.location.z += 0.5f;
            track_traffic.AddTakenGrid(geogrid_id, actor_id);
            break;
          }
        }
      }
    }
    output_array.at(index) = carla::rpc::Command::ApplyTransform(actor_id, teleportation_transform);

    // 在传送车辆后，使用新的变换更新模拟状态
    KinematicState kinematic_state{teleportation_transform.location,
                                   teleportation_transform.rotation,
                                   vehicle_velocity, vehicle_speed_limit,
                                   vehicle_physics_enabled, simulation_state.IsDormant(actor_id),
                                   teleportation_transform.location};
    simulation_state.UpdateKinematicState(actor_id, kinematic_state);
  }

  else {

    // 目标车速
    float max_target_velocity = parameters.GetVehicleTargetVelocity(actor_id, vehicle_speed_limit) / 3.6f;

    // 接近地标时减速的算法
    float max_landmark_target_velocity = GetLandmarkTargetVelocity(*(waypoint_buffer.at(0)), vehicle_location, actor_id, max_target_velocity);

    // 转弯处减速算法
    float max_turn_target_velocity = GetTurnTargetVelocity(waypoint_buffer, max_target_velocity);
    max_target_velocity = std::min(std::min(max_target_velocity, max_landmark_target_velocity), max_turn_target_velocity);

    // 碰撞处理与目标速度修正
    std::pair<bool, float> collision_response = CollisionHandling(collision_hazard, tl_hazard, vehicle_velocity,
                                                                  vehicle_heading, max_target_velocity);
    bool collision_emergency_stop = collision_response.first;
    float dynamic_target_velocity = collision_response.second;

    // 如果路口后没有足够的空间，请勿进入路口
    bool safe_after_junction = SafeAfterJunction(localization, tl_hazard, collision_emergency_stop);

    // 遇到碰撞或交通灯危险时
    bool emergency_stop = tl_hazard || collision_emergency_stop || !safe_after_junction;

    if (vehicle_physics_enabled && !simulation_state.IsDormant(actor_id)) {
      ActuationSignal actuation_signal{0.0f, 0.0f, 0.0f};

      const float target_point_distance = std::max(vehicle_speed * TARGET_WAYPOINT_TIME_HORIZON,
                                                  MIN_TARGET_WAYPOINT_DISTANCE);
      const SimpleWaypointPtr &target_waypoint = GetTargetWaypoint(waypoint_buffer, target_point_distance).first;
      cg::Location target_location = target_waypoint->GetLocation();

      float offset = parameters.GetLaneOffset(actor_id);
      auto right_vector = target_waypoint->GetTransform().GetRightVector();
      auto offset_location = cg::Location(cg::Vector3D(offset*right_vector.x, offset*right_vector.y, 0.0f));
      target_location = target_location + offset_location;

      float dot_product = DeviationDotProduct(vehicle_location, vehicle_heading, target_location);
      float cross_product = DeviationCrossProduct(vehicle_location, vehicle_heading, target_location);
      dot_product = acos(dot_product) / PI;
      if (cross_product < 0.0f) {
        dot_product *= -1.0f;
      }
      const float angular_deviation = dot_product;
      const float velocity_deviation = (dynamic_target_velocity - vehicle_speed) / dynamic_target_velocity;
      // 如果未找到车辆的上一个状态，则初始化状态条目
      if (pid_state_map.find(actor_id) == pid_state_map.end()) {
        const auto initial_state = StateEntry{current_timestamp, 0.0f, 0.0f, 0.0f};
        pid_state_map.insert({actor_id, initial_state});
      }

      // 检索先前状态
      traffic_manager::StateEntry previous_state;
      previous_state = pid_state_map.at(actor_id);

      // 选择PID参数
      std::vector<float> longitudinal_parameters;
      std::vector<float> lateral_parameters;
      if (vehicle_speed > HIGHWAY_SPEED) {
        longitudinal_parameters = highway_longitudinal_parameters;
        lateral_parameters = highway_lateral_parameters;
      } else {
        longitudinal_parameters = urban_longitudinal_parameters;
        lateral_parameters = urban_lateral_parameters;
      }

      //如果为车辆启用了物理效果，请使用PID控制器
      // 车辆状态更新
      current_state = {current_timestamp, angular_deviation, velocity_deviation, 0.0f};

      // 控制器驱动
      actuation_signal = PID::RunStep(current_state, previous_state,
                                      longitudinal_parameters, lateral_parameters);

      if (emergency_stop) {
        actuation_signal.throttle = 0.0f;
        actuation_signal.brake = 1.0f;
      }

      // 构建执行信号

      carla::rpc::VehicleControl vehicle_control;
      vehicle_control.throttle = actuation_signal.throttle;
      vehicle_control.brake = actuation_signal.brake;
      vehicle_control.steer = actuation_signal.steer;

      output_array.at(index) = carla::rpc::Command::ApplyVehicleControl(actor_id, vehicle_control);

      // 更新PID状态
      current_state.steer = actuation_signal.steer;
      StateEntry &state = pid_state_map.at(actor_id);
      state = current_state;
    }
    // 对于无物理特性的载具，确定传送时的位置和方向
    else {
      // 冲洗车辆的控制器状态
      current_state = {current_timestamp,
                      0.0f, 0.0f,
                      0.0f};

      // 如果不在表中，则将条目添加到传送持续时间时钟表中
      if (teleportation_instance.find(actor_id) == teleportation_instance.end()) {
        teleportation_instance.insert({actor_id, current_timestamp});
      }

      // 测量车辆自上次传送以来的时间
      double elapsed_time = current_timestamp.elapsed_seconds - teleportation_instance.at(actor_id).elapsed_seconds;

      // 在车辆前方找到一个传送位置，以实现预期的速度
      if (!emergency_stop && (parameters.GetSynchronousMode() || elapsed_time > HYBRID_MODE_DT)) {

        // 目标位移量以达到目标速度
        const float target_displacement = dynamic_target_velocity * HYBRID_MODE_DT_FL;
        SimpleWaypointPtr teleport_target = waypoint_buffer.front();
        cg::Transform target_base_transform = teleport_target->GetTransform();
        cg::Location target_base_location = target_base_transform.location;
        cg::Vector3D target_heading = target_base_transform.GetForwardVector();
        cg::Vector3D correct_heading = (target_base_location - vehicle_location).MakeSafeUnitVector(EPSILON);

        if (vehicle_location.Distance(target_base_location) < target_displacement) {
          cg::Location teleportation_location = vehicle_location + cg::Location(target_heading.MakeSafeUnitVector(EPSILON) * target_displacement);
          teleportation_transform = cg::Transform(teleportation_location, target_base_transform.rotation);
        }
        else {
          cg::Location teleportation_location = vehicle_location + cg::Location(correct_heading * target_displacement);
          teleportation_transform = cg::Transform(teleportation_location, target_base_transform.rotation);
        }
      // 在紧急停止的情况下，请保持在同一位置
      // 此外，在异步模式下，每 dt 时间仅传送一次
      } else {
        teleportation_transform = cg::Transform(vehicle_location, simulation_state.GetRotation(actor_id));
      }
      // 构建执行信号
      output_array.at(index) = carla::rpc::Command::ApplyTransform(actor_id, teleportation_transform);
      simulation_state.UpdateKinematicHybridEndLocation(actor_id, teleportation_transform.location);
    }
  }
}

bool MotionPlanStage::SafeAfterJunction(const LocalizationData &localization,
                                        const bool tl_hazard,
                                        const bool collision_emergency_stop) {

  SimpleWaypointPtr junction_end_point = localization.junction_end_point;
  SimpleWaypointPtr safe_point = localization.safe_point;

  bool safe_after_junction = true;
  if (!tl_hazard && !collision_emergency_stop
      && localization.is_at_junction_entrance
      && junction_end_point != nullptr && safe_point != nullptr
      && junction_end_point->DistanceSquared(safe_point) > SQUARE(MIN_SAFE_INTERVAL_LENGTH)) {

    ActorIdSet passing_safe_point = track_traffic.GetPassingVehicles(safe_point->GetId());
    ActorIdSet passing_junction_end_point = track_traffic.GetPassingVehicles(junction_end_point->GetId());
    cg::Location mid_point = (junction_end_point->GetLocation() + safe_point->GetLocation())/2.0f;

    // 只检查那些在其通过路径点中具有安全点的车辆，但不连接端点
    ActorIdSet difference;
    std::set_difference(passing_safe_point.begin(), passing_safe_point.end(),
                        passing_junction_end_point.begin(), passing_junction_end_point.end(),
                        std::inserter(difference, difference.begin()));
    if (difference.size() > 0) {
      for (const ActorId &blocking_id: difference) {
        cg::Location blocking_actor_location = simulation_state.GetLocation(blocking_id);
        if (cg::Math::DistanceSquared(blocking_actor_location, mid_point) < SQUARE(MAX_JUNCTION_BLOCK_DISTANCE)
            && simulation_state.GetVelocity(blocking_id).SquaredLength() < SQUARE(AFTER_JUNCTION_MIN_SPEED)) {
          safe_after_junction = false;
          break;
        }
      }
    }
  }

  return safe_after_junction;
}

std::pair<bool, float> MotionPlanStage::CollisionHandling(const CollisionHazardData &collision_hazard,
                                                          const bool tl_hazard,
                                                          const cg::Vector3D vehicle_velocity,
                                                          const cg::Vector3D vehicle_heading,
                                                          const float max_target_velocity) {
  bool collision_emergency_stop = false;
  float dynamic_target_velocity = max_target_velocity;
  const float vehicle_speed = vehicle_velocity.Length();

  if (collision_hazard.hazard && !tl_hazard) {
    const ActorId other_actor_id = collision_hazard.hazard_actor_id;
    const cg::Vector3D other_velocity = simulation_state.GetVelocity(other_actor_id);
    const float vehicle_relative_speed = (vehicle_velocity - other_velocity).Length();
    const float available_distance_margin = collision_hazard.available_distance_margin;

    const float other_speed_along_heading = cg::Math::Dot(other_velocity, vehicle_heading);

    //只有在存在正相对速度的情况下，才考虑避撞决策
    // 自车（即，自车正在缩小与前车的间距）
    if (vehicle_relative_speed > EPSILON_RELATIVE_SPEED) {
      // 如果其他车辆正在接近领头车辆，并且领头车辆距离更远
      // 跟随距离：0公里/小时 -> 5米，100公里/小时 -> 10米
      float follow_lead_distance = FOLLOW_LEAD_FACTOR * vehicle_speed + MIN_FOLLOW_LEAD_DISTANCE;
      if (available_distance_margin > follow_lead_distance) {
        //然后缩小车辆之间的距离，直到达到跟随前车的距离
        // 通过保持与其他_沿航向_速度的相对速度
        dynamic_target_velocity = other_speed_along_heading;
      }
      // 如果车辆正在接近前车，而前车距离较远
      // 但比CRITICAL_BRAKING_MARGIN更近，且比FOLLOW_LEAD_DISTANCE更近
      else if (available_distance_margin > CRITICAL_BRAKING_MARGIN) {
        // 然后跟随前导车辆，获取其沿当前航向的速度
        dynamic_target_velocity = std::max(other_speed_along_heading, RELATIVE_APPROACH_SPEED);
      } else {
        // 如果前车距离小于紧急制动距离，则启动紧急制动
        collision_emergency_stop = true;
      }
    }
    if (available_distance_margin < CRITICAL_BRAKING_MARGIN) {
      collision_emergency_stop = true;
    }
  }

  float max_gradual_velocity = PERC_MAX_SLOWDOWN * vehicle_speed;
  if (dynamic_target_velocity < vehicle_speed - max_gradual_velocity) {
    // 不要每帧减速超过PERC_MAX_SLOWDOWN
    dynamic_target_velocity = vehicle_speed - max_gradual_velocity;
  }
  dynamic_target_velocity = std::min(max_target_velocity, dynamic_target_velocity);

  return {collision_emergency_stop, dynamic_target_velocity};
}

float MotionPlanStage::GetLandmarkTargetVelocity(const SimpleWaypoint& waypoint,
                                                 const cg::Location vehicle_location,
                                                 const ActorId actor_id,
                                                 float max_target_velocity) {

    auto const max_distance = LANDMARK_DETECTION_TIME * max_target_velocity;

    float landmark_target_velocity = std::numeric_limits<float>::max();

    auto all_landmarks = waypoint.GetWaypoint()->GetAllLandmarksInDistance(max_distance, false);

    for (auto &landmark: all_landmarks) {

      auto landmark_location = landmark->GetWaypoint()->GetTransform().location;
      auto landmark_type = landmark->GetType();
      auto distance = landmark_location.Distance(vehicle_location);

      if (distance > max_distance) {
        continue;
      }

      float minimum_velocity = max_target_velocity;
      if (landmark_type == "1000001") {  // 交通信号灯
        minimum_velocity = TL_TARGET_VELOCITY;
      } else if (landmark_type == "206") {  // 停止
        minimum_velocity = STOP_TARGET_VELOCITY;
      } else if (landmark_type == "205") {  // 产出
        minimum_velocity = YIELD_TARGET_VELOCITY;
      } else if (landmark_type == "274") {  // 速度限制
        float value = static_cast<float>(landmark->GetValue()) / 3.6f;
        value = parameters.GetVehicleTargetVelocity(actor_id, value);
        minimum_velocity = (value < max_target_velocity) ? value : max_target_velocity;
      } else {
        continue;
      }

      float v = std::max(((max_target_velocity - minimum_velocity) / max_distance) * distance + minimum_velocity, minimum_velocity);
      landmark_target_velocity = std::min(landmark_target_velocity, v);
    }

    return landmark_target_velocity;
}

float MotionPlanStage::GetTurnTargetVelocity(const Buffer &waypoint_buffer,
                                             float max_target_velocity) {

  if (waypoint_buffer.size() < 3) {
    return max_target_velocity;
  }
  else {
    const SimpleWaypointPtr first_waypoint = waypoint_buffer.front();
    const SimpleWaypointPtr last_waypoint = waypoint_buffer.back();
    const SimpleWaypointPtr middle_waypoint = waypoint_buffer.at(static_cast<uint16_t>(waypoint_buffer.size() / 2));

    float radius = GetThreePointCircleRadius(first_waypoint->GetLocation(),
                                             middle_waypoint->GetLocation(),
                                             last_waypoint->GetLocation());

    // 返回转弯处的最大速度
    return std::sqrt(radius * FRICTION * GRAVITY);
  }
}

float MotionPlanStage::GetThreePointCircleRadius(cg::Location first_location,
                                                 cg::Location middle_location,
                                                 cg::Location last_location) {

    float x1 = first_location.x;
    float y1 = first_location.y;
    float x2 = middle_location.x;
    float y2 = middle_location.y;
    float x3 = last_location.x;
    float y3 = last_location.y;

    float x12 = x1 - x2;
    float x13 = x1 - x3;
    float y12 = y1 - y2;
    float y13 = y1 - y3;
    float y31 = y3 - y1;
    float y21 = y2 - y1;
    float x31 = x3 - x1;
    float x21 = x2 - x1;

    float sx13 = x1 * x1 - x3 * x3;
    float sy13 = y1 * y1 - y3 * y3;
    float sx21 = x2 * x2 - x1 * x1;
    float sy21 = y2 * y2 - y1 * y1;

    float f_denom = 2 * (y31 * x12 - y21 * x13);
    if (f_denom == 0) {
      return std::numeric_limits<float>::max();
    }
    float f = (sx13 * x12 + sy13 * x12 + sx21 * x13 + sy21 * x13) / f_denom;

    float g_denom = 2 * (x31 * y12 - x21 * y13);
    if (g_denom == 0) {
      return std::numeric_limits<float>::max();
    }
    float g = (sx13 * y12 + sy13 * y12 + sx21 * y13 + sy21 * y13) / g_denom;

    float c = - (x1 * x1 + y1 * y1) - 2 * g * x1 - 2 * f * y1;
    float h = -g;
    float k = -f;

  return std::sqrt(h * h + k * k - c);
}

void MotionPlanStage::RemoveActor(const ActorId actor_id) {
  pid_state_map.erase(actor_id);
  teleportation_instance.erase(actor_id);
}

void MotionPlanStage::Reset() {
  pid_state_map.clear();
  teleportation_instance.clear();
}

} // namespace traffic_manager
} // namespace carla
