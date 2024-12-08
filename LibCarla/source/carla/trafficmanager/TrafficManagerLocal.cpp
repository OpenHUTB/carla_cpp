// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <algorithm>

#include "carla/Logging.h"

#include "carla/client/detail/Simulator.h"

#include "carla/trafficmanager/TrafficManagerLocal.h"

namespace carla {
namespace traffic_manager {

using namespace constants::FrameMemory;
// TrafficManagerLocal 类负责管理交通系统中的交通控制，包括车道、交通信号灯等。
// 通过多线程和同步/异步操作模式与模拟器交互，管理车辆的行为和流量。

TrafficManagerLocal::TrafficManagerLocal(
  std::vector<float> longitudinal_PID_parameters,
  std::vector<float> longitudinal_highway_PID_parameters,
  std::vector<float> lateral_PID_parameters,
  std::vector<float> lateral_highway_PID_parameters,
  float perc_difference_from_limit,
  cc::detail::EpisodeProxy &episode_proxy,
  uint16_t &RPCportTM)

  : longitudinal_PID_parameters(longitudinal_PID_parameters),
    longitudinal_highway_PID_parameters(longitudinal_highway_PID_parameters),
    lateral_PID_parameters(lateral_PID_parameters),
    lateral_highway_PID_parameters(lateral_highway_PID_parameters),

    episode_proxy(episode_proxy),
    world(cc::World(episode_proxy)),
// 初始化各个阶段对象，负责不同的交通管理任务
    localization_stage(LocalizationStage(vehicle_id_list,
                                         buffer_map,
                                         simulation_state,
                                         track_traffic,
                                         local_map,
                                         parameters,
                                         marked_for_removal,
                                         localization_frame,
                                         random_device)),

    collision_stage(CollisionStage(vehicle_id_list,
                                   simulation_state,
                                   buffer_map,
                                   track_traffic,
                                   parameters,
                                   collision_frame,
                                   random_device)),

    traffic_light_stage(TrafficLightStage(vehicle_id_list,
                                          simulation_state,
                                          buffer_map,
                                          parameters,
                                          world,
                                          tl_frame,
                                          random_device)),

    motion_plan_stage(MotionPlanStage(vehicle_id_list,
                                      simulation_state,
                                      parameters,
                                      buffer_map,
                                      track_traffic,
                                      longitudinal_PID_parameters,
                                      longitudinal_highway_PID_parameters,
                                      lateral_PID_parameters,
                                      lateral_highway_PID_parameters,
                                      localization_frame,
                                      collision_frame,
                                      tl_frame,
                                      world,
                                      control_frame,
                                      random_device,
                                      local_map)),

    vehicle_light_stage(VehicleLightStage(vehicle_id_list,
                                          buffer_map,
                                          parameters,
                                          world,
                                          control_frame)),

    alsm(ALSM(registered_vehicles,
              buffer_map,
              track_traffic,
              marked_for_removal,
              parameters,
              world,
              local_map,
              simulation_state,
              localization_stage,
              collision_stage,
              traffic_light_stage,
              motion_plan_stage,
              vehicle_light_stage)),

