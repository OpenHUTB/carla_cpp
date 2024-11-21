
#include "carla/geom/Math.h"

#include "carla/trafficmanager/Constants.h"
#include "carla/trafficmanager/LocalizationUtils.h"

#include "carla/trafficmanager/CollisionStage.h"

namespace carla {
namespace traffic_manager {

using Point2D = bg::model::point<double, 2, bg::cs::cartesian>; // 定义一个二维笛卡尔坐标系点类型
using TLS = carla::rpc::TrafficLightState; // 简化交通信号灯状态的命名空间

using namespace constants::Collision; // 引入碰撞相关的常量
using constants::WaypointSelection::JUNCTION_LOOK_AHEAD; // 引入路口前瞻距离常量

// 碰撞阶段的构造函数
CollisionStage::CollisionStage(
  const std::vector<ActorId> &vehicle_id_list, // 所有车辆的ID列表
  const SimulationState &simulation_state,     // 仿真状态
  const BufferMap &buffer_map,                 // 每辆车的路径缓存
  const TrackTraffic &track_traffic,           // 交通跟踪器
  const Parameters &parameters,                // 仿真参数
  CollisionFrame &output_array,                // 碰撞信息输出
  RandomGenerator &random_device)              // 随机数生成器
  : vehicle_id_list(vehicle_id_list),
    simulation_state(simulation_state),
    buffer_map(buffer_map),
    track_traffic(track_traffic),
    parameters(parameters),
    output_array(output_array),
    random_device(random_device) {}

// 更新指定索引的车辆碰撞状态
void CollisionStage::Update(const unsigned long index) {
  ActorId obstacle_id = 0u; // 障碍物ID ，初始为0
  bool collision_hazard = false; //碰撞风险标志
  float available_distance_margin = std::numeric_limits<float>::infinity(); // 可用距离裕量，初始为正无穷

  // 获取当前车辆的ID
  const ActorId ego_actor_id = vehicle_id_list.at(index);
  if (simulation_state.ContainsActor(ego_actor_id)) { // 检查仿真中是否包含此车辆
    const cg::Location ego_location = simulation_state.GetLocation(ego_actor_id); // 获取车辆当前位置
    const Buffer &ego_buffer = buffer_map.at(ego_actor_id); // 获取车辆的路径缓存
    const unsigned long look_ahead_index = GetTargetWaypoint(ego_buffer, JUNCTION_LOOK_AHEAD).second; // 计算前瞻路径点索引
    const float velocity = simulation_state.GetVelocity(ego_actor_id).Length(); // 获取车辆速度

    // 获取与当前车辆路径重叠的其他车辆ID
    ActorIdSet overlapping_actors = track_traffic.GetOverlappingVehicles(ego_actor_id);
    std::vector<ActorId> collision_candidate_ids; // 碰撞候选车辆ID列表
    // 根据速度和参数计算碰撞检测的最大半径平方
    const float distance_to_leading = parameters.GetDistanceToLeadingVehicle(ego_actor_id); // 获取前车的安全距离
    float collision_radius_square = SQUARE(COLLISION_RADIUS_RATE * velocity + COLLISION_RADIUS_MIN); // 碰撞半径平方
    if (velocity < 2.0f) { // 如果车辆速度较低
      const float length = simulation_state.GetDimensions(ego_actor_id).x; // 获取车辆长度
      const float collision_radius_stop = COLLISION_RADIUS_STOP + length; // 设置静止时的碰撞半径
      collision_radius_square = SQUARE(collision_radius_stop);
    }
    if (distance_to_leading > collision_radius_square) { // 如果前车距离更大
        collision_radius_square = SQUARE(distance_to_leading);
    }

    // 遍历重叠路径上的其他车辆，筛选碰撞候选车辆
    for (ActorId overlapping_actor_id : overlapping_actors) {
      // 如果其他车辆在最大碰撞避免范围内，并且垂直方向有重叠
      const cg::Location &overlapping_actor_location = simulation_state.GetLocation(overlapping_actor_id); // 获取重叠车辆的位置
      if (overlapping_actor_id != ego_actor_id // 排除自身
          && cg::Math::DistanceSquared(overlapping_actor_location, ego_location) < collision_radius_square  // 检测是否在碰撞半径范围内
          && std::abs(ego_location.z - overlapping_actor_location.z) < VERTICAL_OVERLAP_THRESHOLD) { // 检测垂直方向的重叠
        collision_candidate_ids.push_back(overlapping_actor_id); // 添加到碰撞候选列表
      }
    }

    // 按与自车的距离对潜在碰撞对象进行升序排序
    std::sort(collision_candidate_ids.begin(), collision_candidate_ids.end(),
              [this, &ego_location](const ActorId &a_id_1, const ActorId &a_id_2) {
                const cg::Location &e_loc = ego_location; // 自车位置
                const cg::Location &loc_1 = simulation_state.GetLocation(a_id_1); // 对象1的位置
                const cg::Location &loc_2 = simulation_state.GetLocation(a_id_2); // 对象2的位置
                // 按距离平方排序，避免使用平方根计算，提升性能
                return (cg::Math::DistanceSquared(e_loc, loc_1) < cg::Math::DistanceSquared(e_loc, loc_2));
              });

    // 遍历排序后的对象，检查每个对象是否构成碰撞威胁
    for (auto iter = collision_candidate_ids.begin();
         iter != collision_candidate_ids.end() && !collision_hazard;
         ++iter) {
      const ActorId other_actor_id = *iter; // 当前检查的对象ID
      const ActorType other_actor_type = simulation_state.GetType(other_actor_id); // 对象的类型（车辆/行人）
      // 检查碰撞检测条件是否满足
      if (parameters.GetCollisionDetection(ego_actor_id, other_actor_id) // 检查自车与目标车之间的碰撞检测设置
          && buffer_map.find(ego_actor_id) != buffer_map.end()           // 检查缓冲区是否存在自车
          && simulation_state.ContainsActor(other_actor_id)) {           // 检查目标对象是否仍在场景中
        // 通过协商函数计算碰撞威胁
        std::pair<bool, float> negotiation_result = NegotiateCollision(ego_actor_id,
                                                                       other_actor_id,
                                                                       look_ahead_index);
        if (negotiation_result.first) { // 如果存在碰撞威胁
          // 根据对象类型和随机概率，决定是否忽略此威胁
          if ((other_actor_type == ActorType::Vehicle
               && parameters.GetPercentageIgnoreVehicles(ego_actor_id) <= random_device.next())
              || (other_actor_type == ActorType::Pedestrian
                  && parameters.GetPercentageIgnoreWalkers(ego_actor_id) <= random_device.next())) {
            collision_hazard = true;      // 标记碰撞威胁
            obstacle_id = other_actor_id; // 记录威胁对象ID
            available_distance_margin = negotiation_result.second; // 记录距离裕度
          }
        }
      }
    }
  }

  // 更新输出碰撞数据
  CollisionHazardData &output_element = output_array.at(index);
  output_element.hazard_actor_id = obstacle_id; // 威胁对象ID
  output_element.hazard = collision_hazard;     // 是否存在碰撞威胁
  output_element.available_distance_margin = available_distance_margin; // 距离裕度
}

void CollisionStage::RemoveActor(const ActorId actor_id) {
  // 移除特定对象的碰撞锁定
  collision_locks.erase(actor_id);
}

void CollisionStage::Reset() {
  // 清空所有碰撞锁定
  collision_locks.clear();
}

float CollisionStage::GetBoundingBoxExtention(const ActorId actor_id) {
  // 根据速度计算对象的碰撞边界延伸
  const float velocity = cg::Math::Dot(simulation_state.GetVelocity(actor_id), simulation_state.GetHeading(actor_id)); // 计算对象的速度
  float bbox_extension;
  // 使用函数来计算边界长度
  float velocity_extension = VEL_EXT_FACTOR * velocity; // 根据速度计算延伸因子
  bbox_extension = BOUNDARY_EXTENSION_MINIMUM + velocity_extension * velocity_extension; // 基础边界延伸
  // 如果对象有有效的碰撞锁定，调整边界以保持锁定
  if (collision_locks.find(actor_id) != collision_locks.end()) {
    const CollisionLock &lock = collision_locks.at(actor_id);
    float lock_boundary_length = static_cast<float>(lock.distance_to_lead_vehicle + LOCKING_DISTANCE_PADDING);
    // 仅当前车辆距离未超过速度相关延伸的最大值时，才延伸边界跟踪车辆
    if ((lock_boundary_length - lock.initial_lock_distance) < MAX_LOCKING_EXTENSION) {
      bbox_extension = lock_boundary_length;
    }
  }

  return bbox_extension; // 返回最终计算的边界长度
}

LocationVector CollisionStage::GetBoundary(const ActorId actor_id) {
  const ActorType actor_type = simulation_state.GetType(actor_id); // 获取实体类型
  const cg::Vector3D heading_vector = simulation_state.GetHeading(actor_id); // 获取实体的朝向向量

  float forward_extension = 0.0f; // 用于扩展边界框的向前长度
  if (actor_type == ActorType::Pedestrian) {
    // 扩展行人的边界框，用于预测行人未来的位置，从而避免碰撞
    forward_extension = simulation_state.GetVelocity(actor_id).Length() * WALKER_TIME_EXTENSION; // 根据速度扩展
  }

  cg::Vector3D dimensions = simulation_state.GetDimensions(actor_id); // 获取实体的尺寸

  float bbox_x = dimensions.x; // 边界框的x轴长度（前后方向）
  float bbox_y = dimensions.y; // 边界框的y轴长度（左右方向）

  const cg::Vector3D x_boundary_vector = heading_vector * (bbox_x + forward_extension); // 计算x方向的边界向量
  const auto perpendicular_vector = cg::Vector3D(-heading_vector.y, heading_vector.x, 0.0f).MakeSafeUnitVector(EPSILON); // 计算垂直于朝向的单位向量
  const cg::Vector3D y_boundary_vector = perpendicular_vector * (bbox_y + forward_extension); // 计算y方向的边界向量

  // 四个顶点，按照顺时针顺序（左手坐标系下的顶视图）
  const cg::Location location = simulation_state.GetLocation(actor_id); // 获取实体位置
  LocationVector bbox_boundary = {
      location + cg::Location(x_boundary_vector - y_boundary_vector), // 左前角
      location + cg::Location(-1.0f * x_boundary_vector - y_boundary_vector), // 左后角
      location + cg::Location(-1.0f * x_boundary_vector + y_boundary_vector), // 右后角
      location + cg::Location(x_boundary_vector + y_boundary_vector), // 右前角
  };

  return bbox_boundary; // 返回边界框
}

LocationVector CollisionStage::GetGeodesicBoundary(const ActorId actor_id) {
  LocationVector geodesic_boundary;

  if (geodesic_boundary_map.find(actor_id) != geodesic_boundary_map.end()) {
    // 如果地理边界已经缓存，则直接获取
    geodesic_boundary = geodesic_boundary_map.at(actor_id);
  } else {
    const LocationVector bbox = GetBoundary(actor_id); //获取边界框

    if (buffer_map.find(actor_id) != buffer_map.end()) {
      float bbox_extension = GetBoundingBoxExtention(actor_id); // 获取边界框扩展值
      const float specific_lead_distance = parameters.GetDistanceToLeadingVehicle(actor_id); // 获取特定的前车距离
      bbox_extension = std::max(specific_lead_distance, bbox_extension); // 扩展边界框，使用更大的距离
      const float bbox_extension_square = SQUARE(bbox_extension); // 计算扩展距离的平方

      LocationVector left_boundary; // 左边界点集合
      LocationVector right_boundary; // 右边界点集合
      cg::Vector3D dimensions = simulation_state.GetDimensions(actor_id); // 获取实体的尺寸
      const float width = dimensions.y; // 宽度
      const float length = dimensions.x; // 长度

      const Buffer &waypoint_buffer = buffer_map.at(actor_id); // 获取路径缓冲区
      const TargetWPInfo target_wp_info = GetTargetWaypoint(waypoint_buffer, length); // 获取目标路径点和起点索引
      const SimpleWaypointPtr boundary_start = target_wp_info.first; // 边界起始路径点
      const uint64_t boundary_start_index = target_wp_info.second; // 边界起始索引

      // 在无信号交叉口，我们扩展边界穿过交叉口
      // 在所有其他情况下，边界长度与速度相关
      SimpleWaypointPtr boundary_end = nullptr;
      SimpleWaypointPtr current_point = waypoint_buffer.at(boundary_start_index);
      bool reached_distance = false;
      for (uint64_t j = boundary_start_index; !reached_distance && (j < waypoint_buffer.size()); ++j) {
        if (boundary_start->DistanceSquared(current_point) > bbox_extension_square || j == waypoint_buffer.size() - 1) {
          reached_distance = true;
        }
        if (boundary_end == nullptr
            || cg::Math::Dot(boundary_end->GetForwardVector(), current_point->GetForwardVector()) < COS_10_DEGREES
            || reached_distance) {

          const cg::Vector3D heading_vector = current_point->GetForwardVector();
          const cg::Location location = current_point->GetLocation();
          cg::Vector3D perpendicular_vector = cg::Vector3D(-heading_vector.y, heading_vector.x, 0.0f);
          perpendicular_vector = perpendicular_vector.MakeSafeUnitVector(EPSILON);
          // 方向根据左手坐标系确定
          const cg::Vector3D scaled_perpendicular = perpendicular_vector * width;
          left_boundary.push_back(location + cg::Location(scaled_perpendicular));
          right_boundary.push_back(location + cg::Location(-1.0f * scaled_perpendicular));

          boundary_end = current_point;
        }

        current_point = waypoint_buffer.at(j);
      }

      // 反向右边界以构建顺时针（左手坐标系）
      // 边界。这是因为左边界和右边界向量都有
      // 在右边界的起始索引处与车辆的最近点
      // 边界
      // 我们希望从最远的点开始，以获得顺时针轨迹
      std::reverse(right_boundary.begin(), right_boundary.end());
      geodesic_boundary.insert(geodesic_boundary.end(), right_boundary.begin(), right_boundary.end());
      geodesic_boundary.insert(geodesic_boundary.end(), bbox.begin(), bbox.end());
      geodesic_boundary.insert(geodesic_boundary.end(), left_boundary.begin(), left_boundary.end());
    } else {

      geodesic_boundary = bbox;
    }

    geodesic_boundary_map.insert({actor_id, geodesic_boundary});
  }

  return geodesic_boundary;
}

Polygon CollisionStage::GetPolygon(const LocationVector &boundary) {

  traffic_manager::Polygon boundary_polygon; //定义一个多边形对象
  for (const cg::Location &location : boundary) {
    // 将边界点逐一添加到多边形外环中
    bg::append(boundary_polygon.outer(), Point2D(location.x, location.y));
  }
  // 将起始点再次添加到外环，闭合多边形
  bg::append(boundary_polygon.outer(), Point2D(boundary.front().x, boundary.front().y));

  return boundary_polygon; // 返回多边形
}

GeometryComparison CollisionStage::GetGeometryBetweenActors(const ActorId reference_vehicle_id,
                                                            const ActorId other_actor_id) {


  std::pair<ActorId, ActorId> key_parts;
  if (reference_vehicle_id < other_actor_id) {
    // 确保缓存键的生成始终保持一致，选择较小的 ActorId 为第一个键
    key_parts = {reference_vehicle_id, other_actor_id};
  } else {
    key_parts = {other_actor_id, reference_vehicle_id};
  }

  uint64_t actor_id_key = 0u; // 用于存储唯一键
  actor_id_key |= key_parts.first; // 首先存储第一个实体 ID
  actor_id_key <<= 32; // 左移32位，为第二个实体ID留出空间
  actor_id_key |= key_parts.second; // 添加第二个实体ID

  GeometryComparison comparision_result{-1.0, -1.0, -1.0, -1.0}; // 默认比较结果，初始化为-1.0

  if (geometry_cache.find(actor_id_key) != geometry_cache.end()) {
    // 如果几何关系已缓存，则直接获取
    comparision_result = geometry_cache.at(actor_id_key);
    double mref_veh_other = comparision_result.reference_vehicle_to_other_geodesic;
    // 交换参考车辆到其他车辆的距离和相反方向的距离
    comparision_result.reference_vehicle_to_other_geodesic = comparision_result.other_vehicle_to_reference_geodesic;
    comparision_result.other_vehicle_to_reference_geodesic = mref_veh_other;
  } else {
    // 获取参考车辆的边界多边形
    const Polygon reference_polygon = GetPolygon(GetBoundary(reference_vehicle_id));
    // 获取其他实体的边界多边形
    const Polygon other_polygon = GetPolygon(GetBoundary(other_actor_id));
    // 获取参考车辆的地理边界多边形
    const Polygon reference_geodesic_polygon = GetPolygon(GetGeodesicBoundary(reference_vehicle_id));
    //获取其他实体的地理边界多边形
    const Polygon other_geodesic_polygon = GetPolygon(GetGeodesicBoundary(other_actor_id));
    // 计算参考车辆到其他实体地理边界的距离
    const double reference_vehicle_to_other_geodesic = bg::distance(reference_polygon, other_geodesic_polygon);
    // 计算其他实体到参考车辆地理边界的距离
    const double other_vehicle_to_reference_geodesic = bg::distance(other_polygon, reference_geodesic_polygon);
    // 计算两实体地理边界之间的距离
    const auto inter_geodesic_distance = bg::distance(reference_geodesic_polygon, other_geodesic_polygon);
    // 计算两实体边界框之间的距离
    const auto inter_bbox_distance = bg::distance(reference_polygon, other_polygon);
    // 将计算结果存储到比较结果中
    comparision_result = {reference_vehicle_to_other_geodesic,
              other_vehicle_to_reference_geodesic,
              inter_geodesic_distance,
              inter_bbox_distance};
    // 将结果缓存
    geometry_cache.insert({actor_id_key, comparision_result});
  }

  return comparision_result; // 返回几何比较结果
}

std::pair<bool, float> CollisionStage::NegotiateCollision(const ActorId reference_vehicle_id,
                                                          const ActorId other_actor_id,
                                                          const uint64_t reference_junction_look_ahead_index) {
  // 方法的输出变量
  bool hazard = false;
  float available_distance_margin = std::numeric_limits<float>::infinity();

  const cg::Location reference_location = simulation_state.GetLocation(reference_vehicle_id);
  const cg::Location other_location = simulation_state.GetLocation(other_actor_id);

  // 自我和其他车辆的方向
  const cg::Vector3D reference_heading = simulation_state.GetHeading(reference_vehicle_id);
  // Vector from ego position to position of the other vehicle.
  cg::Vector3D reference_to_other = other_location - reference_location;
  reference_to_other = reference_to_other.MakeSafeUnitVector(EPSILON);

  // 其他车辆的方向
  const cg::Vector3D other_heading = simulation_state.GetHeading(other_actor_id);
  // 从其他车辆位置到自我位置的向量
  cg::Vector3D other_to_reference = reference_location - other_location;
  other_to_reference = other_to_reference.MakeSafeUnitVector(EPSILON);

  float reference_vehicle_length = simulation_state.GetDimensions(reference_vehicle_id).x * SQUARE_ROOT_OF_TWO;
  float other_vehicle_length = simulation_state.GetDimensions(other_actor_id).x * SQUARE_ROOT_OF_TWO;

  float inter_vehicle_distance = cg::Math::DistanceSquared(reference_location, other_location);
  float ego_bounding_box_extension = GetBoundingBoxExtention(reference_vehicle_id);
  float other_bounding_box_extension = GetBoundingBoxExtention(other_actor_id);
  // Calculate minimum distance between vehicle to consider collision negotiation.
  float inter_vehicle_length = reference_vehicle_length + other_vehicle_length;
  float ego_detection_range = SQUARE(ego_bounding_box_extension + inter_vehicle_length);
  float cross_detection_range = SQUARE(ego_bounding_box_extension + inter_vehicle_length + other_bounding_box_extension);

  // Conditions to consider collision negotiation.
  bool other_vehicle_in_ego_range = inter_vehicle_distance < ego_detection_range;
  bool other_vehicles_in_cross_detection_range = inter_vehicle_distance < cross_detection_range;
  float reference_heading_to_other_dot = cg::Math::Dot(reference_heading, reference_to_other);
  bool other_vehicle_in_front = reference_heading_to_other_dot > 0;
  const Buffer &reference_vehicle_buffer = buffer_map.at(reference_vehicle_id);
  SimpleWaypointPtr closest_point = reference_vehicle_buffer.front();
  bool ego_inside_junction = closest_point->CheckJunction();
  TrafficLightState reference_tl_state = simulation_state.GetTLS(reference_vehicle_id);
  bool ego_at_traffic_light = reference_tl_state.at_traffic_light;
  bool ego_stopped_by_light = reference_tl_state.tl_state != TLS::Green && reference_tl_state.tl_state != TLS::Off;
  SimpleWaypointPtr look_ahead_point = reference_vehicle_buffer.at(reference_junction_look_ahead_index);
  bool ego_at_junction_entrance = !closest_point->CheckJunction() && look_ahead_point->CheckJunction();

  // Conditions to consider collision negotiation.
  if (!(ego_at_junction_entrance && ego_at_traffic_light && ego_stopped_by_light)
      && ((ego_inside_junction && other_vehicles_in_cross_detection_range)
          || (!ego_inside_junction && other_vehicle_in_front && other_vehicle_in_ego_range))) {
    GeometryComparison geometry_comparison = GetGeometryBetweenActors(reference_vehicle_id, other_actor_id);

    // Conditions for collision negotiation.
    bool geodesic_path_bbox_touching = geometry_comparison.inter_geodesic_distance < OVERLAP_THRESHOLD;
    bool vehicle_bbox_touching = geometry_comparison.inter_bbox_distance < OVERLAP_THRESHOLD;
    bool ego_path_clear = geometry_comparison.other_vehicle_to_reference_geodesic > OVERLAP_THRESHOLD;
    bool other_path_clear = geometry_comparison.reference_vehicle_to_other_geodesic > OVERLAP_THRESHOLD;
    bool ego_path_priority = geometry_comparison.reference_vehicle_to_other_geodesic < geometry_comparison.other_vehicle_to_reference_geodesic;
    bool other_path_priority = geometry_comparison.reference_vehicle_to_other_geodesic > geometry_comparison.other_vehicle_to_reference_geodesic;
    bool ego_angular_priority = reference_heading_to_other_dot< cg::Math::Dot(other_heading, other_to_reference);

    // Whichever vehicle's path is farthest away from the other vehicle gets priority to move.
    bool lower_priority = !ego_path_priority && (other_path_priority || !ego_angular_priority);
    bool blocked_by_other_or_lower_priority = !ego_path_clear || (other_path_clear && lower_priority);
    bool yield_pre_crash = !vehicle_bbox_touching && blocked_by_other_or_lower_priority;
    bool yield_post_crash = vehicle_bbox_touching && !ego_angular_priority;

    if (geodesic_path_bbox_touching && (yield_pre_crash || yield_post_crash)) {

      hazard = true;

      const float reference_lead_distance = parameters.GetDistanceToLeadingVehicle(reference_vehicle_id);
      const float specific_distance_margin = std::max(reference_lead_distance, MIN_REFERENCE_DISTANCE);
      available_distance_margin = static_cast<float>(std::max(geometry_comparison.reference_vehicle_to_other_geodesic
                                                              - static_cast<double>(specific_distance_margin), 0.0));

      ///////////////////////////////////// Collision locking mechanism /////////////////////////////////
      // The idea is, when encountering a possible collision,
      // we should ensure that the bounding box extension doesn't decrease too fast and loose collision tracking.
      // This enables us to smoothly approach the lead vehicle.

      // When possible collision found, check if an entry for collision lock present.
      if (collision_locks.find(reference_vehicle_id) != collision_locks.end()) {
        CollisionLock &lock = collision_locks.at(reference_vehicle_id);
        // Check if the same vehicle is under lock.
        if (other_actor_id == lock.lead_vehicle_id) {
          // If the body of the lead vehicle is touching the reference vehicle bounding box.
          if (geometry_comparison.other_vehicle_to_reference_geodesic < OVERLAP_THRESHOLD) {
            // Distance between the bodies of the vehicles.
            lock.distance_to_lead_vehicle = geometry_comparison.inter_bbox_distance;
          } else {
            // Distance from reference vehicle body to other vehicle path polygon.
            lock.distance_to_lead_vehicle = geometry_comparison.reference_vehicle_to_other_geodesic;
          }
        } else {
          // If possible collision with a new vehicle, re-initialize with new lock entry.
          lock = {geometry_comparison.inter_bbox_distance, geometry_comparison.inter_bbox_distance, other_actor_id};
        }
      } else {
        // Insert and initialize lock entry if not present.
        collision_locks.insert({reference_vehicle_id,
                                {geometry_comparison.inter_bbox_distance,
                                 geometry_comparison.inter_bbox_distance,
                                 other_actor_id}});
      }
    }
  }

  // If no collision hazard detected, then flush collision lock held by the vehicle.
  if (!hazard && collision_locks.find(reference_vehicle_id) != collision_locks.end()) {
    collision_locks.erase(reference_vehicle_id);
  }

  return {hazard, available_distance_margin};
}

void CollisionStage::ClearCycleCache() {
  geodesic_boundary_map.clear();
  geometry_cache.clear();
}

} // namespace traffic_manager
} // namespace carla
