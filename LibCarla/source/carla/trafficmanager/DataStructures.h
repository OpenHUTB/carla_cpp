
/// 该文件包含交通管理器中常用数据结构的定义。

#pragma once

#include <chrono>
#include <deque>
#include <vector>

#include "carla/client/Actor.h"
#include "carla/geom/Location.h"
#include "carla/geom/Rotation.h"
#include "carla/geom/Vector3D.h"
#include "carla/rpc/ActorId.h"
#include "carla/rpc/Command.h"
#include "carla/rpc/TrafficLightState.h"

#include "carla/trafficmanager/SimpleWaypoint.h"

namespace carla {
namespace traffic_manager {

namespace chr = std::chrono;   // std::chrono是C++11引入的日期时间处理库，包含3种时钟：system_clock，steady_clock，high_resolution_clock
namespace cc = carla::client;
namespace cg = carla::geom;

using ActorId = carla::ActorId;
using ActorPtr = carla::SharedPtr<cc::Actor>;
using JunctionID = carla::road::JuncId;
using Junction = carla::SharedPtr<carla::client::Junction>;
using SimpleWaypointPtr = std::shared_ptr<SimpleWaypoint>;
using Buffer = std::deque<SimpleWaypointPtr>;
using BufferMap = std::unordered_map<carla::ActorId, Buffer>;
using TimeInstance = chr::time_point<chr::system_clock, chr::nanoseconds>;
using TLS = carla::rpc::TrafficLightState;

struct LocalizationData {
  SimpleWaypointPtr junction_end_point;
  SimpleWaypointPtr safe_point;
  bool is_at_junction_entrance;
};
using LocalizationFrame = std::vector<LocalizationData>;

struct CollisionHazardData {
  float available_distance_margin;
  ActorId hazard_actor_id;
  bool hazard;
};
using CollisionFrame = std::vector<CollisionHazardData>;

using ControlFrame = std::vector<carla::rpc::Command>;

using TLFrame = std::vector<bool>;

/// 用来保存驱动信号的结构
struct ActuationSignal {
  float throttle;  // 油门
  float brake;     // 删车
  float steer;     // 方向盘
};

/// 用来保存控制器状态的状态。
struct StateEntry {
  cc::Timestamp time_instance;
  float angular_deviation;
  float velocity_deviation;
  float steer;
};

} // namespace traffic_manager
} // namespace carla