    server(TrafficManagerServer(RPCportTM, static_cast<carla::traffic_manager::TrafficManagerBase *>(this))) {

  parameters.SetGlobalPercentageSpeedDifference(perc_difference_from_limit);

  registered_vehicles_state = -1;
 // 设置本地地图
  SetupLocalMap();
 // 启动交通管理器
  Start();
}

TrafficManagerLocal::~TrafficManagerLocal() {
 // 销毁交通管理器
  episode_proxy.Lock()->DestroyTrafficManager(server.port());
  Release();
}
// 设置本地地图
void TrafficManagerLocal::SetupLocalMap() {
  const carla::SharedPtr<const cc::Map> world_map = world.GetMap();
  local_map = std::make_shared<InMemoryMap>(world_map);
 // 获取缓存的地图文件
  auto files = episode_proxy.Lock()->GetRequiredFiles("TM");
  if (!files.empty()) {
    auto content = episode_proxy.Lock()->GetCacheFile(files[0], true);
    if (content.size() != 0) {
      local_map->Load(content);
    } else {
      log_warning("No InMemoryMap cache found. Setting up local map. This may take a while...");
      local_map->SetUp();
    }
  } else {
    log_warning("No InMemoryMap cache found. Setting up local map. This may take a while...");
    local_map->SetUp();
  }
}
// 启动交通管理器的工作线程
void TrafficManagerLocal::Start() {
  run_traffic_manger.store(true);
  worker_thread = std::make_unique<std::thread>(&TrafficManagerLocal::Run, this);
}
// 启动交通管理器的工作线程
void TrafficManagerLocal::Run() {

  localization_frame.reserve(INITIAL_SIZE);
  collision_frame.reserve(INITIAL_SIZE);
  tl_frame.reserve(INITIAL_SIZE);
  control_frame.reserve(INITIAL_SIZE);
  current_reserved_capacity = INITIAL_SIZE;

  size_t last_frame = 0;
  while (run_traffic_manger.load()) {

    bool synchronous_mode = parameters.GetSynchronousMode();
    bool hybrid_physics_mode = parameters.GetHybridPhysicsMode();
    parameters.SetMaxBoundaries(20.0f, episode_proxy.Lock()->GetEpisodeSettings().actor_active_distance);

       if (synchronous_mode) {   // 在同步模式下，等待外部触发以启动循环
      std::unique_lock<std::mutex> lock(step_execution_mutex);
      step_begin_trigger.wait(lock, [this]() {return step_begin.load() || !run_traffic_manger.load();});
      step_begin.store(false);
    }

    //   如果在异步混合模式下，经过的时间小于0.05秒，则跳过速度更新
    if (!synchronous_mode && hybrid_physics_mode) {
      TimePoint current_instance = chr::system_clock::now();
      chr::duration<float> elapsed_time = current_instance - previous_update_instance;
      chr::duration<float> time_to_wait = chr::duration<float>(HYBRID_MODE_DT) - elapsed_time;
      if (time_to_wait > chr::duration<float>(0.0f)) {
        std::this_thread::sleep_for(time_to_wait);
      }
      previous_update_instance = current_instance;
    }

    // 停止TM处理同一帧多次
    if (!synchronous_mode) {
      carla::client::Timestamp timestamp = world.GetSnapshot().GetTimestamp();
      if (timestamp.frame == last_frame) {
        continue;
      }
      last_frame = timestamp.frame;
    }

    std::unique_lock<std::mutex> registration_lock(registration_mutex);
    // 更新模拟状态、角色生命周期并执行必要的清理
    alsm.Update();

    // 基于已注册车辆数量变化的阶段间通信帧重新分配
    int current_registered_vehicles_state = registered_vehicles.GetState();
    unsigned long number_of_vehicles = vehicle_id_list.size();
    if (registered_vehicles_state != current_registered_vehicles_state || number_of_vehicles != registered_vehicles.Size()) {
      vehicle_id_list = registered_vehicles.GetIDList();
      number_of_vehicles = vehicle_id_list.size();

      // 根据需要，请预留更多空间
      uint64_t growth_factor = static_cast<uint64_t>(static_cast<float>(number_of_vehicles) * INV_GROWTH_STEP_SIZE);
      uint64_t new_frame_capacity = INITIAL_SIZE + GROWTH_STEP_SIZE * growth_factor;
      if (new_frame_capacity > current_reserved_capacity) {
        localization_frame.reserve(new_frame_capacity);
        collision_frame.reserve(new_frame_capacity);
        tl_frame.reserve(new_frame_capacity);
        control_frame.reserve(new_frame_capacity);
      }

      registered_vehicles_state = registered_vehicles.GetState();
    }

    // 重置当前周期的帧
    localization_frame.clear();
    localization_frame.resize(number_of_vehicles);
    collision_frame.clear();
    collision_frame.resize(number_of_vehicles);
    tl_frame.clear();
    tl_frame.resize(number_of_vehicles);
    control_frame.clear();
    // 为每辆车预留两个帧：一个用于ApplyVehicleControl命令
    // 以及一个用于可选的 SetVehicleLightState 命令
    control_frame.reserve(2 * number_of_vehicles);
    // 调整大小以容纳至少所有 ApplyVehicleControl 命令
    // 这将在运动规划阶段插入
    control_frame.resize(number_of_vehicles);

    // 运行核心操作阶段
    for (unsigned long index = 0u; index < vehicle_id_list.size(); ++index) {
      localization_stage.Update(index);
    }
    for (unsigned long index = 0u; index < vehicle_id_list.size(); ++index) {
      collision_stage.Update(index);
    }
    collision_stage.ClearCycleCache();
    vehicle_light_stage.UpdateWorldInfo();
    for (unsigned long index = 0u; index < vehicle_id_list.size(); ++index) {
      traffic_light_stage.Update(index);
      motion_plan_stage.Update(index);
      vehicle_light_stage.Update(index);
    }

    registration_lock.unlock();

    // 将当前周期的批处理命令发送给模拟器
    if (synchronous_mode) {
      episode_proxy.Lock()->ApplyBatchSync(control_frame, false);
      step_end.store(true);
      step_end_trigger.notify_one();
    } else {
      if (control_frame.size() > 0){
        episode_proxy.Lock()->ApplyBatchSync(control_frame, false);
      }
    }
  }
}
// 在同步模式下执行单步操作
bool TrafficManagerLocal::SynchronousTick() {
  if (parameters.GetSynchronousMode()) {
    step_begin.store(true);
    step_begin_trigger.notify_one();

    std::unique_lock<std::mutex> lock(step_execution_mutex);
    step_end_trigger.wait(lock, [this]() { return step_end.load(); });
    step_end.store(false);
  }
  return true;
}

void TrafficManagerLocal::Stop() {
// 停止交通管理器的工作线程并清理资源
  run_traffic_manger.store(false);void TrafficManagerLocal::Stop() {
// 停止交通管理器的工作线程并清理资源
  run_traffic_manger.store(false);
  if (parameters.GetSynchronousMode()) {
    step_begin_trigger.notify_one();// 如果是同步模式，开始步骤
  }

  if (worker_thread) {
    if (worker_thread->joinable()) {
      worker_thread->join();// 等待工作线程结束
    }
    worker_thread.release(); // 释放工作线程资源
  }

  vehicle_id_list.clear();// 清空车辆ID列表
  registered_vehicles.Clear();// 清空注册的车辆

  registered_vehicles_state = -1;// 重置注册的车辆状态
  track_traffic.Clear();// 清空交通跟踪数据
  previous_update_instance = chr::system_clock::now();// 记录最后更新时间
  current_reserved_capacity = 0u;// 重置当前保留的容量

  simulation_state.Reset(); // 重置模拟状态
  localization_stage.Reset();// 重置定位阶段
  collision_stage.Reset();// 重置定位阶段
  traffic_light_stage.Reset(); // 重置交通信号灯阶段
  motion_plan_stage.Reset();// 重置运动规划阶段

  buffer_map.clear();// 清空缓冲区地图

  localization_frame.clear(); // 清空定位帧数据
  collision_frame.clear();// 清空碰撞帧数据
  tl_frame.clear();// 清空交通灯帧数据
  control_frame.clear();  // 清空控制帧数据


  run_traffic_manger.store(true);// 恢复交通管理器运行
  step_begin.store(false);// 重置步骤开始标志
  step_end.store(false);// 重置步骤结束标志
}

void TrafficManagerLocal::Release() {
// 释放交通管理器资源
  Stop();
// 停止交通管理器
  local_map.reset();
}
 // 重置本地地图
void TrafficManagerLocal::Reset() {// 重置交通管理器
  Release(); // 先释放资源
  episode_proxy = episode_proxy.Lock()->GetCurrentEpisode(); // 获取当前的剧情
  world = cc::World(episode_proxy);// 使用当前剧情创建世界实例
  SetupLocalMap();// 设置本地地图
  Start();// 启动交通管理器
}


void TrafficManagerLocal::RegisterVehicles(const std::vector<ActorPtr> &vehicle_list) {// 注册车辆
  std::lock_guard<std::mutex> // 使用锁保护注册操作registration_lock(registration_mutex);
  registered_vehicles.Insert(vehicle_list);// 将车辆添加到注册列表
}

void TrafficManagerLocal::UnregisterVehicles(const std::vector<ActorPtr> &actor_list) {// 取消注册车辆
  std::lock_guard<std::mutex> registration_lock(registration_mutex);
  std::vector<ActorId> actor_id_list;
  for (auto &actor : actor_list) {
    alsm.RemoveActor(actor->GetId(), true);// 移除指定ID的车辆
  }
}

void TrafficManagerLocal::SetPercentageSpeedDifference(const ActorPtr &actor, const float percentage) {// 设置车辆速度差异百分比
  parameters.SetPercentageSpeedDifference(actor, percentage);
}

void TrafficManagerLocal::SetGlobalPercentageSpeedDifference(const float percentage) {// 设置全局速度差异百分比
  parameters.SetGlobalPercentageSpeedDifference(percentage);
}

void TrafficManagerLocal::SetLaneOffset(const ActorPtr &actor, const float offset) {// 设置车辆行驶偏移
  parameters.SetLaneOffset(actor, offset);
}

void TrafficManagerLocal::SetGlobalLaneOffset(const float offset) {// 设置全局行驶偏移
  parameters.SetGlobalLaneOffset(offset);
}

void TrafficManagerLocal::SetDesiredSpeed(const ActorPtr &actor, const float value) {// 设置车辆期望速度
  parameters.SetDesiredSpeed(actor, value);
}

/// 设置车辆灯光自动管理方法
void TrafficManagerLocal::SetUpdateVehicleLights(const ActorPtr &actor, const bool do_update) {
  parameters.SetUpdateVehicleLights(actor, do_update);
}// 设置是否自动更新车辆灯光

void TrafficManagerLocal::SetCollisionDetection(const ActorPtr &reference_actor, const ActorPtr &other_actor, const bool detect_collision) {// 设置碰撞检测
  parameters.SetCollisionDetection(reference_actor, other_actor, detect_collision);
}

void TrafficManagerLocal::SetForceLaneChange(const ActorPtr &actor, const bool direction) {// 设置强制变道
  parameters.SetForceLaneChange(actor, direction);
}

void TrafficManagerLocal::SetAutoLaneChange(const ActorPtr &actor, const bool enable) {// 设置自动变道
  parameters.SetAutoLaneChange(actor, enable);
}

void TrafficManagerLocal::SetDistanceToLeadingVehicle(const ActorPtr &actor, const float distance) {// 设置与前车的距离
  parameters.SetDistanceToLeadingVehicle(actor, distance);
}

void TrafficManagerLocal::SetGlobalDistanceToLeadingVehicle(const float distance) {// 设置全局与前车的距离
  parameters.SetGlobalDistanceToLeadingVehicle(distance);
}

void TrafficManagerLocal::SetPercentageIgnoreWalkers(const ActorPtr &actor, const float perc) {// 设置忽略行人的百分比
  parameters.SetPercentageIgnoreWalkers(actor, perc);
}

void TrafficManagerLocal::SetPercentageIgnoreVehicles(const ActorPtr &actor, const float perc) {// 设置忽略其他车辆的百分比
  parameters.SetPercentageIgnoreVehicles(actor, perc);
}

void TrafficManagerLocal::SetPercentageRunningLight(const ActorPtr &actor, const float perc) {// 设置闯红灯的百分比
  parameters.SetPercentageRunningLight(actor, perc);
}

void TrafficManagerLocal::SetPercentageRunningSign(const ActorPtr &actor, const float perc) {// 设置闯停牌的百分比
  parameters.SetPercentageRunningSign(actor, perc);
}

void TrafficManagerLocal::SetKeepRightPercentage(const ActorPtr &actor, const float percentage) {// 设置保持右侧行驶的百分比
  parameters.SetKeepRightPercentage(actor, percentage);
}

void TrafficManagerLocal::SetRandomLeftLaneChangePercentage(const ActorPtr &actor, const float percentage) {// 设置随机左侧变道的百分比
  parameters.SetRandomLeftLaneChangePercentage(actor, percentage);
}

void TrafficManagerLocal::SetRandomRightLaneChangePercentage(const ActorPtr &actor, const float percentage) {// 设置随机右侧变道的百分比
  parameters.SetRandomRightLaneChangePercentage(actor, percentage);
}

void TrafficManagerLocal::SetHybridPhysicsMode(const bool mode_switch) {// 设置混合物理模式
  parameters.SetHybridPhysicsMode(mode_switch);
}

void TrafficManagerLocal::SetHybridPhysicsRadius(const float radius) {// 设置混合物理模式的半径
  parameters.SetHybridPhysicsRadius(radius);
}

void TrafficManagerLocal::SetOSMMode(const bool mode_switch) {// 设置开放街图（OSM）模式
  parameters.SetOSMMode(mode_switch);
}

void TrafficManagerLocal::SetCustomPath(const ActorPtr &actor, const Path path, const bool empty_buffer) {// 设置车辆的自定义路径
  parameters.SetCustomPath(actor, path, empty_buffer);
}

void TrafficManagerLocal::RemoveUploadPath(const ActorId &actor_id, const bool remove_path) {// 移除上传的路径
  parameters.RemoveUploadPath(actor_id, remove_path);
}

void TrafficManagerLocal::UpdateUploadPath(const ActorId &actor_id, const Path path) {// 更新上传的路径
  parameters.UpdateUploadPath(actor_id, path);
}

void TrafficManagerLocal::SetImportedRoute(const ActorPtr &actor, const Route route, const bool empty_buffer) {// 设置车辆的导入路线
  parameters.SetImportedRoute(actor, route, empty_buffer);
}

void TrafficManagerLocal::RemoveImportedRoute(const ActorId &actor_id, const bool remove_path) {// 移除车辆的导入路线

  parameters.RemoveImportedRoute(actor_id, remove_path);
}

void TrafficManagerLocal::UpdateImportedRoute(const ActorId &actor_id, const Route route) {// 更新车辆的导入路线
  parameters.UpdateImportedRoute(actor_id, route);
}

void TrafficManagerLocal::SetRespawnDormantVehicles(const bool mode_switch) {// 设置是否重新生成休眠车辆
  parameters.SetRespawnDormantVehicles(mode_switch);
}

void TrafficManagerLocal::SetBoundariesRespawnDormantVehicles(const float lower_bound, const float upper_bound) {// 设置休眠车辆生成的边界范围
  parameters.SetBoundariesRespawnDormantVehicles(lower_bound, upper_bound);
}

void TrafficManagerLocal::SetMaxBoundaries(const float lower, const float upper) {// 设置车辆的最大边界
  parameters.SetMaxBoundaries(lower, upper);
}

Action TrafficManagerLocal::GetNextAction(const ActorId &actor_id) {// 获取指定车辆的下一步动作
  return localization_stage.ComputeNextAction(actor_id);
}

ActionBuffer TrafficManagerLocal::GetActionBuffer(const ActorId &actor_id) {// 获取指定车辆的动作缓冲区
  return localization_stage.ComputeActionBuffer(actor_id);
}

bool TrafficManagerLocal::CheckAllFrozen(TLGroup tl_to_freeze) {// 检查交通信号灯是否全部被冻结
  for (auto &elem : tl_to_freeze) {
    if (!elem->IsFrozen() || elem->GetState() != TLS::Red) {
      return false;// 如果有任何交通信号灯没有冻结或不是红灯，返回
    }
  }
  return true;
}

void TrafficManagerLocal::SetSynchronousMode(bool mode) {
  const bool previous_mode = parameters.GetSynchronousMode();
  parameters.SetSynchronousMode(mode);
  if (previous_mode && !mode) {
    step_begin.store(true);
    step_begin_trigger.notify_one();
  }
}

void TrafficManagerLocal::SetSynchronousModeTimeOutInMiliSecond(double time) {
  parameters.SetSynchronousModeTimeOutInMiliSecond(time);
}

carla::client::detail::EpisodeProxy &TrafficManagerLocal::GetEpisodeProxy() {
  return episode_proxy;
}

std::vector<ActorId> TrafficManagerLocal::GetRegisteredVehiclesIDs() {
  return registered_vehicles.GetIDList();
}

void TrafficManagerLocal::SetRandomDeviceSeed(const uint64_t _seed) {
  seed = _seed;
  random_device = RandomGenerator(seed);
  world.ResetAllTrafficLights();
}

} // namespace traffic_manager
} // namespace carla

