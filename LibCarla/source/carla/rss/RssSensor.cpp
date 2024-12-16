// Copyright (c) 2019-2020 Intel Corporation
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/rss/RssSensor.h"  // 包含RssSensor的头文件

#include <ad/map/access/Operation.hpp>  // 包含地图操作的头文件
#include <ad/rss/state/ProperResponse.hpp>  // 包含适当响应的状态头文件
#include <ad/rss/world/Velocity.hpp>  // 包含速度的头文件
#include <exception>  // 包含异常处理头文件
#include <fstream>  // 包含文件流头文件

#include "carla/Logging.h"  // 包含日志记录的头文件
#include "carla/client/Map.h"  // 包含地图客户端的头文件
#include "carla/client/Sensor.h"  // 包含传感器客户端的头文件
#include "carla/client/Vehicle.h"  // 包含车辆客户端的头文件
#include "carla/client/detail/Simulator.h"  // 包含模拟器的细节头文件
#include "carla/rss/RssCheck.h"  // 包含Rss检查的头文件
#include "carla/sensor/data/RssResponse.h"  // 包含Rss响应数据的头文件

namespace carla {  
namespace client {  

std::atomic_uint RssSensor::_global_map_initialization_counter_{0u};  // 全局地图初始化计数器

RssSensor::RssSensor(ActorInitializer init) : Sensor(std::move(init)), _on_tick_register_id(0u), _drop_route(false) {}  // 构造函数，初始化传感器

RssSensor::~RssSensor() {  // 析构函数
  if (IsListening()) {  // 如果正在监听
    Stop();  // 停止监听
  }
}

void RssSensor::RegisterActorConstellationCallback(ActorConstellationCallbackFunctionType callback) {  // 注册演员星座回调
  if (IsListening()) {  // 如果已经在监听
    log_error(GetDisplayId(), ": registering of the actor constellation callback has to be done before start listening. Register has no effect.");  // 记录错误
    return;  // 返回
  }
  _rss_actor_constellation_callback = callback;  // 设置回调
}

void RssSensor::Listen(CallbackFunctionType callback) {  // 开始监听
  if (IsListening()) {  // 如果已经在监听
    log_error(GetDisplayId(), ": already listening");  // 记录错误
    return;  // 返回
  }

  if (GetParent() == nullptr) {  // 如果父级为nullptr
    throw_exception(std::runtime_error(GetDisplayId() + ": not attached to actor"));  // 抛出异常
    return;  // 返回
  }

  auto vehicle = boost::dynamic_pointer_cast<carla::client::Vehicle>(GetParent());  // 动态转换父级为车辆
  if (vehicle == nullptr) {  // 如果转换失败
    throw_exception(std::runtime_error(GetDisplayId() + ": parent is not a vehicle"));  // 抛出异常
    return;  // 返回
  }

  float max_steer_angle_deg = 0.f;  // 最大转向角度初始化
  for (auto const &wheel : vehicle->GetPhysicsControl().GetWheels()) {  // 遍历车辆轮子
    max_steer_angle_deg = std::max(max_steer_angle_deg, wheel.max_steer_angle);  // 更新最大转向角度
  }
  auto max_steering_angle = max_steer_angle_deg * static_cast<float>(M_PI) / 180.0f;  // 转换为弧度

  auto map = GetWorld().GetMap();  // 获取地图
  DEBUG_ASSERT(map != nullptr);  // 确保地图不为nullptr
  std::string const open_drive_content = map->GetOpenDrive();  // 获取OpenDrive内容

  auto mapInitializationResult = ::ad::map::access::initFromOpenDriveContent(  // 初始化地图
      open_drive_content, 0.2, ::ad::map::intersection::IntersectionType::TrafficLight,  // 设置参数
      ::ad::map::landmark::TrafficLightType::LEFT_STRAIGHT_RED_YELLOW_GREEN);  // 设置交通灯类型

  if (!mapInitializationResult) {  // 如果地图初始化失败
    log_error(GetDisplayId(), ": Initialization of map failed");  // 记录错误
    return;  // 返回
  }

  _global_map_initialization_counter_++;  // 增加地图初始化计数器

  if (_rss_actor_constellation_callback == nullptr) {  // 如果没有回调
    _rss_check = std::make_shared<::carla::rss::RssCheck>(max_steering_angle);  // 创建Rss检查实例
  } else {  // 如果有回调
    _rss_check = std::make_shared<::carla::rss::RssCheck>(max_steering_angle, _rss_actor_constellation_callback, GetParent());  // 创建Rss检查实例，传入回调
  }

  auto self = boost::static_pointer_cast<RssSensor>(shared_from_this());  // 获取自身的共享指针

  _last_processed_frame=0u;  // 初始化最后处理帧
  log_debug(GetDisplayId(), ": subscribing to tick event");  // 记录调试信息
  _on_tick_register_id = GetEpisode().Lock()->RegisterOnTickEvent(  // 注册tick事件
      [ cb = std::move(callback), weak_self = WeakPtr<RssSensor>(self) ](const auto &snapshot) {  // Lambda函数处理事件
        auto self = weak_self.lock();  // 锁定弱指针
        if (self != nullptr) {  // 如果锁定成功
          self->TickRssSensor(snapshot.GetTimestamp(), cb);  // 调用TickRssSensor
        }
      });
}

void RssSensor::Stop() {  // 停止监听
  if (!IsListening()) {  // 如果不在监听
    log_error(GetDisplayId(), ": not listening at all");  // 记录错误
    return;  // 返回
  }
 
  log_debug(GetDisplayId(), ": unsubscribing from tick event");// 记录调试信息，表示取消订阅tick事件
  GetEpisode().Lock()->RemoveOnTickEvent(_on_tick_register_id);// 从事件管理器中移除tick事件的注册ID
  _on_tick_register_id = 0u;// 重置tick事件的注册ID

  if ( bool(_rss_check) ) {// 检查_rss_check对象是否有效
    _rss_check->GetLogger()->info("RssSensor stopping");// 记录信息，表示RssSensor正在停止
  }
  // 不要移除花括号，因为它们保护了lock_guard
  {
     // 确保在删除_rss_check对象时没有正在处理的任务
    const std::lock_guard<std::mutex> lock(_processing_lock);// 加锁以防止并发访问

    if ( bool(_rss_check) ) {// 检查_rss_check对象是否有效
      _rss_check->GetLogger()->info("RssSensor delete checker");// 记录信息，表示正在删除检查器
    }
    _rss_check.reset();// 重置_rss_check对象，释放资源
    auto const map_initialization_counter_value = _global_map_initialization_counter_--;// 减少全局地图初始化计数器
    if (map_initialization_counter_value == 0u) {// 如果计数器变为0
      // 最后一个停止监听的，清理地图
      ::ad::map::access::cleanup();// 清理地图资源
    }
  }
}

void RssSensor::SetLogLevel(const uint8_t &log_level) {// 设置日志级别的函数
  if (!bool(_rss_check)) {// 检查_rss_check对象是否有效
    log_error(GetDisplayId(), ": not yet listening. SetLogLevel has no effect.");// 记录错误，表示尚未开始监听
    return;// 直接返回
  }

  if (log_level < spdlog::level::n_levels) {// 检查日志级别是否在有效范围内
    _rss_check->SetLogLevel(spdlog::level::level_enum(log_level));// 设置_rss_check的日志级别
  }
}

void RssSensor::SetMapLogLevel(const uint8_t &map_log_level) {// 设置地图日志级别的函数
  if (!bool(_rss_check)) {// 检查_rss_check对象是否有效
    log_error(GetDisplayId(), ": not yet listening. SetMapLogLevel has no effect.");// 记录错误，表示尚未开始监听
    return;// 直接返回
  }

  if (map_log_level < spdlog::level::n_levels) {// 检查地图日志级别是否在有效范围内
    _rss_check->SetMapLogLevel(spdlog::level::level_enum(map_log_level));// 设置_rss_check的地图日志级别
  }
}

const ::ad::rss::world::RssDynamics &RssSensor::GetEgoVehicleDynamics() const {// 获取自车动态的函数
  static auto default_vehicle_dynamics = rss::RssCheck::GetDefaultVehicleDynamics();// 获取默认的车辆动态
  if (!bool(_rss_check)) {// 检查_rss_check对象是否有效
    log_error(GetDisplayId(), ": not yet listening. GetEgoVehicleDynamics has no effect.");// 记录错误，表示尚未开始监听
    return default_vehicle_dynamics;// 返回默认的车辆动态
  }

  if (bool(_rss_actor_constellation_callback)) {// 检查是否注册了参与者星座回调
    log_error(GetDisplayId(), ": Actor constellation callback registered. GetEgoVehicleDynamics has no effect.");// 记录错误，表示回调已注册
    return default_vehicle_dynamics;// 返回默认的车辆动态
  }

  return _rss_check->GetDefaultActorConstellationCallbackEgoVehicleDynamics();// 返回自车动态
}

void RssSensor::SetEgoVehicleDynamics(const ::ad::rss::world::RssDynamics &ego_dynamics) {// 设置自车动态的函数
  if (!bool(_rss_check)) {// 检查_rss_check对象是否有效
    log_error(GetDisplayId(), ": not yet listening. SetEgoVehicleDynamics has no effect.");// 记录错误，表示尚未开始监听
    return;// 直接返回
  }

  if (bool(_rss_actor_constellation_callback)) {// 检查是否注册了参与者星座回调
    log_error(GetDisplayId(), ": Actor constellation callback registered. SetEgoVehicleDynamics has no effect.");// 记录错误，表示回调已注册
    return;// 直接返回
  }

  _rss_check->SetDefaultActorConstellationCallbackEgoVehicleDynamics(ego_dynamics);// 设置自车动态
}

const ::ad::rss::world::RssDynamics &RssSensor::GetOtherVehicleDynamics() const {// 获取其他车辆动态的函数
  static auto default_vehicle_dynamics = rss::RssCheck::GetDefaultVehicleDynamics();// 获取默认的车辆动态
  if (!bool(_rss_check)) {// 检查_rss_check对象是否有效
    log_error(GetDisplayId(), ": not yet listening. GetOtherVehicleDynamics has no effect.");// 记录错误，表示尚未开始监听
    return default_vehicle_dynamics;// 返回默认的车辆动态
  }

  if (bool(_rss_actor_constellation_callback)) {// 检查是否注册了参与者星座回调
    log_error(GetDisplayId(), ": Actor constellation callback registered. GetOtherVehicleDynamics has no effect.");// 记录错误，表示回调已注册
    return default_vehicle_dynamics;// 返回默认的车辆动态
  }

  return _rss_check->GetDefaultActorConstellationCallbackOtherVehicleDynamics();// 返回其他车辆动态
}

void RssSensor::SetOtherVehicleDynamics(const ::ad::rss::world::RssDynamics &other_vehicle_dynamics) { // 设置其他车辆动态的函数
  if (!bool(_rss_check)) {// 检查_rss_check对象是否有效
    log_error(GetDisplayId(), ": not yet listening. SetOtherVehicleDynamics has no effect.");// 记录错误，表示尚未开始监听
    return;// 直接返回
  }

  if (bool(_rss_actor_constellation_callback)) {// 检查是否注册了参与者星座回调
    log_error(GetDisplayId(), ": Actor constellation callback registered. SetOtherVehicleDynamics has no effect.");// 记录错误，表示回调已注册
    return;// 直接返回
  }

  _rss_check->SetDefaultActorConstellationCallbackOtherVehicleDynamics(other_vehicle_dynamics);// 设置其他车辆动态
}

const ::ad::rss::world::RssDynamics &RssSensor::GetPedestrianDynamics() const {// 获取行人动态的函数
  static auto default_pedestrian_dynamics = rss::RssCheck::GetDefaultPedestrianDynamics();// 获取默认的行人动态
  if (!bool(_rss_check)) {// 检查_rss_check对象是否有效
    log_error(GetDisplayId(), ": not yet listening. GetPedestrianDynamics has no effect.");// 记录错误，表示尚未开始监听
    return default_pedestrian_dynamics;// 返回默认的行人动态
  }

  if (bool(_rss_actor_constellation_callback)) {// 如果已注册参与者星座回调
    log_error(GetDisplayId(), ": Actor constellation callback registered. GetPedestrianDynamics has no effect.");// 记录错误信息：参与者星座回调已注册，GetPedestrianDynamics没有效果
    return default_pedestrian_dynamics;// 返回默认的行人动态
  }

  return _rss_check->GetDefaultActorConstellationCallbackPedestrianDynamics();// 返回默认的参与者星座回调行人动态
}

void RssSensor::SetPedestrianDynamics(const ::ad::rss::world::RssDynamics &pedestrian_dynamics) {// 设置行人动态
  if (!bool(_rss_check)) {// 如果还没有开始监听
    log_error(GetDisplayId(), ": not yet listening. SetPedestrianDynamics has no effect."); // 记录错误信息：尚未监听，SetPedestrianDynamics没有效果
    return;// 返回
  }

  if (bool(_rss_actor_constellation_callback)) {// 如果已注册参与者星座回调
    log_error(GetDisplayId(), ": Actor constellation callback registered. SetPedestrianDynamics has no effect.");// 记录错误信息：参与者星座回调已注册，SetPedestrianDynamics没有效果
    return;// 返回
  }

  _rss_check->SetDefaultActorConstellationCallbackPedestrianDynamics(pedestrian_dynamics);// 设置默认的参与者星座回调行人动态
}

const ::carla::rss::RoadBoundariesMode &RssSensor::GetRoadBoundariesMode() const {// 获取道路边界模式
  if (!bool(_rss_check)) { // 如果还没有开始监听
    log_error(GetDisplayId(), ": not yet listening. GetRoadBoundariesMode has no effect.");// 记录错误信息：尚未监听，GetRoadBoundariesMode没有效果
    static auto default_road_boundaries_mode = rss::RssCheck::GetDefaultRoadBoundariesMode();// 获取默认道路边界模式
    return default_road_boundaries_mode;// 返回默认道路边界模式
  }

  return _rss_check->GetRoadBoundariesMode();// 返回道路边界模式
}

void RssSensor::SetRoadBoundariesMode(const ::carla::rss::RoadBoundariesMode &road_boundaries_mode) {// 设置道路边界模式
  if (!bool(_rss_check)) {// 如果还没有开始监听
    log_error(GetDisplayId(), ": not yet listening. SetRoadBoundariesMode has no effect.");// 记录错误信息：尚未监听，SetRoadBoundariesMode没有效果
    return;
  }

  _rss_check->SetRoadBoundariesMode(road_boundaries_mode);// 返回道路边界模式
}

void RssSensor::AppendRoutingTarget(const ::carla::geom::Transform &routing_target) {// 添加路由目标
  if (!bool(_rss_check)) {// 如果还没有开始监听
    log_error(GetDisplayId(), ": not yet listening. AppendRoutingTarget has no effect.");// 记录错误信息：尚未监听，AppendRoutingTarget没有效果
    return;// 返回
  }

  _rss_check->AppendRoutingTarget(routing_target);// 添加路由目标
}

const std::vector<::carla::geom::Transform> RssSensor::GetRoutingTargets() const {// 获取路由目标
  if (!bool(_rss_check)) {// 如果还没有开始监听
    log_error(GetDisplayId(), ": not yet listening. GetRoutingTargets has no effect.");// 记录错误信息：尚未监听，GetRoutingTargets没有效果
    return std::vector<::carla::geom::Transform>();// 返回空的路由目标向量
  }

  return _rss_check->GetRoutingTargets();// 返回路由目标
}

void RssSensor::ResetRoutingTargets() {// 重置路由目标
  if (!bool(_rss_check)) {// 如果还没有开始监听
    log_error(GetDisplayId(), ": not yet listening. ResetRoutingTargets has no effect.");// 记录错误信息：尚未监听，ResetRoutingTargets没有效果
    return;// 返回
  }

  _rss_check->ResetRoutingTargets();// 重置路由目标
}

void RssSensor::DropRoute() {// 放弃路由
     // 不要立即执行，因为这可能会完全破坏计算
    // 推迟到下一个传感器tick
  _drop_route = true;// 标记为放弃路由
}

void RssSensor::TickRssSensor(const client::Timestamp &timestamp, CallbackFunctionType callback) {// 处理Rss传感器的tick
  if (_processing_lock.try_lock()) {// 尝试锁定处理
    if (!bool(_rss_check)){// 如果还没有开始监听
      _processing_lock.unlock();// 解锁处理
      return;// 返回
    }
    if ((timestamp.frame < _last_processed_frame) && ((_last_processed_frame - timestamp.frame) < 0xffffffffu)) {// 如果当前帧小于最后处理的帧
      _processing_lock.unlock();// 解锁处理
      _rss_check->GetLogger()->warn("RssSensor[{}] outdated tick dropped, LastProcessed={}", timestamp.frame, _last_processed_frame);// 记录警告：过时的tick被丢弃
      return;// 返回
    }
    _last_processed_frame = timestamp.frame;// 更新最后处理的帧
    SharedPtr<carla::client::ActorList> actors = GetWorld().GetActors();// 获取世界中的参与者列表

    auto const settings = GetWorld().GetSettings();// 获取世界设置
    if ( settings.synchronous_mode ) {// 如果是同步模式
      _rss_check->GetLogger()->info("RssSensor[{}] sync-tick", timestamp.frame);// 记录信息：同步tick
      TickRssSensorThreadLocked(timestamp, actors, callback);// 处理同步tick
    }
    else {// 如果是异步模式
      // 存储future以防止future的析构函数阻塞等待
      _rss_check->GetLogger()->info("RssSensor[{}] async-tick", timestamp.frame);// 记录信息：异步tick
      _tick_future = std::async(&RssSensor::TickRssSensorThreadLocked, this, timestamp, actors, callback);// 异步处理tick
    }
  } else {// 如果处理锁定失败
    if (bool(_rss_check)){// 如果正在监听
      _rss_check->GetLogger()->info("RssSensor[{}] tick dropped", timestamp.frame);// 记录信息：tick被丢弃
    }
  }
}

void RssSensor::TickRssSensorThreadLocked(const client::Timestamp &timestamp,
                                          SharedPtr<carla::client::ActorList> actors, CallbackFunctionType callback) {
  try {
     // 记录检查开始时的时间（毫秒）
    double const time_since_epoch_check_start_ms =
        std::chrono::duration<double, std::milli>(std::chrono::system_clock::now().time_since_epoch()).count();

     // 如果需要丢弃路线
    if (_drop_route) {
      _drop_route = false;
      _rss_check->DropRoute();// 丢弃当前路线
    }

    // 检查所有参与者与自车的配对，并计算适当的响应
    ::ad::rss::state::ProperResponse response;// 存储响应
    ::ad::rss::state::RssStateSnapshot rss_state_snapshot;// RSS状态快照
    ::ad::rss::situation::SituationSnapshot situation_snapshot;// 情况快照
    ::ad::rss::world::WorldModel world_model;// 世界模型
    carla::rss::EgoDynamicsOnRoute ego_dynamics_on_route;// 自车在路线上的动态
     // 执行RSS检查
    auto const result = _rss_check->CheckObjects(timestamp, actors, GetParent(), response, rss_state_snapshot,
                                                             situation_snapshot, world_model, ego_dynamics_on_route);

     // 记录检查结束时的时间（毫秒）
    double const time_since_epoch_check_end_ms =
        std::chrono::duration<double, std::milli>(std::chrono::system_clock::now().time_since_epoch()).count();
     // 计算检查的时间差（毫秒）
    auto const delta_time_ms = time_since_epoch_check_end_ms - time_since_epoch_check_start_ms;
    // 记录日志，显示响应和时间差
    _rss_check->GetLogger()->debug("RssSensor[{}] response: S:{}->E:{} DeltaT:{}", timestamp.frame,
                                  time_since_epoch_check_start_ms, time_since_epoch_check_end_ms,
                                  delta_time_ms);
    // 将时间差添加到处理时间列表
    _rss_check_timings.push_back(delta_time_ms);
    // 保持处理时间列表的大小不超过10
    while (_rss_check_timings.size() > 10u) {
      _rss_check_timings.pop_front();
    }
    double agv_time=0.;// 平均时间初始化
     // 计算平均处理时间
    for (auto run_time: _rss_check_timings) {
      agv_time += run_time;
    }
    agv_time /= _rss_check_timings.size();// 计算平均时间
     // 记录平均处理时间的日志
    _rss_check->GetLogger()->info("RssSensor[{}] runtime {} avg {}", timestamp.frame, delta_time_ms, agv_time);
    _processing_lock.unlock();// 解锁处理

    // 调用回调函数，传递RSS响应数据
    callback(MakeShared<sensor::data::RssResponse>(timestamp.frame, timestamp.elapsed_seconds, GetTransform(), result,
                                                 response, rss_state_snapshot, situation_snapshot, world_model,
                                                   ego_dynamics_on_route));
  } catch (const std::exception &e) {
    // 捕捉到标准异常，记录错误日志
    _rss_check->GetLogger()->error("RssSensor[{}] tick exception", timestamp.frame);
    _processing_lock.unlock();// 解锁处理
  } catch (...) {
    // 捕捉到未知异常，记录错误日志
    _rss_check->GetLogger()->error("RssSensor[{}] tick exception", timestamp.frame);
    _processing_lock.unlock();// 解锁处理
  }
}

}  // namespace client
}  // namespace carla
