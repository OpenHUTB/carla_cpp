// Copyright (c) 2019-2020 Intel Corporation
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <atomic>
#include <future>
#include <list>
#include <memory>
#include <mutex>
#include <vector>
#include "carla/client/Sensor.h"

namespace ad {
// 命名空间ad的定义
namespace rss {
// 命名空间rss的定义，位于ad命名空间内
namespace world {
// 命名空间world的定义，位于rss命名空间内

/// forward declaration of the RssDynamics struct
struct RssDynamics;

}  // namespace world
}  // namespace rss
}  // namespace ad

namespace carla {
// carla命名空间的定义开始
namespace rss {
// rss命名空间的定义，位于carla命名空间内
/// 前向声明 RoadBoundariesMode 枚举类
enum class RoadBoundariesMode;
/// 前向声明 RssCheck 类
class RssCheck;
/// 前向声明 ActorConstellationResult 结构体
struct ActorConstellationResult;
/// 前向声明 ActorConstellationData 结构体
struct ActorConstellationData;
}  // namespace rss

namespace client {

/// 实现carla::client::Sensor接口的RSS传感器类
/// 这个类是RssCheck类的代理
class RssSensor : public Sensor {
public:
// 使用基类Sensor的构造函数
  using Sensor::Sensor;
// 定义一个函数类型，用于Actor星座（即周围物体的配置或分布）回调  
  // 该函数接收一个指向carla::rss::ActorConstellationData的智能指针，并返回一个carla::rss::ActorConstellationResult
  using ActorConstellationCallbackFunctionType =
      std::function<::carla::rss::ActorConstellationResult(carla::SharedPtr<::carla::rss::ActorConstellationData>)>;

  /// @brief 构造函数  
  // 显式地使用ActorInitializer对象作为参数进行初始化 /// @brief constructor
  explicit RssSensor(ActorInitializer init);

  /// @brief 析构函数  
  // 用于RssSensor对象的清理工作  
  ~RssSensor();

  /// 为每个待处理的测量中的每个行为体注册一个回调函数，以便决定RSS传感器相对于自我车辆与行为体星座的操作。
  void RegisterActorConstellationCallback(ActorConstellationCallbackFunctionType callback);

  /// 注册一个回调函数，每次接收到新的测量数据时都会执行该函数。
///
/// @警告：如果对一个已经在监听的传感器调用此函数，则会窃取之前设置的回调函数的数据流。请注意，
/// 模拟器中的多个Sensor实例（甚至在不同进程中）可能指向同一个传感器。
  void Listen(CallbackFunctionType callback) override;

  /// 停止监听新的测量数据。
/// 注意：为了能够等待非活动状态的回调函数，全局解释器锁（GIL）必须被解锁。
  void Stop() override;

  /// 返回此Sensor实例当前是否正在监听模拟器中的相关传感器
  bool IsListening() const override {
    return _on_tick_register_id != 0u;
  }

  /// @brief 设置当前的日志级别
  void SetLogLevel(const uint8_t &log_level);

  /// @brief 设置当前的地图日志级别
  void SetMapLogLevel(const uint8_t &map_log_level);

  /// 返回当前自车（ego vehicle）所使用的动力学参数（@see also  
  /// RssCheck::GetEgoVehicleDynamics() 以获取更多信息）
  const ::ad::rss::world::RssDynamics &GetEgoVehicleDynamics() const;
  /// @brief 设置自车（ego vehicle）将要使用的动力学参数（@see  
  /// also RssCheck::SetEgoVehicleDynamics() 以获取更多信息） 
  void SetEgoVehicleDynamics(const ::ad::rss::world::RssDynamics &ego_dynamics);

  /// 返回当前其他车辆所使用的动力学参数（@see RssCheck::GetOtherVehicleDynamics() 获取更多信息）
  const ::ad::rss::world::RssDynamics &GetOtherVehicleDynamics() const;
  /// @brief 设置其他车辆所使用的动力学参数（@see RssCheck::SetOtherVehicleDynamics() 获取更多信息）
  void SetOtherVehicleDynamics(const ::ad::rss::world::RssDynamics &other_vehicle_dynamics);