  if (parameters.GetSynchronousMode()) {
    step_begin_trigger.notify_one();
  }

  if (worker_thread) {
    if (worker_thread->joinable()) {
      worker_thread->join();
    }
    worker_thread.release();
  }

  vehicle_id_list.clear();
  registered_vehicles.Clear();
  registered_vehicles_state = -1;
  track_traffic.Clear();
  previous_update_instance = chr::system_clock::now();
  current_reserved_capacity = 0u;

  simulation_state.Reset();
  localization_stage.Reset();
  collision_stage.Reset();
  traffic_light_stage.Reset();
  motion_plan_stage.Reset();

  buffer_map.clear();
  localization_frame.clear();
  collision_frame.clear();
  tl_frame.clear();
  control_frame.clear();

  run_traffic_manger.store(true);
  step_begin.store(false);
  step_end.store(false);
}

void TrafficManagerLocal::Release() {

  Stop();

  local_map.reset();
}

void TrafficManagerLocal::Reset() {
  Release();
  episode_proxy = episode_proxy.Lock()->GetCurrentEpisode();
  world = cc::World(episode_proxy);
  SetupLocalMap();
  Start();
}

void TrafficManagerLocal::RegisterVehicles(const std::vector<ActorPtr> &vehicle_list) {
  std::lock_guard<std::mutex> registration_lock(registration_mutex);
  registered_vehicles.Insert(vehicle_list);
}

