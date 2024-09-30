// Copyright (c) 2019-2020 Intel Corporation
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/rss/RssSensor.h" // 包含 RssSensor 头文件

#include <ad/map/access/Operation.hpp> // 包含地图访问操作的头文件
#include <ad/rss/state/ProperResponse.hpp> // 包含适当响应状态的头文件
#include <ad/rss/world/Velocity.hpp> // 包含速度相关的头文件
#include <exception> // 包含异常处理的头文件
#include <fstream> // 包含文件流处理的头文件

#include "carla/Logging.h" // 包含日志记录的头文件
#include "carla/client/Map.h" // 包含地图客户端的头文件
#include "carla/client/Sensor.h" // 包含传感器客户端的头文件
#include "carla/client/Vehicle.h" // 包含车辆客户端的头文件
#include "carla/client/detail/Simulator.h" // 包含模拟器的详细头文件
#include "carla/rss/RssCheck.h" // 包含 Rss 检查的头文件
#include "carla/sensor/data/RssResponse.h" // 包含 Rss 响应数据的头文件

namespace carla {
namespace client {

// 定义一个原子无符号整数，用于全局地图初始化计数
std::atomic_uint RssSensor::_global_map_initialization_counter_{0u};

// RssSensor 构造函数，接受一个 ActorInitializer 参数并初始化
RssSensor::RssSensor(ActorInitializer init) : Sensor(std::move(init)), _on_tick_register_id(0u), _drop_route(false) {}

// RssSensor 析构函数
RssSensor::~RssSensor() {
  // 确保没有正在处理的任务
  if (IsListening()) {
    Stop(); // 停止监听
  }
}

// 注册演员星座回调函数
void RssSensor::RegisterActorConstellationCallback(ActorConstellationCallbackFunctionType callback) {
  // 如果已经在监听，则无法注册回调
  if (IsListening()) {
    log_error(GetDisplayId(),
              ": registering of the actor constellation callback has to be done before start listening. Register has "
              "no effect."); // 记录错误日志
    return; // 提前返回
  }
  _rss_actor_constellation_callback = callback; // 保存回调函数
}

// 启动监听
void RssSensor::Listen(CallbackFunctionType callback) {
  // 如果已经在监听，则记录错误并返回
  if (IsListening()) {
    log_error(GetDisplayId(), ": already listening");
    return; // 提前返回
  }

  // 检查父对象是否为空
  if (GetParent() == nullptr) {
    throw_exception(std::runtime_error(GetDisplayId() + ": not attached to actor")); // 抛出异常
    return; // 提前返回
  }

  // 尝试将父对象转换为 Vehicle 类型
  auto vehicle = boost::dynamic_pointer_cast<carla::client::Vehicle>(GetParent());
  // 如果转换失败，则抛出异常
  if (vehicle == nullptr) {
    throw_exception(std::runtime_error(GetDisplayId() + ": parent is not a vehicle")); // 抛出异常
    return; // 提前返回
  }
  
  // 获取最大转向角度
  float max_steer_angle_deg = 0.f; // 初始化最大转向角度
  for (auto const &wheel : vehicle->GetPhysicsControl().GetWheels()) {
    max_steer_angle_deg = std::max(max_steer_angle_deg, wheel.max_steer_angle); // 更新最大转向角度
  }
  
  // 将角度转换为弧度
  auto max_steering_angle = max_steer_angle_deg * static_cast<float>(M_PI) / 180.0f;

  // 获取当前地图
  auto map = GetWorld().GetMap();
  DEBUG_ASSERT(map != nullptr); // 确保地图不为空
  std::string const open_drive_content = map->GetOpenDrive(); // 获取 OpenDrive 内容

  // 从 OpenDrive 内容初始化地图
  auto mapInitializationResult = ::ad::map::access::initFromOpenDriveContent(
      open_drive_content, 0.2, ::ad::map::intersection::IntersectionType::TrafficLight,
      ::ad::map::landmark::TrafficLightType::LEFT_STRAIGHT_RED_YELLOW_GREEN);

  // 检查地图初始化结果
  if (!mapInitializationResult) {
    log_error(GetDisplayId(), ": Initialization of map failed"); // 记录初始化失败的错误
    return; // 提前返回
  }

_global_map_initialization_counter_++; // 增加全局地图初始化计数器

// 检查是否有演员星座回调函数
if (_rss_actor_constellation_callback == nullptr) {
  // 如果没有回调函数，创建一个新的 RssCheck 对象
  _rss_check = std::make_shared<::carla::rss::RssCheck>(max_steering_angle);
} else {
  // 如果有回调函数，使用它来创建 RssCheck 对象
  _rss_check = std::make_shared<::carla::rss::RssCheck>(max_steering_angle, _rss_actor_constellation_callback, GetParent());
}

// 将当前对象转换为共享指针
auto self = boost::static_pointer_cast<RssSensor>(shared_from_this());

_last_processed_frame = 0u; // 初始化最后处理的帧编号
log_debug(GetDisplayId(), ": subscribing to tick event"); // 记录调试信息，表示正在订阅 tick 事件

// 注册一个 tick 事件的回调函数
_on_tick_register_id = GetEpisode().Lock()->RegisterOnTickEvent(
    [cb = std::move(callback), weak_self = WeakPtr<RssSensor>(self)](const auto &snapshot) {
      auto self = weak_self.lock(); // 尝试从弱指针锁定到共享指针
      if (self != nullptr) {
        // 如果成功锁定，则调用 TickRssSensor 函数
        self->TickRssSensor(snapshot.GetTimestamp(), cb);
      }
    });
}

// 停止监听函数
void RssSensor::Stop() {
  // 检查是否正在监听
  if (!IsListening()) {
    log_error(GetDisplayId(), ": not listening at all"); // 记录错误信息
    return; // 提前返回
  }

  log_debug(GetDisplayId(), ": unsubscribing from tick event"); // 记录调试信息，表示正在取消订阅 tick 事件
  GetEpisode().Lock()->RemoveOnTickEvent(_on_tick_register_id); // 移除 tick 事件的回调函数
  _on_tick_register_id = 0u; // 重置注册 ID

  // 检查 rss_check 是否有效
  if (bool(_rss_check)) {
    _rss_check->GetLogger()->info("RssSensor stopping"); // 记录停止信息
  }

  // 确保在删除 rss_check 对象时没有正在处理的任务
  {
    const std::lock_guard<std::mutex> lock(_processing_lock); // 使用锁保护临界区

    // 检查 rss_check 是否有效
    if (bool(_rss_check)) {
      _rss_check->GetLogger()->info("RssSensor delete checker"); // 记录删除信息
    }

    _rss_check.reset(); // 重置 rss_check 指针
    auto const map_initialization_counter_value = _global_map_initialization_counter_--; // 减少地图初始化计数器值
    
    // 如果计数器值为零，表示这是最后一个停止监听的对象
    if (map_initialization_counter_value == 0u) {
      // 清理地图
      ::ad::map::access::cleanup();
    }
  }
}

void RssSensor::SetLogLevel(const uint8_t &log_level) {
  // 检查是否已经开始监听
  if (!bool(_rss_check)) {
    log_error(GetDisplayId(), ": not yet listening. SetLogLevel has no effect."); // 记录错误信息
    return; // 提前返回
  }

  // 检查日志级别是否在有效范围内
  if (log_level < spdlog::level::n_levels) {
    _rss_check->SetLogLevel(spdlog::level::level_enum(log_level)); // 设置 RSS 检查的日志级别
  }
}

void RssSensor::SetMapLogLevel(const uint8_t &map_log_level) {
  // 检查是否已经开始监听
  if (!bool(_rss_check)) {
    log_error(GetDisplayId(), ": not yet listening. SetMapLogLevel has no effect."); // 记录错误信息
    return; // 提前返回
  }

  // 检查地图日志级别是否在有效范围内
  if (map_log_level < spdlog::level::n_levels) {
    _rss_check->SetMapLogLevel(spdlog::level::level_enum(map_log_level)); // 设置地图的日志级别
  }
}

const ::ad::rss::world::RssDynamics &RssSensor::GetEgoVehicleDynamics() const {
  // 创建一个默认的车辆动态对象
  static auto default_vehicle_dynamics = rss::RssCheck::GetDefaultVehicleDynamics();
  
  // 检查是否已经开始监听
  if (!bool(_rss_check)) {
    log_error(GetDisplayId(), ": not yet listening. GetEgoVehicleDynamics has no effect."); // 记录错误信息
    return default_vehicle_dynamics; // 返回默认车辆动态
  }

  // 检查是否注册了演员星座回调
  if (bool(_rss_actor_constellation_callback)) {
    log_error(GetDisplayId(), ": Actor constellation callback registered. GetEgoVehicleDynamics has no effect."); // 记录错误信息
    return default_vehicle_dynamics; // 返回默认车辆动态
  }

  // 返回当前的 Ego 车辆动态
  return _rss_check->GetDefaultActorConstellationCallbackEgoVehicleDynamics();
}

void RssSensor::SetEgoVehicleDynamics(const ::ad::rss::world::RssDynamics &ego_dynamics) {
  // 检查是否已经开始监听
  if (!bool(_rss_check)) {
    log_error(GetDisplayId(), ": not yet listening. SetEgoVehicleDynamics has no effect."); // 记录错误信息
    return; // 提前返回
  }

  // 检查是否注册了演员星座回调
  if (bool(_rss_actor_constellation_callback)) {
    log_error(GetDisplayId(), ": Actor constellation callback registered. SetEgoVehicleDynamics has no effect."); // 记录错误信息
    return; // 提前返回
  }

  // 在这里可以添加设置 Ego 车辆动态的代码
}

void RssSensor::SetEgoVehicleDynamics(const ::ad::rss::world::RssDynamics &ego_dynamics) {
  // 检查是否已经开始监听
  if (!bool(_rss_check)) {
    log_error(GetDisplayId(), ": not yet listening. SetEgoVehicleDynamics has no effect."); // 记录错误信息
    return; // 提前返回
  }

  // 检查是否注册了演员星座回调
  if (bool(_rss_actor_constellation_callback)) {
    log_error(GetDisplayId(), ": Actor constellation callback registered. SetEgoVehicleDynamics has no effect."); // 记录错误信息
    return; // 提前返回
  }

  // 设置 Ego 车辆动态
  _rss_check->SetDefaultActorConstellationCallbackEgoVehicleDynamics(ego_dynamics);
}

const ::ad::rss::world::RssDynamics &RssSensor::GetOtherVehicleDynamics() const {
  // 创建一个默认的其他车辆动态对象
  static auto default_vehicle_dynamics = rss::RssCheck::GetDefaultVehicleDynamics();
  
  // 检查是否已经开始监听
  if (!bool(_rss_check)) {
    log_error(GetDisplayId(), ": not yet listening. GetOtherVehicleDynamics has no effect."); // 记录错误信息
    return default_vehicle_dynamics; // 返回默认其他车辆动态
  }

  // 检查是否注册了演员星座回调
  if (bool(_rss_actor_constellation_callback)) {
    log_error(GetDisplayId(), ": Actor constellation callback registered. GetOtherVehicleDynamics has no effect."); // 记录错误信息
    return default_vehicle_dynamics; // 返回默认其他车辆动态
  }

  // 返回当前的其他车辆动态
  return _rss_check->GetDefaultActorConstellationCallbackOtherVehicleDynamics();
}

void RssSensor::SetOtherVehicleDynamics(const ::ad::rss::world::RssDynamics &other_vehicle_dynamics) {
  // 检查是否已经开始监听
  if (!bool(_rss_check)) {
    log_error(GetDisplayId(), ": not yet listening. SetOtherVehicleDynamics has no effect."); // 记录错误信息
    return; // 提前返回
  }

  // 检查是否注册了演员星座回调
  if (bool(_rss_actor_constellation_callback)) {
    log_error(GetDisplayId(), ": Actor constellation callback registered. SetOtherVehicleDynamics has no effect."); // 记录错误信息
    return; // 提前返回
  }

  // 设置其他车辆动态
  _rss_check->SetDefaultActorConstellationCallbackOtherVehicleDynamics(other_vehicle_dynamics);
}

const ::ad::rss::world::RssDynamics &RssSensor::GetPedestrianDynamics() const {
  // 创建一个默认的行人动态对象
  static auto default_pedestrian_dynamics = rss::RssCheck::GetDefaultPedestrianDynamics();
  
  // 检查是否已经开始监听
  if (!bool(_rss_check)) {
    log_error(GetDisplayId(), ": not yet listening. GetPedestrianDynamics has no effect."); // 记录错误信息
    return default_pedestrian_dynamics; // 返回默认行人动态
  }

  // 检查是否注册了演员星座回调
  if (bool(_rss_actor_constellation_callback)) {
    log_error(GetDisplayId(), ": Actor constellation callback registered. GetPedestrianDynamics has no effect."); // 记录错误信息
    return default_pedestrian_dynamics; // 返回默认行人动态
  }

  // 返回当前的行人动态
  return _rss_check->GetDefaultActorConstellationCallbackPedestrianDynamics();
}

void RssSensor::SetPedestrianDynamics(const ::ad::rss::world::RssDynamics &pedestrian_dynamics) {
  // 检查是否已经开始监听
  if (!bool(_rss_check)) {
    log_error(GetDisplayId(), ": not yet listening. SetPedestrianDynamics has no effect."); // 记录错误信息
    return; // 提前返回
  }

  // 检查是否注册了演员星座回调
  if (bool(_rss_actor_constellation_callback)) {
    log_error(GetDisplayId(), ": Actor constellation callback registered. SetPedestrianDynamics has no effect."); // 记录错误信息
    return; // 提前返回
  }

  // 设置行人动态
  _rss_check->SetDefaultActorConstellationCallbackPedestrianDynamics(pedestrian_dynamics);
}

const ::carla::rss::RoadBoundariesMode &RssSensor::GetRoadBoundariesMode() const {
  // 检查是否已经开始监听
  if (!bool(_rss_check)) {
    log_error(GetDisplayId(), ": not yet listening. GetRoadBoundariesMode has no effect."); // 记录错误信息
    static auto default_road_boundaries_mode = rss::RssCheck::GetDefaultRoadBoundariesMode(); // 获取默认的道路边界模式
    return default_road_boundaries_mode; // 返回默认道路边界模式
  }

  // 返回当前的道路边界模式
  return _rss_check->GetRoadBoundariesMode();
}

void RssSensor::SetRoadBoundariesMode(const ::carla::rss::RoadBoundariesMode &road_boundaries_mode) {
  // 检查是否已经开始监听
  if (!bool(_rss_check)) {
    log_error(GetDisplayId(), ": not yet listening. SetRoadBoundariesMode has no effect."); // 记录错误信息
    return; // 提前返回
  }

  // 设置道路边界模式
  _rss_check->SetRoadBoundariesMode(road_boundaries_mode);
}

void RssSensor::AppendRoutingTarget(const ::carla::geom::Transform &routing_target) {
  // 检查是否已经开始监听
  if (!bool(_rss_check)) {
    log_error(GetDisplayId(), ": not yet listening. AppendRoutingTarget has no effect."); // 记录错误信息
    return; // 提前返回
  }

  // 添加路由目标
  _rss_check->AppendRoutingTarget(routing_target);
}

const std::vector<::carla::geom::Transform> RssSensor::GetRoutingTargets() const {
  // 检查是否已经开始监听
  if (!bool(_rss_check)) {
    log_error(GetDisplayId(), ": not yet listening. GetRoutingTargets has no effect."); // 记录错误信息
    return std::vector<::carla::geom::Transform>(); // 返回空的路由目标向量
  }

  // 返回当前的路由目标
  return _rss_check->GetRoutingTargets();
}

void RssSensor::ResetRoutingTargets() {
  // 检查是否已经开始监听
  if (!bool(_rss_check)) {
    log_error(GetDisplayId(), ": not yet listening. ResetRoutingTargets has no effect."); // 记录错误信息
    return; // 提前返回
  }

  // 重置路由目标
  _rss_check->ResetRoutingTargets();
}