  /// 返回当前行人所使用的动力学参数（@see RssCheck::GetPedestrianDynamics() 获取更多信息）
  const ::ad::rss::world::RssDynamics &GetPedestrianDynamics() const;
  /// @brief 设置行人所使用的动力学参数（@see RssCheck::SetPedestrianDynamics() 获取更多信息）
  void SetPedestrianDynamics(const ::ad::rss::world::RssDynamics &pedestrian_dynamics);

  /// @returns 当前用于遵守道路边界的模式 (@see 同样参见 RssCheck::GetRoadBoundariesMode())
  const ::carla::rss::RoadBoundariesMode &GetRoadBoundariesMode() const;
  /// @brief 设置当前用于遵守道路边界的模式 (@see 同样参见 RssCheck::SetRoadBoundariesMode())
  void SetRoadBoundariesMode(const ::carla::rss::RoadBoundariesMode &road_boundaries_mode);

  /// @returns 当前的路由目标 (@see 同样参见 RssCheck::GetRoutingTargets())
  const std::vector<::carla::geom::Transform> GetRoutingTargets() const;
  /// @brief 向当前路由目标列表追加一个路由目标 (@see 同样参见 RssCheck::AppendRoutingTarget())
  void AppendRoutingTarget(const ::carla::geom::Transform &routing_target);
  /// @brief 重置当前的路由目标 (@see 同样参见 RssCheck::ResetRoutingTargets())
  void ResetRoutingTargets();

  /// @brief drop the current route (@see also RssCheck::DropRoute())
  void DropRoute();

private:
 /// 传感器滴答（tick，可以理解为周期性触发的操作）回调函数，实际执行传感器相关逻辑的函数
  /// the acutal sensor tick callback function
  void TickRssSensor(const client::Timestamp &timestamp, CallbackFunctionType callback);
  void TickRssSensorThreadLocked(const client::Timestamp &timestamp, SharedPtr<carla::client::ActorList> actors,
                                 CallbackFunctionType callback);
// 在相关线程加锁的情况下执行传感器滴答操作的函数，传入时间戳、演员列表和回调函数等参数
  //// the object actually performing the RSS processing
  std::shared_ptr<::carla::rss::RssCheck> _rss_check;
 // 实际执行RSS处理的对象，是一个智能指针指向RssCheck类的实例，用于进行RSS相关的核心处理逻辑
  /// the id got when registering for the on tick event
  std::size_t _on_tick_register_id;
/// 注册滴答事件（on tick event，周期性触发的事件）时获取的ID，用于标识该注册操作等相关用途
  /// the mutex to protect the actual RSS processing and in case it takes too long to process ever frame
  std::mutex _processing_lock;
/// 互斥锁，用于保护实际的RSS处理过程，防止在处理每帧数据耗时过长等情况下出现并发问题
  /// the future for the async ticking thread
  std::future<void> _tick_future;
/// 用于异步滴答线程的未来（future，用于异步获取线程执行结果等操作）对象
  /// some debug timings
  std::list<double> _rss_check_timings;
/// 一些用于调试的时间记录，可能记录RSS检查等操作的耗时情况等，以列表形式存储
  //// the rss actor constellation callback function
  ActorConstellationCallbackFunctionType _rss_actor_constellation_callback;
/ /RSS行为体星座（周围物体分布相关）回调函数，用于在相关场景下执行自定义的回调逻辑
  /// reqired to store DropRoute() requests until next sensor tick
  bool _drop_route;
 /// 用于存储DropRoute()请求的标志，直到下一次传感器滴答时处理，用于控制路由丢弃相关逻辑的标记
  /// last processed frame
  std::size_t _last_processed_frame;
/// 上一次处理的帧编号，用于记录处理进度等相关情况
  static std::atomic_uint _global_map_initialization_counter_;
};

}  // namespace client
}  // namespace carla