void TrafficManagerLocal::UnregisterVehicles(const std::vector<ActorPtr> &actor_list) {
  std::lock_guard<std::mutex> registration_lock(registration_mutex);
  std::vector<ActorId> actor_id_list;
  for (auto &actor : actor_list) {
    alsm.RemoveActor(actor->GetId(), true);
  }
}

void TrafficManagerLocal::SetPercentageSpeedDifference(const ActorPtr &actor, const float percentage) {
  parameters.SetPercentageSpeedDifference(actor, percentage);
}

void TrafficManagerLocal::SetGlobalPercentageSpeedDifference(const float percentage) {
  parameters.SetGlobalPercentageSpeedDifference(percentage);
}

void TrafficManagerLocal::SetLaneOffset(const ActorPtr &actor, const float offset) {
  parameters.SetLaneOffset(actor, offset);
}

void TrafficManagerLocal::SetGlobalLaneOffset(const float offset) {
  parameters.SetGlobalLaneOffset(offset);
}

void TrafficManagerLocal::SetDesiredSpeed(const ActorPtr &actor, const float value) {
  parameters.SetDesiredSpeed(actor, value);
}

/// 设置车辆灯光自动管理的方法
void TrafficManagerLocal::SetUpdateVehicleLights(const ActorPtr &actor, const bool do_update) {
  parameters.SetUpdateVehicleLights(actor, do_update);
}