  _rss_check->ResetRoutingTargets();
}

void RssSensor::DropRoute() {
  // 不要立即执行这个操作，因为它可能会完全破坏计算
  // 将其推迟到下一个传感器周期
  _drop_route = true; // 设置标志以在下一个周期丢弃路由
}

void RssSensor::TickRssSensor(const client::Timestamp &timestamp, CallbackFunctionType callback) {
  // 尝试锁定处理，防止多个线程同时处理
  if (_processing_lock.try_lock()) {
    // 检查是否已经开始监听
    if (!bool(_rss_check)){
      _processing_lock.unlock(); // 解锁处理
      return; // 提前返回
    }
    // 检查时间戳是否过时
    if ((timestamp.frame < _last_processed_frame) && ((_last_processed_frame - timestamp.frame) < 0xffffffffu)) {
      _processing_lock.unlock(); // 解锁处理
      // 记录警告信息，表示丢弃过时的周期
      _rss_check->GetLogger()->warn("RssSensor[{}] outdated tick dropped, LastProcessed={}", timestamp.frame, _last_processed_frame);
      return; // 提前返回
    }
    
    // 更新最后处理的帧
    _last_processed_frame = timestamp.frame;
    // 获取当前世界中的演员列表
    SharedPtr<carla::client::ActorList> actors = GetWorld().GetActors();

    // 获取当前世界的设置
    auto const settings = GetWorld().GetSettings();
    // 检查是否为同步模式
    if (settings.synchronous_mode) {
      // 记录信息，表示进行同步周期处理
      _rss_check->GetLogger()->info("RssSensor[{}] sync-tick", timestamp.frame);
      // 调用处理函数，进行锁定的线程处理
      TickRssSensorThreadLocked(timestamp, actors, callback);
    } else {
      // 存储未来对象以防止其析构时被阻塞等待
      _rss_check->GetLogger()->info("RssSensor[{}] async-tick", timestamp.frame);
      // 异步调用处理函数，允许其他操作进行
      _tick_future = std::async(&RssSensor::TickRssSensorThreadLocked, this, timestamp, actors, callback);
    }
  } else {
    // 如果处理锁定失败，表示已有处理在进行中
    if (bool(_rss_check)){
      // 记录信息，表示当前周期被丢弃
      _rss_check->GetLogger()->info("RssSensor[{}] tick dropped", timestamp.frame);
    }
  }
}

void RssSensor::TickRssSensorThreadLocked(const client::Timestamp &timestamp,
                                          SharedPtr<carla::client::ActorList> actors, CallbackFunctionType callback) {
  try {
    // 记录检查开始时的时间（以毫秒为单位）
    double const time_since_epoch_check_start_ms =
        std::chrono::duration<double, std::milli>(std::chrono::system_clock::now().time_since_epoch()).count();

    // 检查是否需要丢弃路由
    if (_drop_route) {
      _drop_route = false; // 重置丢弃路由标志
      _rss_check->DropRoute(); // 执行丢弃路由操作
    }

    // 定义 RSS 状态快照及其它必要的变量
    ::ad::rss::state::ProperResponse response; // 正确响应
    ::ad::rss::state::RssStateSnapshot rss_state_snapshot; // RSS 状态快照
    ::ad::rss::situation::SituationSnapshot situation_snapshot; // 情境快照
    ::ad::rss::world::WorldModel world_model; // 世界模型
    carla::rss::EgoDynamicsOnRoute ego_dynamics_on_route; // 自我动态数据

    // 检查对象与自我之间的关系，计算相应的响应
    auto const result = _rss_check->CheckObjects(timestamp, actors, GetParent(), response, rss_state_snapshot,
                                                             situation_snapshot, world_model, ego_dynamics_on_route);

    // 记录检查结束时的时间（以毫秒为单位）
    double const time_since_epoch_check_end_ms =
        std::chrono::duration<double, std::milli>(std::chrono::system_clock::now().time_since_epoch()).count();
    
    // 计算检查所用的时间
    auto const delta_time_ms = time_since_epoch_check_end_ms - time_since_epoch_check_start_ms;

    // 记录调试信息，输出当前帧的响应时间等信息
    _rss_check->GetLogger()->debug("RssSensor[{}] response: S:{}->E:{} DeltaT:{}", timestamp.frame,
                                  time_since_epoch_check_start_ms, time_since_epoch_check_end_ms,
                                  delta_time_ms);

    // 将当前的运行时间记录到时间列表中
    _rss_check_timings.push_back(delta_time_ms);
    // 保持时间列表的大小不超过10
    while (_rss_check_timings.size() > 10u) {
      _rss_check_timings.pop_front(); // 移除最旧的记录
    }

    // 计算平均运行时间
    double agv_time = 0.;
    for (auto run_time: _rss_check_timings) {
      agv_time += run_time; // 累加所有运行时间
    }
    agv_time /= _rss_check_timings.size(); // 计算平均值

    // 记录信息，输出当前帧的运行时间和平均运行时间
    _rss_check->GetLogger()->info("RssSensor[{}] runtime {} avg {}", timestamp.frame, delta_time_ms, agv_time);
    
    // 解锁处理
    _processing_lock.unlock();

    // 调用回调函数，传递产生的 RSS 响应数据
    callback(MakeShared<sensor::data::RssResponse>(timestamp.frame, timestamp.elapsed_seconds, GetTransform(), result,
                                                 response, rss_state_snapshot, situation_snapshot, world_model,
                                                   ego_dynamics_on_route));
  } catch (const std::exception &e) {
    // 捕获标准异常并记录错误信息
    _rss_check->GetLogger()->error("RssSensor[{}] tick exception", timestamp.frame);
    _processing_lock.unlock(); // 解锁处理
  } catch (...) {
    // 捕获其它未知异常并记录错误信息
    _rss_check->GetLogger()->error("RssSensor[{}] tick exception", timestamp.frame);
    _processing_lock.unlock(); // 解锁处理
  }
}
