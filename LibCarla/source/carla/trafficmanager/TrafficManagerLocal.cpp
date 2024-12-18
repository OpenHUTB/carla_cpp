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
    run_traffic_manger.store(false);// 停止交通管理器运行
  if (parameters.GetSynchronousMode()) {
    step_begin_trigger.notify_one();
  }
 // 如果工作线程存在且可连接，等待线程结束
  if (worker_thread) {
    if (worker_thread->joinable()) {
      worker_thread->join(); // 等待工作线程完成
    }
    worker_thread.release();// 释放工作线程资源
  }
  // 清除所有车辆ID和注册车辆的状态
  vehicle_id_list.clear(); // 清空车辆ID列表
  registered_vehicles.Clear(); // 清除所有已注册的车辆
  registered_vehicles_state = -1; // 重置注册的车辆状态
  track_traffic.Clear();// 清除交通跟踪数据
   // 重置模拟相关的状态数据
  previous_update_instance = chr::system_clock::now();// 更新上次更新时间
  current_reserved_capacity = 0u; // 重置保留的容量
   // 清理各个阶段的数据
  simulation_state.Reset();// 重置模拟状态
  localization_stage.Reset(); // 重置定位阶段
  collision_stage.Reset(); // 重置碰撞检测阶段
  traffic_light_stage.Reset(); // 重置交通灯阶段
  motion_plan_stage.Reset(); // 重置运动规划阶段
  // 清空缓存数据
  buffer_map.clear();
  localization_frame.clear();
  collision_frame.clear();
  tl_frame.clear();
  control_frame.clear();
   // 恢复状态变量
  run_traffic_manger.store(true); // 恢复交通管理器的运行状态
  step_begin.store(false);// 重置步开始标志
  step_end.store(false); // 重置步结束标志
}
}

void TrafficManagerLocal::Release() {

  Stop();

  local_map.reset();// 释放本地图资源
}

void TrafficManagerLocal::Reset() { // 释放并重置资源
  Release();
  episode_proxy = episode_proxy.Lock()->GetCurrentEpisode();// 获取当前的剧集
  world = cc::World(episode_proxy);// 创建新的世界对象
  SetupLocalMap(); // 设置本地图
  Start(); // 启动交通管理器
}

void TrafficManagerLocal::RegisterVehicles(const std::vector<ActorPtr> &vehicle_list) { // 注册新车辆到交通管理器
  std::lock_guard<std::mutex> registration_lock(registration_mutex);// 锁定注册互斥量，避免并发问题
  registered_vehicles.Insert(vehicle_list); // 将车辆添加到已注册列表
}

void TrafficManagerLocal::UnregisterVehicles(const std::vector<ActorPtr> &actor_list) { // 注销车辆并移除其ID
  std::lock_guard<std::mutex> registration_lock(registration_mutex);// 锁定注册互斥量
  std::vector<ActorId> actor_id_list;// 存储人物ID
  for (auto &actor : actor_list) {
    alsm.RemoveActor(actor->GetId(), true); // 从系统中移除每个人物
  }
}

void TrafficManagerLocal::SetPercentageSpeedDifference(const ActorPtr &actor, const float percentage) { // 设置车辆与默认速度的差异百分比
  parameters.SetPercentageSpeedDifference(actor, percentage);
}

void TrafficManagerLocal::SetGlobalPercentageSpeedDifference(const float percentage) {// 设置全局速度差异百分比
  parameters.SetGlobalPercentageSpeedDifference(percentage);
}

void TrafficManagerLocal::SetLaneOffset(const ActorPtr &actor, const float offset) { // 设置车辆在车道中的偏移
  parameters.SetLaneOffset(actor, offset);
}

void TrafficManagerLocal::SetGlobalLaneOffset(const float offset) // 设置全局车道偏移{  parameters.SetGlobalLaneOffset(offset);
}

void TrafficManagerLocal::SetDesiredSpeed(const ActorPtr &actor, const float value) {// 设置车辆期望的行驶速度
  parameters.SetDesiredSpeed(actor, value);
}