void TrafficManagerLocal::SetCollisionDetection(const ActorPtr &reference_actor, const ActorPtr &other_actor, const bool detect_collision) {
  parameters.SetCollisionDetection(reference_actor, other_actor, detect_collision);
}

void TrafficManagerLocal::SetForceLaneChange(const ActorPtr &actor, const bool direction) {
  parameters.SetForceLaneChange(actor, direction);
}

void TrafficManagerLocal::SetAutoLaneChange(const ActorPtr &actor, const bool enable) {
  parameters.SetAutoLaneChange(actor, enable);
}

void TrafficManagerLocal::SetDistanceToLeadingVehicle(const ActorPtr &actor, const float distance) {
  parameters.SetDistanceToLeadingVehicle(actor, distance);
}

void TrafficManagerLocal::SetGlobalDistanceToLeadingVehicle(const float distance) {
  parameters.SetGlobalDistanceToLeadingVehicle(distance);
}

void TrafficManagerLocal::SetPercentageIgnoreWalkers(const ActorPtr &actor, const float perc) {
  parameters.SetPercentageIgnoreWalkers(actor, perc);
}

void TrafficManagerLocal::SetPercentageIgnoreVehicles(const ActorPtr &actor, const float perc) {
  parameters.SetPercentageIgnoreVehicles(actor, perc);
}

