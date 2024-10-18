
#include "boost/pointer_cast.hpp"

#include "carla/client/Actor.h" //导入 Actor 类
#include "carla/client/Vehicle.h" //导入 Vehicle (车辆)类
#include "carla/client/Walker.h" //导入 Walker (行人)类

#include "carla/trafficmanager/Constants.h" //导入交通管理中的常量定义
#include "carla/trafficmanager/LocalizationUtils.h" //导入定义相关的工具
#include "carla/trafficmanager/SimpleWaypoint.h" //导入 SimpleWaypoint 类

#include "carla/trafficmanager/ALSM.h" //导入 ALSM 类的声明

namespace carla {
namespace traffic_manager {

//ALSM 类的构造函数，用于初始化各个模块
ALSM::ALSM(
  AtomicActorSet &registered_vehicles, //已注册车辆的集合
  BufferMap &buffer_map, //存储交通流中路径缓存
  TrackTraffic &track_traffic, //用于追踪交通的模块
  std::vector<ActorId>& marked_for_removal, //标记即将移除的车辆 ID 列表
  const Parameters &parameters, //系统的参数配置
  const cc::World &world, //代表仿真世界
  const LocalMapPtr &local_map, //本地地图指针，用于交通管理
  SimulationState &simulation_state, //仿真状态
  LocalizationStage &localization_stage, //定位模块
  CollisionStage &collision_stage,//碰撞检测模块
  TrafficLightStage &traffic_light_stage, //交通信号灯控制模块
  MotionPlanStage &motion_plan_stage, //运动规划模块
  VehicleLightStage &vehicle_light_stage) //车辆灯光控制模块
  : registered_vehicles(registered_vehicles), //初始化已注册车辆
    buffer_map(buffer_map), //初始化路径缓存
    track_traffic(track_traffic), //初始化交通追踪器
    marked_for_removal(marked_for_removal), //初始化将移除的车辆列表
    parameters(parameters), //初始化系统参数
    world(world), //初始化仿真世界
    local_map(local_map), //初始化本地地图
    simulation_state(simulation_state), //初始化仿真状态
    localization_stage(localization_stage), //初始化定位模块
    collision_stage(collision_stage), //初始化碰撞检测模块
    traffic_light_stage(traffic_light_stage), //初始化交通信号灯控制模块
    motion_plan_stage(motion_plan_stage), //初始化运动规划模块
    vehicle_light_stage(vehicle_light_stage) {} //初始化车辆灯光控制模块

void ALSM::Update() {
  //获取是否启用混合物理模式参数
  bool hybrid_physics_mode = parameters.GetHybridPhysicsMode();
  //定义两个集合用于存储世界中的车辆 ID 和行人 ID
  std::set<ActorId> world_vehicle_ids;
  std::set<ActorId> world_pedestrian_ids;
  //存储待删除的未注册参与者的 ID 列表
  std::vector<ActorId> unregistered_list_to_be_deleted;

  current_timestamp = world.GetSnapshot().GetTimestamp(); //获取当前时间截
  ActorList world_actors = world.GetActors();//获取当前世界中的所有参与者列表

  // 找到已经销毁的参与者并进行清理
  const ALSM::DestroyeddActors destroyed_actors = IdentifyDestroyedActors(world_actors);
  
  //处理已注册的被销毁的参与者
  const ActorIdSet &destroyed_registered = destroyed_actors.first;
  for (const auto &deletion_id: destroyed_registered) {
    RemoveActor(deletion_id, true); //删除角色并标记为注册参与者
  }
  //处理未注册的被销毁参与者
  const ActorIdSet &destroyed_unregistered = destroyed_actors.second;
  for (auto deletion_id : destroyed_unregistered) {
    RemoveActor(deletion_id, false);
  }

  // 检查英雄参与者是否存活，如果英雄参与者已被销毁，则将其从英雄列表中移除
  if (hero_actors.size() != 0u) {
    ActorIdSet hero_actors_to_delete;
    //遍历英雄参与者，查看它们是否已被销毁
    for (auto &hero_actor_info: hero_actors) { 
    //如果在未注册销毁列表中找到英雄参与者，则标记其删除
      if (destroyed_unregistered.find(hero_actor_info.first) != destroyed_unregistered.end()) {
        hero_actors_to_delete.insert(hero_actor_info.first);
      }
      //如果在已注册销毁列表中找到英雄参与者，则标记其删除
      if (destroyed_registered.find(hero_actor_info.first) != destroyed_registered.end()) {
        hero_actors_to_delete.insert(hero_actor_info.first);
      }
    }
    
    //删除所有已标记的英雄参与者
    for (auto &deletion_id: hero_actors_to_delete) {
      hero_actors.erase(deletion_id);
    }
  }

  // 扫描并识别新的未注册参与者
  IdentifyNewActors(world_actors);

  // 更新所有已注册的车辆的动态状态和静态属性
  ALSM::IdleInfo max_idle_time = std::make_pair(0u, current_timestamp.elapsed_seconds);
  UpdateRegisteredActorsData(hybrid_physics_mode, max_idle_time);

  // 如果某辆已注册的车在某位置停留过久，则销毁该车辆
  if (IsVehicleStuck(max_idle_time.first)
      && (current_timestamp.elapsed_seconds - elapsed_last_actor_destruction) > DELTA_TIME_BETWEEN_DESTRUCTIONS
      && hero_actors.find(max_idle_time.first) == hero_actors.end()) {
    // 如果车辆被卡住，且它不是英雄参与者，并且距离上次销毁的时间超过了预设的时间间隔，则销毁该车辆。
    
	registered_vehicles.Destroy(max_idle_time.first); // 销毁长时间停滞不动的车辆
    RemoveActor(max_idle_time.first, true); //从已注册的参与者中移除该辆车
    elapsed_last_actor_destruction = current_timestamp.elapsed_seconds;//更新上一次销毁的时间
  }

  //分阶段销毁标记为移除的车辆
  if (parameters.GetOSMMode()) {
  	//如果系统处于 OSM 模式，遍历标记为移除的参与者列表
    for (const ActorId& actor_id: marked_for_removal) {
      registered_vehicles.Destroy(actor_id); //销毁这些标记为移除的车辆
      RemoveActor(actor_id, true); //从已注册参与者列表中移除
    }
    marked_for_removal.clear(); //清空标记为移除的参与者列表
  }

  // 更新未注册参与者的动态状态和静态属性
  UpdateUnregisteredActorsData();
}

//识别新的参与者
void ALSM::IdentifyNewActors(const ActorList &actor_list) {
    //遍历传入的参与者列表
  for (auto iter = actor_list->begin(); iter != actor_list->end(); ++iter) {
    ActorPtr actor = *iter; //获取当前的参与者对象
    ActorId actor_id = actor->GetId(); //获取当前参与者的唯一标识符（ID）
    // 识别新的英雄车辆
    if (actor->GetTypeId().front() == 'v') { //通过其类型（ID）判断当前参与者
     //如果没有任何已识别的英雄车辆，或者该参与者不在英雄参与者列表中
     if (hero_actors.size() == 0u || hero_actors.find(actor_id) == hero_actors.end()) {
      //遍历该参与者的所有属性
      for (auto&& attribute: actor->GetAttributes()) {
        //如果属性的 ID 是 "role_name"，并且其值是 "hero"
        if (attribute.GetId() == "role_name" && attribute.GetValue() == "hero") {
         //将英雄车辆插入到英雄列表中
          hero_actors.insert({actor_id, actor});
        }
      }
    }
  }
    //如果该参与者不在已注册车辆列表中，且不在未注册的参与者列表中
    if (!registered_vehicles.Contains(actor_id)
        && unregistered_actors.find(actor_id) == unregistered_actors.end()) {
      //将该参与者添加到未注册参与者中
      unregistered_actors.insert({actor_id, actor});
    }
  }
}

//识别已销毁的参与者
ALSM::DestroyeddActors ALSM::IdentifyDestroyedActors(const ActorList &actor_list) {

  ALSM::DestroyeddActors destroyed_actors; //用于存储销毁的参与者 ID
  ActorIdSet &deleted_registered = destroyed_actors.first; //存储已销毁的注册车辆的 ID
  ActorIdSet &deleted_unregistered = destroyed_actors.second; //存储已销毁的未注册参与者的 ID

  //构建当前帧中存在的参与者的哈希集合
  ActorIdSet current_actors;
  for  (auto iter = actor_list->begin(); iter != actor_list->end(); ++iter) {
    current_actors.insert((*iter)->GetId()); //将当前参与者的 ID 插入集合中
  }

  // 查找被销毁的已注册车辆
  std::vector<ActorId> registered_ids = registered_vehicles.GetIDList();
  for (const ActorId &actor_id : registered_ids) {
    //如果当前帧中不存在某个已注册车辆
    if (current_actors.find(actor_id) == current_actors.end()) {
        //将该车辆的 ID 加入到已销毁的注册车辆列表中
      deleted_registered.insert(actor_id);
    }
  }

  // 查找被销毁的未注册参与者
  for (const auto &actor_info: unregistered_actors) {
    const ActorId &actor_id = actor_info.first;
    //如果当前帧中不存在某个未注册的参与者，或者该参与者已经注册为车辆
     if (current_actors.find(actor_id) == current_actors.end()
         || registered_vehicles.Contains(actor_id)) {
      //将该参与者的 ID 加入到已销毁的未注册参与者列表中
      deleted_unregistered.insert(actor_id);
    }
  }
  //返回销毁的参与者列表
  return destroyed_actors;
}

void ALSM::UpdateRegisteredActorsData(const bool hybrid_physics_mode, ALSM::IdleInfo &max_idle_time) {

  std::vector<ActorPtr> vehicle_list = registered_vehicles.GetList();
  bool hero_actor_present = hero_actors.size() != 0u;
  float physics_radius = parameters.GetHybridPhysicsRadius();
  float physics_radius_square = SQUARE(physics_radius);
  bool is_respawn_vehicles = parameters.GetRespawnDormantVehicles();
  if (is_respawn_vehicles && !hero_actor_present) {
    track_traffic.SetHeroLocation(cg::Location(0,0,0));
  }
  // Update first the information regarding any hero vehicle.
  for (auto &hero_actor_info: hero_actors){
    if (is_respawn_vehicles) {
      track_traffic.SetHeroLocation(hero_actor_info.second->GetTransform().location);
    }
    UpdateData(hybrid_physics_mode, hero_actor_info.second, hero_actor_present, physics_radius_square);
  }
  // Update information for all other registered vehicles.
  for (const Actor &vehicle : vehicle_list) {
    ActorId actor_id = vehicle->GetId();
    if (hero_actors.find(actor_id) == hero_actors.end()) {
      UpdateData(hybrid_physics_mode, vehicle, hero_actor_present, physics_radius_square);
      UpdateIdleTime(max_idle_time, actor_id);
    }
  }
}

void ALSM::UpdateData(const bool hybrid_physics_mode, const Actor &vehicle,
                      const bool hero_actor_present, const float physics_radius_square) {

  ActorId actor_id = vehicle->GetId();
  cg::Transform vehicle_transform = vehicle->GetTransform();
  cg::Location vehicle_location = vehicle_transform.location;
  cg::Rotation vehicle_rotation = vehicle_transform.rotation;
  cg::Vector3D vehicle_velocity = vehicle->GetVelocity();
  bool state_entry_present = simulation_state.ContainsActor(actor_id);

  // Initializing idle times.
  if (idle_time.find(actor_id) == idle_time.end() && current_timestamp.elapsed_seconds != 0.0) {
    idle_time.insert({actor_id, current_timestamp.elapsed_seconds});
  }

  // Check if current actor is in range of hero actor and enable physics in hybrid mode.
  bool in_range_of_hero_actor = false;
  if (hero_actor_present && hybrid_physics_mode) {
    for (auto &hero_actor_info: hero_actors) {
      const ActorId &hero_actor_id =  hero_actor_info.first;
      if (simulation_state.ContainsActor(hero_actor_id)) {
        const cg::Location &hero_location = simulation_state.GetLocation(hero_actor_id);
        if (cg::Math::DistanceSquared(vehicle_location, hero_location) < physics_radius_square) {
          in_range_of_hero_actor = true;
          break;
        }
      }
    }
  }

  bool enable_physics = hybrid_physics_mode ? in_range_of_hero_actor : true;
  if (!has_physics_enabled.count(actor_id) || has_physics_enabled[actor_id] != enable_physics) {
    if (hero_actors.find(actor_id) == hero_actors.end()) {
      vehicle->SetSimulatePhysics(enable_physics);
      has_physics_enabled[actor_id] = enable_physics;
      if (enable_physics == true && state_entry_present) {
        vehicle->SetTargetVelocity(simulation_state.GetVelocity(actor_id));
      }
    }
  }

  // If physics are disabled, calculate velocity based on change in position.
  // Do not use 'enable_physics' as turning off the physics in this tick doesn't remove the velocity.
  // To avoid issues with other clients teleporting the actors, use the previous outpout location.
  if (state_entry_present && !simulation_state.IsPhysicsEnabled(actor_id)){
    cg::Location previous_location = simulation_state.GetLocation(actor_id);
    cg::Location previous_end_location = simulation_state.GetHybridEndLocation(actor_id);
    cg::Vector3D displacement = (previous_end_location - previous_location);
    vehicle_velocity = displacement * INV_HYBRID_DT;
  }

  // Updated kinematic state object.
  auto vehicle_ptr = boost::static_pointer_cast<cc::Vehicle>(vehicle);
  KinematicState kinematic_state{vehicle_location, vehicle_rotation,
                                  vehicle_velocity, vehicle_ptr->GetSpeedLimit(),
                                  enable_physics, vehicle->IsDormant(), cg::Location()};

  // Updated traffic light state object.
  TrafficLightState tl_state = {vehicle_ptr->GetTrafficLightState(), vehicle_ptr->IsAtTrafficLight()};

  // Update simulation state.
  if (state_entry_present) {
    simulation_state.UpdateKinematicState(actor_id, kinematic_state);
    simulation_state.UpdateTrafficLightState(actor_id, tl_state);
  }
  else {
    cg::Vector3D dimensions = vehicle_ptr->GetBoundingBox().extent;
    StaticAttributes attributes{ActorType::Vehicle, dimensions.x, dimensions.y, dimensions.z};

    simulation_state.AddActor(actor_id, kinematic_state, attributes, tl_state);
  }
}


void ALSM::UpdateUnregisteredActorsData() {
  for (auto &actor_info: unregistered_actors) {

    const ActorId actor_id = actor_info.first;
    const ActorPtr actor_ptr = actor_info.second;
    const std::string type_id = actor_ptr->GetTypeId();

    const cg::Transform actor_transform = actor_ptr->GetTransform();
    const cg::Location actor_location = actor_transform.location;
    const cg::Rotation actor_rotation = actor_transform.rotation;
    const cg::Vector3D actor_velocity = actor_ptr->GetVelocity();
    const bool actor_is_dormant = actor_ptr->IsDormant();
    KinematicState kinematic_state {actor_location, actor_rotation, actor_velocity, -1.0f, true, actor_is_dormant, cg::Location()};

    TrafficLightState tl_state;
    ActorType actor_type = ActorType::Any;
    cg::Vector3D dimensions;
    std::vector<SimpleWaypointPtr> nearest_waypoints;

    bool state_entry_not_present = !simulation_state.ContainsActor(actor_id);
    if (type_id.front() == 'v') {
      auto vehicle_ptr = boost::static_pointer_cast<cc::Vehicle>(actor_ptr);
      kinematic_state.speed_limit = vehicle_ptr->GetSpeedLimit();

      tl_state = {vehicle_ptr->GetTrafficLightState(), vehicle_ptr->IsAtTrafficLight()};

      if (state_entry_not_present) {
        dimensions = vehicle_ptr->GetBoundingBox().extent;
        actor_type = ActorType::Vehicle;
        StaticAttributes attributes {actor_type, dimensions.x, dimensions.y, dimensions.z};

        simulation_state.AddActor(actor_id, kinematic_state, attributes, tl_state);
      } else {
        simulation_state.UpdateKinematicState(actor_id, kinematic_state);
        simulation_state.UpdateTrafficLightState(actor_id, tl_state);
      }

      // Identify occupied waypoints.
      cg::Vector3D extent = vehicle_ptr->GetBoundingBox().extent;
      cg::Vector3D heading_vector = vehicle_ptr->GetTransform().GetForwardVector();
      std::vector<cg::Location> corners = {actor_location + cg::Location(extent.x * heading_vector),
                                           actor_location,
                                           actor_location + cg::Location(-extent.x * heading_vector)};
      for (cg::Location &vertex: corners) {
        SimpleWaypointPtr nearest_waypoint = local_map->GetWaypoint(vertex);
        nearest_waypoints.push_back(nearest_waypoint);
      }
    }
    else if (type_id.front() == 'w') {
      auto walker_ptr = boost::static_pointer_cast<cc::Walker>(actor_ptr);

      if (state_entry_not_present) {
        dimensions = walker_ptr->GetBoundingBox().extent;
        actor_type = ActorType::Pedestrian;
        StaticAttributes attributes {actor_type, dimensions.x, dimensions.y, dimensions.z};

        simulation_state.AddActor(actor_id, kinematic_state, attributes, tl_state);
      } else {
        simulation_state.UpdateKinematicState(actor_id, kinematic_state);
      }

      // Identify occupied waypoints.
      SimpleWaypointPtr nearest_waypoint = local_map->GetWaypoint(actor_location);
      nearest_waypoints.push_back(nearest_waypoint);
    }

    track_traffic.UpdateUnregisteredGridPosition(actor_id, nearest_waypoints);
  }
}

void ALSM::UpdateIdleTime(std::pair<ActorId, double>& max_idle_time, const ActorId& actor_id) {
  if (idle_time.find(actor_id) != idle_time.end()) {
    double &idle_duration = idle_time.at(actor_id);
    if (simulation_state.GetVelocity(actor_id).SquaredLength() > SQUARE(STOPPED_VELOCITY_THRESHOLD)) {
      idle_duration = current_timestamp.elapsed_seconds;
    }

    // Checking maximum idle time.
    if (max_idle_time.first == 0u || max_idle_time.second > idle_duration) {
      max_idle_time = std::make_pair(actor_id, idle_duration);
    }
  }
}

bool ALSM::IsVehicleStuck(const ActorId& actor_id) {
  if (idle_time.find(actor_id) != idle_time.end()) {
    double delta_idle_time = current_timestamp.elapsed_seconds - idle_time.at(actor_id);
    TrafficLightState tl_state = simulation_state.GetTLS(actor_id);
    if ((delta_idle_time >= RED_TL_BLOCKED_TIME_THRESHOLD)
    || (delta_idle_time >= BLOCKED_TIME_THRESHOLD && tl_state.tl_state != TLS::Red))
    {
      return true;
    }
  }
  return false;
}

void ALSM::RemoveActor(const ActorId actor_id, const bool registered_actor) {
  if (registered_actor) {
    registered_vehicles.Remove({actor_id});
    buffer_map.erase(actor_id);
    idle_time.erase(actor_id);
    localization_stage.RemoveActor(actor_id);
    collision_stage.RemoveActor(actor_id);
    traffic_light_stage.RemoveActor(actor_id);
    motion_plan_stage.RemoveActor(actor_id);
    vehicle_light_stage.RemoveActor(actor_id);
  }
  else {
    unregistered_actors.erase(actor_id);
    hero_actors.erase(actor_id);
  }

  track_traffic.DeleteActor(actor_id);
  simulation_state.RemoveActor(actor_id);
}

void ALSM::Reset() {
  unregistered_actors.clear();
  idle_time.clear();
  hero_actors.clear();
  elapsed_last_actor_destruction = 0.0;
  current_timestamp = world.GetSnapshot().GetTimestamp();
}

} // namespace traffic_manager
} // namespace carla
