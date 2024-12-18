
/// 该文件包含交通管理器中常用数据结构的定义。

#pragma once  // 防止头文件被多次包含

#include <chrono>  // 包含时间处理相关的头文件
#include <deque>   // 包含双端队列的头文件
#include <vector>  // 包含动态数组的头文件

#include "carla/client/Actor.h"  // 引入Actor类的定义
#include "carla/geom/Location.h"  // 引入位置类的定义
#include "carla/geom/Rotation.h"  // 引入旋转类的定义
#include "carla/geom/Vector3D.h"  // 引入三维向量类的定义
#include "carla/rpc/ActorId.h"    // 引入ActorId类的定义
#include "carla/rpc/Command.h"     // 引入命令类的定义
#include "carla/rpc/TrafficLightState.h"  // 引入交通灯状态类的定义

#include "carla/trafficmanager/SimpleWaypoint.h"  // 引入简单路径点类的定义

namespace carla {
namespace traffic_manager {

namespace chr = std::chrono;    // std::chrono是C++11引入的日期时间处理库，包含3种时钟：system_clock，steady_clock，
namespace cc = carla::client;  // 为carla::client起别名
namespace cg = carla::geom;    // 为carla::geom起别名

using ActorId = carla::ActorId;  // 使用ActorId类型
using ActorPtr = carla::SharedPtr<cc::Actor>;  // 定义Actor的智能指针类型
using JunctionID = carla::road::JuncId;  // 使用交叉口ID类型
using Junction = carla::SharedPtr<carla::client::Junction>;  // 定义交叉口的智能指针类型
using SimpleWaypointPtr = std::shared_ptr<SimpleWaypoint>;  // 定义简单路径点的智能指针类型
using Buffer = std::deque<SimpleWaypointPtr>;  // 定义一个缓冲区类型，用于存储路径点
using BufferMap = std::unordered_map<carla::ActorId, Buffer>;  // 定义一个哈希映射，键为ActorId，值为Buffer
using TimeInstance = chr::time_point<chr::system_clock, chr::nanoseconds>;  // 定义时间实例类型
using TLS = carla::rpc::TrafficLightState;  // 使用交通灯状态类型

// 用于保存定位数据的结构体
struct LocalizationData {
  SimpleWaypointPtr junction_end_point;  // 交叉口结束点
  SimpleWaypointPtr safe_point;           // 安全点
  bool is_at_junction_entrance;           // 是否在交叉口入口
};

using LocalizationFrame = std::vector<LocalizationData>;  // 定义定位帧类型，存储多个定位数据

// 用于保存碰撞危险数据的结构体
struct CollisionHazardData {
  float available_distance_margin;  // 可用距离余量
  ActorId hazard_actor_id;          // 危险Actor的ID
  bool hazard;                      // 是否存在危险
};

using CollisionFrame = std::vector<CollisionHazardData>;  // 定义碰撞帧类型，存储多个碰撞危险数据

using ControlFrame = std::vector<carla::rpc::Command>;  // 定义控制帧类型，存储多个控制命令

using TLFrame = std::vector<bool>;  // 定义交通灯帧类型，存储交通灯状态（真或假）

/// 用来保存驱动信号的结构
struct ActuationSignal {
  float throttle;  // 油门
  float brake;     // 刹车
  float steer;     // 方向盘
};

/// 用来保存控制器状态的结构体
struct StateEntry {
  cc::Timestamp time_instance;  // 时间实例
  float angular_deviation;       // 角度偏差
  float velocity_deviation;      // 速度偏差
  float steer;                   // 方向盘转角
};

} // namespace traffic_manager
} // namespace carla