void TrafficManagerLocal::SetPercentageRunningLight(const ActorPtr &actor, const float perc) {
  parameters.SetPercentageRunningLight(actor, perc);
}

void TrafficManagerLocal::SetPercentageRunningSign(const ActorPtr &actor, const float perc) {
  parameters.SetPercentageRunningSign(actor, perc);
}

void TrafficManagerLocal::SetKeepRightPercentage(const ActorPtr &actor, const float percentage) {
  parameters.SetKeepRightPercentage(actor, percentage);
}

void TrafficManagerLocal::SetRandomLeftLaneChangePercentage(const ActorPtr &actor, const float percentage) {
  parameters.SetRandomLeftLaneChangePercentage(actor, percentage);
}

void TrafficManagerLocal::SetRandomRightLaneChangePercentage(const ActorPtr &actor, const float percentage) {
  parameters.SetRandomRightLaneChangePercentage(actor, percentage);
}

void TrafficManagerLocal::SetHybridPhysicsMode(const bool mode_switch) {
  parameters.SetHybridPhysicsMode(mode_switch);
}

void TrafficManagerLocal::SetHybridPhysicsRadius(const float radius) {
  parameters.SetHybridPhysicsRadius(radius);
}

void TrafficManagerLocal::SetOSMMode(const bool mode_switch) {
  parameters.SetOSMMode(mode_switch);
}