/// 设置车辆灯光自动管理的方法
void TrafficManagerLocal::SetUpdateVehicleLights(const ActorPtr &actor, const bool do_update) {
  parameters.SetUpdateVehicleLights(actor, do_update);
}
// 设置车辆与其他车辆的碰撞检测
void TrafficManagerLocal::SetCollisionDetection(const ActorPtr &reference_actor, const ActorPtr &other_actor, const bool detect_collision) {
  parameters.SetCollisionDetection(reference_actor, other_actor, detect_collision);
}
// 强制车辆变道
void TrafficManagerLocal::SetForceLaneChange(const ActorPtr &actor, const bool direction) {
  parameters.SetForceLaneChange(actor, direction);
}
// 设置自动变道开关
void TrafficManagerLocal::SetAutoLaneChange(const ActorPtr &actor, const bool enable) {
  parameters.SetAutoLaneChange(actor, enable);
}
// 设置车辆与前车的距离
void TrafficManagerLocal::SetDistanceToLeadingVehicle(const ActorPtr &actor, const float distance) {
  parameters.SetDistanceToLeadingVehicle(actor, distance);
}
// 设置全局车辆与前车的距离
void TrafficManagerLocal::SetGlobalDistanceToLeadingVehicle(const float distance) {
  parameters.SetGlobalDistanceToLeadingVehicle(distance);
}
// 设置忽略行人的百分比
void TrafficManagerLocal::SetPercentageIgnoreWalkers(const ActorPtr &actor, const float perc) {
  parameters.SetPercentageIgnoreWalkers(actor, perc);
}
// 设置忽略其他车辆的百分比
void TrafficManagerLocal::SetPercentageIgnoreVehicles(const ActorPtr &actor, const float perc) {
  parameters.SetPercentageIgnoreVehicles(actor, perc);
}
// 设置车辆在绿灯时通过的百分比
void TrafficManagerLocal::SetPercentageRunningLight(const ActorPtr &actor, const float perc) {
  parameters.SetPercentageRunningLight(actor, perc);
}
// 设置车辆在红灯时的行为
void TrafficManagerLocal::SetPercentageRunningSign(const ActorPtr &actor, const float perc) {
  parameters.SetPercentageRunningSign(actor, perc);
}
// 设置车辆右侧行驶的百分比
void TrafficManagerLocal::SetKeepRightPercentage(const ActorPtr &actor, const float percentage) {
  parameters.SetKeepRightPercentage(actor, percentage);
}
// 设置车辆随机变道的百分比
void TrafficManagerLocal::SetRandomLeftLaneChangePercentage(const ActorPtr &actor, const float percentage) {
  parameters.SetRandomLeftLaneChangePercentage(actor, percentage);
}
// 设置车辆随机变道的百分比（右侧变道）
void TrafficManagerLocal::SetRandomRightLaneChangePercentage(const ActorPtr &actor, const float percentage) {
  parameters.SetRandomRightLaneChangePercentage(actor, percentage);
}
// 开关混合物理模式
void TrafficManagerLocal::SetHybridPhysicsMode(const bool mode_switch) {
  parameters.SetHybridPhysicsMode(mode_switch);
}
// 设置混合物理模式下的半径
void TrafficManagerLocal::SetHybridPhysicsRadius(const float radius) {
  parameters.SetHybridPhysicsRadius(radius);
}
// 设置是否启用OSM模式（Open Street Map）
void TrafficManagerLocal::SetOSMMode(const bool mode_switch) {
  parameters.SetOSMMode(mode_switch);
}
// 设置自定义路径给车辆
void TrafficManagerLocal::SetCustomPath(const ActorPtr &actor, const Path path, const bool empty_buffer) {
  parameters.SetCustomPath(actor, path, empty_buffer);
}
// 移除上传路径
void TrafficManagerLocal::RemoveUploadPath(const ActorId &actor_id, const bool remove_path) {
  parameters.RemoveUploadPath(actor_id, remove_path);
}
// 更新上传路径
void TrafficManagerLocal::UpdateUploadPath(const ActorId &actor_id, const Path path) {
  parameters.UpdateUploadPath(actor_id, path);
}
// 设置导入的路线
void TrafficManagerLocal::SetImportedRoute(const ActorPtr &actor, const Route route, const bool empty_buffer) {
  parameters.SetImportedRoute(actor, route, empty_buffer);
}
// 移除导入的路线
void TrafficManagerLocal::RemoveImportedRoute(const ActorId &actor_id, const bool remove_path) {
  parameters.RemoveImportedRoute(actor_id, remove_path);
}
// 更新导入的路线
void TrafficManagerLocal::UpdateImportedRoute(const ActorId &actor_id, const Route route) {
  parameters.UpdateImportedRoute(actor_id, route);
}
// 设置是否重生休眠车辆
void TrafficManagerLocal::SetRespawnDormantVehicles(const bool mode_switch) {
  parameters.SetRespawnDormantVehicles(mode_switch);
}
// 设置重生休眠车辆的边界
void TrafficManagerLocal::SetBoundariesRespawnDormantVehicles(const float lower_bound, const float upper_bound) {
  parameters.SetBoundariesRespawnDormantVehicles(lower_bound, upper_bound);
}
// 设置最大边界
void TrafficManagerLocal::SetMaxBoundaries(const float lower, const float upper) {
  parameters.SetMaxBoundaries(lower, upper);
}
// 获取车辆的下一动作
Action TrafficManagerLocal::GetNextAction(const ActorId &actor_id) {
  return localization_stage.ComputeNextAction(actor_id);
}// 获取车辆的动作缓冲区

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