void TrafficManagerLocal::SetCustomPath(const ActorPtr &actor, const Path path, const bool empty_buffer) {
  parameters.SetCustomPath(actor, path, empty_buffer);
}

void TrafficManagerLocal::RemoveUploadPath(const ActorId &actor_id, const bool remove_path) {
  parameters.RemoveUploadPath(actor_id, remove_path);
}

void TrafficManagerLocal::UpdateUploadPath(const ActorId &actor_id, const Path path) {
  parameters.UpdateUploadPath(actor_id, path);
}

void TrafficManagerLocal::SetImportedRoute(const ActorPtr &actor, const Route route, const bool empty_buffer) {
  parameters.SetImportedRoute(actor, route, empty_buffer);
}

void TrafficManagerLocal::RemoveImportedRoute(const ActorId &actor_id, const bool remove_path) {
  parameters.RemoveImportedRoute(actor_id, remove_path);
}

void TrafficManagerLocal::UpdateImportedRoute(const ActorId &actor_id, const Route route) {
  parameters.UpdateImportedRoute(actor_id, route);
}

void TrafficManagerLocal::SetRespawnDormantVehicles(const bool mode_switch) {
  parameters.SetRespawnDormantVehicles(mode_switch);
}

void TrafficManagerLocal::SetBoundariesRespawnDormantVehicles(const float lower_bound, const float upper_bound) {
  parameters.SetBoundariesRespawnDormantVehicles(lower_bound, upper_bound);
}

void TrafficManagerLocal::SetMaxBoundaries(const float lower, const float upper) {
  parameters.SetMaxBoundaries(lower, upper);
}

Action TrafficManagerLocal::GetNextAction(const ActorId &actor_id) {
  return localization_stage.ComputeNextAction(actor_id);
}

ActionBuffer TrafficManagerLocal::GetActionBuffer(const ActorId &actor_id) {
  return localization_stage.ComputeActionBuffer(actor_id);
}

bool TrafficManagerLocal::CheckAllFrozen(TLGroup tl_to_freeze) {
  for (auto &elem : tl_to_freeze) {
    if (!elem->IsFrozen() || elem->GetState() != TLS::Red) {
      return false;
    }
  }
  return true;
}

void TrafficManagerLocal::SetSynchronousMode(bool mode) {
  const bool previous_mode = parameters.GetSynchronousMode();
  parameters.SetSynchronousMode(mode);
  if (previous_mode && !mode) {
    step_begin.store(true);
    step_begin_trigger.notify_one();
  }
}

void TrafficManagerLocal::SetSynchronousModeTimeOutInMiliSecond(double time) {
  parameters.SetSynchronousModeTimeOutInMiliSecond(time);
}

carla::client::detail::EpisodeProxy &TrafficManagerLocal::GetEpisodeProxy() {
  return episode_proxy;
}

std::vector<ActorId> TrafficManagerLocal::GetRegisteredVehiclesIDs() {
  return registered_vehicles.GetIDList();
}

void TrafficManagerLocal::SetRandomDeviceSeed(const uint64_t _seed) {
  seed = _seed;
  random_device = RandomGenerator(seed);
  world.ResetAllTrafficLights();
}

} // namespace traffic_manager
} // namespace carla
