// Copyright (c) 2019-2020 Intel Corporation
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/rss/RssCheck.h"

#include <spdlog/sinks/stdout_color_sinks.h>
#ifdef RSS_USE_TBB
#include <tbb/tbb.h>
#endif
// 包含与地图访问相关的日志记录功能的头文件，用于在地图操作等过程中记录日志
#include <ad/map/access/Logging.hpp>
// 包含地图访问操作相关的头文件，定义了一些对地图进行查询、修改等操作的函数或类
#include <ad/map/access/Operation.hpp>
// 包含与地图中交叉路口相关操作的头文件
#include <ad/map/intersection/Intersection.hpp>
// 包含与地图中车道相关操作的头文件，例如车道信息查询、车道变换等操作的实现可能在这里面
#include <ad/map/lane/Operation.hpp>
// 包含地图匹配相关功能的头文件，用于将车辆等对象与地图中的位置进行匹配等操作
#include <ad/map/match/AdMapMatching.hpp>
// 包含地图匹配操作相关的头文件，对地图匹配结果进一步处理等操作的定义所在
#include <ad/map/match/MapMatchedOperation.hpp>
// 包含与路线中车道区间操作相关的头文件，用于处理路线上车道区间相关的逻辑，比如获取区间信息等
#include <ad/map/route/LaneIntervalOperation.hpp>
// 包含与路线操作相关的头文件
#include <ad/map/route/Operation.hpp>
// 包含与路线规划相关的头文件，涉及根据一些条件规划路线的功能实现
#include <ad/map/route/Planning.hpp>
// 包含 RSS 相关的地图日志记录功能的头文件，用于在 RSS 模块涉及地图操作时记录日志
#include <ad/rss/map/Logging.hpp>
// 包含 RSS 中对象转换相关的头文件，用于将不同格式或类型的对象进行转换以适配 RSS 相关处理逻辑
#include <ad/rss/map/RssObjectConversion.hpp>
// 包含 RSS 中对象数据相关的头文件，定义了表示 RSS 场景中对象的数据结构及相关操作
#include <ad/rss/map/RssObjectData.hpp>
// 包含 RSS 场景创建相关的头文件，用于创建 RSS 场景，组合各种对象及其关系等
#include <ad/rss/map/RssSceneCreator.hpp>
// 包含 RSS 状态操作相关的头文件
#include <ad/rss/state/RssStateOperation.hpp>
// 包含时间相关的头文件，用于处理时间戳、时间间隔等时间相关的操作
#include <chrono>
#include <tuple>

#include "carla/client/Map.h"
#include "carla/client/TrafficLight.h"
#include "carla/client/Vehicle.h"
#include "carla/client/Walker.h"
#include "carla/client/Waypoint.h"

#define DEBUG_TIMING 0
// 定义在carla命名空间的rss子命名空间中 
namespace carla {
namespace rss {
 // 定义一个函数printRoute，用于打印路由信息  
 // 参数包括一个描述路由的字符串route_descr和一个FullRoute类型的常量引用route 
void printRoute(std::string const &route_descr, ::ad::map::route::FullRoute const &route) {
    // 首先打印出路由的描述信息
    std::cout << route_descr << std::endl;
    // 遍历路由中的每一个路段（road segment）
    for (auto road_segment : route.roadSegments) {
        // 对于每一个路段，遍历其所有可驾驶的车道段（lane segment）
        for (auto lane_segment : road_segment.drivableLaneSegments) {
        // 打印出车道段的详细信息，包括：  
         // 车道ID（laneId），转换为uint64_t类型  
         // 车道段的起始位置（start）和结束位置（end），转换为double类型，并保留两位小数  
         // 输出格式为：(车道ID | 起始位置:结束位置)
            std::cout << "(" << static_cast<uint64_t>(lane_segment.laneInterval.laneId) << " | " << std::setprecision(2)
                << static_cast<double>(lane_segment.laneInterval.start) << ":"
                << static_cast<double>(lane_segment.laneInterval.end) << ")    ";
    }
    // 每个路段的车道段信息打印完毕后，输出一个换行符，以便于区分不同的路段
    std::cout << std::endl;
  }
}

// 度到弧度转换常数 PI / 180
constexpr float to_radians = static_cast<float>(M_PI) / 180.0f;
// 定义了一个名为EgoDynamicsOnRoute的类
// 在构造函数中初始化了多个成员变量
EgoDynamicsOnRoute::EgoDynamicsOnRoute()
  : time_since_epoch_check_start_ms(0.),
    time_since_epoch_check_end_ms(0.),
    ego_speed(0.),// 速度
    min_stopping_distance(0.),// 停止距离
    ego_center({0., 0., 0.}),// 中心位置
    ego_heading(0.),//航向
    ego_heading_change(0.),// 航向变化
    ego_steering_angle(0.),// 转向角度
    ego_center_within_route(false),
    crossing_border(false),
    route_heading(0.),
    route_nominal_center({0., 0., 0.}),
    heading_diff(0.),
    route_speed_lat(0.),
    route_speed_lon(0.),
    route_accel_lat(0.),
    route_accel_lon(0.),
    avg_route_accel_lat(0.),
    avg_route_accel_lon(0.) {
   // 初始化时间戳对象中的已流逝秒数为 0
  timestamp.elapsed_seconds = 0.;
}
// 获取一个指向 spdlog::logger 的共享指针，用于记录日志信息。这里创建了一个名为 "RssCheck" 的日志记录器，并且是线程安全的（通过 stdout_color_mt 函数创建），每次调用都会返回同一个静态实例
std::shared_ptr<spdlog::logger> getLogger() {
  static auto logger = spdlog::stdout_color_mt("RssCheck");
  return logger;
}
// 获取一个指向 spdlog::logger 的共享指针，用于记录计时相关的日志信息。创建了一个名为 "RssCheckTiming" 的日志记录器，同样是线程安全的，每次调用返回同一个静态实例，用于记录特定的计时相关内容
std::shared_ptr<spdlog::logger> getTimingLogger() {
  static auto logger = spdlog::stdout_color_mt("RssCheckTiming");
  return logger;
}
// RssCheck 类的成员函数，用于获取默认的车辆动力学参数配置，返回一个 ::ad::rss::world::RssDynamics 类型的对象，包含了车辆在纵向、横向等方向的加速度、刹车等参数设置
::ad::rss::world::RssDynamics RssCheck::GetDefaultVehicleDynamics() {
  ::ad::rss::world::RssDynamics default_ego_vehicle_dynamics;
 // 设置纵向最大加速度，单位可能与具体的物理模拟相关，这里设置为 3.5（具体单位需结合项目其他部分确定）
  default_ego_vehicle_dynamics.alphaLon.accelMax = ::ad::physics::Acceleration(3.5);
 // 设置纵向最大刹车加速度，为负值表示减速，这里设置为 -8.
  default_ego_vehicle_dynamics.alphaLon.brakeMax = ::ad::physics::Acceleration(-8.);
 // 设置纵向最小刹车加速度，同样为负值，这里设置为 -4.
  default_ego_vehicle_dynamics.alphaLon.brakeMin = ::ad::physics::Acceleration(-4.);
 // 设置纵向最小修正刹车加速度，这里设置为 -3
  default_ego_vehicle_dynamics.alphaLon.brakeMinCorrect = ::ad::physics::Acceleration(-3);
 // 设置横向最大加速度，这里设置为 0.2
  default_ego_vehicle_dynamics.alphaLat.accelMax = ::ad::physics::Acceleration(0.2);
 // 设置横向最小刹车加速度，这里设置为 -0.8
  default_ego_vehicle_dynamics.alphaLat.brakeMin = ::ad::physics::Acceleration(-0.8);
 // 设置横向波动余量，单位为距离相关，这里设置为 0.1（具体单位结合项目确定）
  default_ego_vehicle_dynamics.lateralFluctuationMargin = ::ad::physics::Distance(0.1);
 // 设置响应时间，单位为时间相关，这里设置为 1.0（具体单位结合项目确定），表示车辆对外部情况做出反应的时间
  default_ego_vehicle_dynamics.responseTime = ::ad::physics::Duration(1.0);
  // 设置加速时的最大速度，这里设置为 100.（具体单位结合项目确定）
  default_ego_vehicle_dynamics.maxSpeedOnAcceleration = ::ad::physics::Speed(100.);
 // 设置行人转弯半径，单位为距离相关，这里设置为 2.0
  default_ego_vehicle_dynamics.unstructuredSettings.pedestrianTurningRadius = ad::physics::Distance(2.0);
  // 设置驶离最大角度，单位为角度相关，这里设置为 2.4（具体角度单位结合项目确定）
  default_ego_vehicle_dynamics.unstructuredSettings.driveAwayMaxAngle = ad::physics::Angle(2.4);
  // 设置车辆偏航角速度变化率，单位为角加速度相关，这里设置为 0.3（具体单位结合项目确定）
  default_ego_vehicle_dynamics.unstructuredSettings.vehicleYawRateChange = ad::physics::AngularAcceleration(0.3);
 // 设置车辆最小转弯半径，单位为距离相关，这里设置为 3.5
  default_ego_vehicle_dynamics.unstructuredSettings.vehicleMinRadius = ad::physics::Distance(3.5);
 // 设置车辆轨迹计算步长，单位为时间相关，这里设置为 0.2
  default_ego_vehicle_dynamics.unstructuredSettings.vehicleTrajectoryCalculationStep = ad::physics::Duration(0.2);
  return default_ego_vehicle_dynamics;
}
// RssCheck 类的成员函数，用于获取默认的行人动力学参数配置，返回一个 ::ad::rss::world::RssDynamics 类型的对象，同样包含了行人在不同方向的加速度、刹车等相关参数设置
::ad::rss::world::RssDynamics RssCheck::GetDefaultPedestrianDynamics() {
  ::ad::rss::world::RssDynamics default_pedestrian_dynamics;
 // 设置行人纵向最大加速度，这里设置为 2.0
  default_pedestrian_dynamics.alphaLon.accelMax = ::ad::physics::Acceleration(2.0);
 // 设置行人纵向最大刹车加速度，这里设置为 -4.
  default_pedestrian_dynamics.alphaLon.brakeMax = ::ad::physics::Acceleration(-4.);
 // 设置行人纵向最小刹车加速度，这里设置为 -2.
  default_pedestrian_dynamics.alphaLon.brakeMin = ::ad::physics::Acceleration(-2.);
 // 设置行人纵向最小修正刹车加速度，这里设置为 -2.
  default_pedestrian_dynamics.alphaLon.brakeMinCorrect = ::ad::physics::Acceleration(-2.);
 // 设置行人横向最大加速度，这里设置为 0.001，数值较小符合行人横向移动相对缓慢的特点
  default_pedestrian_dynamics.alphaLat.accelMax = ::ad::physics::Acceleration(0.001);
 / 设置行人横向最小刹车加速度，这里设置为 -0.001
  default_pedestrian_dynamics.alphaLat.brakeMin = ::ad::physics::Acceleration(-0.001);
 // 设置行人横向波动余量，这里设置为 0.1
  default_pedestrian_dynamics.lateralFluctuationMargin = ::ad::physics::Distance(0.1);
 // 设置行人响应时间，这里设置为 0.5，通常行人反应时间相对车辆可能较短一些（具体取决于模拟设定）
  default_pedestrian_dynamics.responseTime = ::ad::physics::Duration(0.5);
 // 设置行人加速时的最大速度，这里设置为 10.
  default_pedestrian_dynamics.maxSpeedOnAcceleration = ::ad::physics::Speed(10.);
 // 设置行人转弯半径，这里设置为 2.0
  default_pedestrian_dynamics.unstructuredSettings.pedestrianTurningRadius = ad::physics::Distance(2.0);
 // 设置行人驶离最大角度，这里设置为 2.4
  default_pedestrian_dynamics.unstructuredSettings.driveAwayMaxAngle = ad::physics::Angle(2.4);
 // 设置行人相关的车辆偏航角速度变化率（虽然对于行人来说这个概念有点不太常规，但可能在统一的模拟框架中有相关用途），这里设置为 0.3
  default_pedestrian_dynamics.unstructuredSettings.vehicleYawRateChange = ad::physics::AngularAcceleration(0.3);
 // 设置行人相关的车辆最小转弯半径（同样，可能是在统一框架下的一种设定），这里设置为 3.5
  default_pedestrian_dynamics.unstructuredSettings.vehicleMinRadius = ad::physics::Distance(3.5);
 // 设置行人相关的车辆轨迹计算步长，这里设置为 0.2
  default_pedestrian_dynamics.unstructuredSettings.vehicleTrajectoryCalculationStep = ad::physics::Duration(0.2);

  return default_pedestrian_dynamics;
}

RssCheck::RssCheck(float maximum_steering_angle)// 设置允许的最大转向角度
// 初始化成员变量
  : _maximum_steering_angle(maximum_steering_angle), _road_boundaries_mode(GetDefaultRoadBoundariesMode()) {
   // 获取标准的日志记录器
  _logger = getLogger();
   // 获取专门用于计时的日志记录器
  _timing_logger = getTimingLogger();
   // 将计时日志记录器的级别设置为关闭
  _timing_logger->set_level(spdlog::level::off);
// 设置全局和地图相关的日志级别为警告
  SetLogLevel(spdlog::level::warn);
  SetMapLogLevel(spdlog::level::warn);
// 获取并设置行人的自我车辆动态回调
  _default_actor_constellation_callback_ego_vehicle_dynamics = GetDefaultVehicleDynamics();
   //获取并设置其他车辆的默认动态回调
  _default_actor_constellation_callback_other_vehicle_dynamics = GetDefaultVehicleDynamics();
  // 将其他车辆的响应时间设置为 2 秒；其余保持不变
  _default_actor_constellation_callback_other_vehicle_dynamics.responseTime = ::ad::physics::Duration(2.0);
   // 获取行人的默认动态回调
  _default_actor_constellation_callback_pedestrian_dynamics = GetDefaultPedestrianDynamics();

  //创建一个默认回调.
   // 回调函数
  _actor_constellation_callback =
      [this](carla::SharedPtr<::carla::rss::ActorConstellationData> actor_constellation_data)
      -> ::carla::rss::ActorConstellationResult {
    ::carla::rss::ActorConstellationResult actor_constellation_result;

    actor_constellation_result.rss_calculation_mode = ::ad::rss::map::RssMode::NotRelevant;
    actor_constellation_result.restrict_speed_limit_mode =
        ::ad::rss::map::RssSceneCreation::RestrictSpeedLimitMode::IncreasedSpeedLimit10;
    actor_constellation_result.actor_object_type = ad::rss::world::ObjectType::Invalid;
    actor_constellation_result.ego_vehicle_dynamics = this->_default_actor_constellation_callback_ego_vehicle_dynamics;
    actor_constellation_result.actor_dynamics = this->_default_actor_constellation_callback_other_vehicle_dynamics;

    if (actor_constellation_data->other_actor != nullptr) {
      if (boost::dynamic_pointer_cast<carla::client::Walker>(actor_constellation_data->other_actor) != nullptr) {
        actor_constellation_result.rss_calculation_mode = ::ad::rss::map::RssMode::Unstructured;
        actor_constellation_result.actor_object_type = ad::rss::world::ObjectType::Pedestrian;
        actor_constellation_result.actor_dynamics = this->_default_actor_constellation_callback_pedestrian_dynamics;
      } else if (boost::dynamic_pointer_cast<carla::client::Vehicle>(actor_constellation_data->other_actor) !=
                 nullptr) {
        actor_constellation_result.rss_calculation_mode = ::ad::rss::map::RssMode::Structured;
        actor_constellation_result.actor_object_type = ad::rss::world::ObjectType::OtherVehicle;

        if (GetSpeed(*actor_constellation_data->other_actor) == ::ad::physics::Speed(0.)) {
          /*
             对静止不动的车辆进行特殊处理，以应对核心 RSS 实现中尚未实现的横向交叉路口检查。
             如果另一个待着不动，我们就不会假设他会加速；
             否则，如果待在十字路口，反应时间内的加速度将使车进入十字路口，当前的 RSS 实现将会立即认为这是危险的。

           */
          actor_constellation_result.actor_dynamics.alphaLon.accelMax = ::ad::physics::Acceleration(0.);
        }
      }
    }

    /* 
      由于 Ego 车辆是手动控制的，因此 Ego 车辆在横向加速度很可能远高于自动控制时的加速度
      在自动驾驶汽车中，当合理利用 RSS 实现时，会考虑到低级控制器
      但是 CARLA 中的简单 RSS 限制器无法做到这一点...
      因此，我们至少应该告诉 RSS 我们的加速度超过这个数值时，及时对此做出反应。

      */
    auto const abs_avg_route_accel_lat = std::fabs(actor_constellation_data->ego_dynamics_on_route.avg_route_accel_lat);
    if (abs_avg_route_accel_lat > actor_constellation_result.ego_vehicle_dynamics.alphaLat.accelMax) {
      actor_constellation_result.ego_vehicle_dynamics.alphaLat.accelMax =
          std::min(ad::physics::Acceleration(20.), abs_avg_route_accel_lat);
    }
    return actor_constellation_result;
  };

  // 设置默认动态
  _carla_rss_state.default_ego_vehicle_dynamics = _default_actor_constellation_callback_ego_vehicle_dynamics;

  _logger->debug("RssCheck with default actor constellation callback created");
}
// RssCheck类的构造函数
// 参数：
// - maximum_steering_angle：最大转向角度
// - rss_actor_constellation_callback：参与者星座回调函数类型，用于获取参与者星座相关信息
// - carla_ego_actor：指向CARLA中自车参与者的共享指针
RssCheck::RssCheck(float maximum_steering_angle,
                   ActorConstellationCallbackFunctionType rss_actor_constellation_callback,
                   carla::SharedPtr<carla::client::Actor> const &carla_ego_actor)
  : _maximum_steering_angle(maximum_steering_angle),
    _actor_constellation_callback(rss_actor_constellation_callback),
    _road_boundaries_mode(GetDefaultRoadBoundariesMode()) {
   // 获取日志记录器实例，用于记录各类日志信息
  _logger = getLogger();
   // 获取计时日志记录器实例，用于记录时间相关的日志信息
  _timing_logger = getTimingLogger();
   // 将计时日志记录器的日志级别设置为关闭（不记录）
  _timing_logger->set_level(spdlog::level::off);
// 设置整体的日志级别为警告级别
  SetLogLevel(spdlog::level::warn);
   // 设置地图相关的日志级别为警告级别
  SetMapLogLevel(spdlog::level::warn);
 // 获取与自车参与者匹配的对象，允许车辆距离路线至少2.0米，以免丧失与路线的接触
  _carla_rss_state.ego_match_object = GetMatchObject(carla_ego_actor, ::ad::physics::Distance(2.0));
     // 更新默认的RSS动力学相关信息
  UpdateDefaultRssDynamics(_carla_rss_state);
 // 在日志中输出调试信息，表示RssCheck对象已创建
  _logger->debug("RssCheck created");
}
// RssCheck类的析构函数，这里为空实现，可能在子类中会有具体的资源释放等操作
RssCheck::~RssCheck() {}
// 设置日志级别，对spdlog库和RssCheck类的内部_logger都生效
void RssCheck::SetLogLevel(const spdlog::level::level_enum &log_level) {
  spdlog::set_level(log_level); // 设置全局spdlog日志级别
  _logger->set_level(log_level); // 设置RssCheck类内部_logger的日志级别
}
// 设置地图日志级别，对ad::map和ad::rss::map模块的日志器生效
void RssCheck::SetMapLogLevel(const spdlog::level::level_enum &map_log_level) {
  ::ad::map::access::getLogger()->set_level(map_log_level); // 设置ad::map模块的日志级别
  ::ad::rss::map::getLogger()->set_level(map_log_level); // 设置ad::rss::map模块的日志级别
}
// 获取默认的自我车辆动态参数，这些参数用于RSS检查回调
const ::ad::rss::world::RssDynamics &RssCheck::GetDefaultActorConstellationCallbackEgoVehicleDynamics() const {
  return _default_actor_constellation_callback_ego_vehicle_dynamics;
}
// 设置默认的自我车辆动态参数
void RssCheck::SetDefaultActorConstellationCallbackEgoVehicleDynamics(
    const ::ad::rss::world::RssDynamics &ego_vehicle_dynamics) {
  _default_actor_constellation_callback_ego_vehicle_dynamics = ego_vehicle_dynamics;
}
// 获取默认的其他车辆动态参数，这些参数用于RSS检查回调
const ::ad::rss::world::RssDynamics &RssCheck::GetDefaultActorConstellationCallbackOtherVehicleDynamics() const {
  return _default_actor_constellation_callback_other_vehicle_dynamics;
}
// 设置默认的其他车辆动态参数
void RssCheck::SetDefaultActorConstellationCallbackOtherVehicleDynamics(
    const ::ad::rss::world::RssDynamics &other_vehicle_dynamics) {
  _default_actor_constellation_callback_other_vehicle_dynamics = other_vehicle_dynamics;
}
// 获取默认的行人动态参数
const ::ad::rss::world::RssDynamics &RssCheck::GetDefaultActorConstellationCallbackPedestrianDynamics() const {
  return _default_actor_constellation_callback_pedestrian_dynamics;
}
// 设置默认的行人动态参数
void RssCheck::SetDefaultActorConstellationCallbackPedestrianDynamics(
    const ::ad::rss::world::RssDynamics &pedestrian_dynamics) {
  _default_actor_constellation_callback_pedestrian_dynamics = pedestrian_dynamics;
}
// 获取道路边界模式
const ::carla::rss::RoadBoundariesMode &RssCheck::GetRoadBoundariesMode() const {
  return _road_boundaries_mode;
}
// 设置道路边界模式
void RssCheck::SetRoadBoundariesMode(const ::carla::rss::RoadBoundariesMode &road_boundaries_mode) {
  _road_boundaries_mode = road_boundaries_mode;
}
// 向RssCheck类添加路由目标，这些目标用于RSS检查
void RssCheck::AppendRoutingTarget(::carla::geom::Transform const &routing_target) {
  _routing_targets_to_append.push_back(
      ::ad::map::point::createENUPoint(routing_target.location.x, -1. * routing_target.location.y, 0.));
}
// 获取路由目标列表信息（常量函数）
// 返回值：包含路由目标坐标变换信息的向量
const std::vector<::carla::geom::Transform> RssCheck::GetRoutingTargets() const {
  std::vector<::carla::geom::Transform> routing_targets;
  // 判断输入范围是否有效，如果有效则进行后续处理
  if (withinValidInputRange(_routing_targets)) {
    for (auto const &target : _routing_targets) {
      ::carla::geom::Transform routing_target;
      routing_target.location.x = static_cast<float>(target.x);
      routing_target.location.y = static_cast<float>(-target.y);
      routing_target.location.z = 0.f;
      routing_targets.push_back(routing_target);
    }
  }
  return routing_targets;
}
// 重置路由目标，清空已有的路由目标列表和待添加的路由目标列表
void RssCheck::ResetRoutingTargets() {
  _routing_targets.clear();
  _routing_targets_to_append.clear();
}
// 放弃当前路线，在日志中输出相关调试信息，并将自车的路线设置为空路线
void RssCheck::DropRoute() {
  _logger->debug("Dropping Route:: {}", _carla_rss_state.ego_route);
  _carla_rss_state.ego_route = ::ad::map::route::FullRoute();
}
// 检查对象相关信息，进行一系列的逻辑判断和操作
// 参数：
// - timestamp：时间戳信息
// - actors：指向参与者列表的共享指针
// - carla_ego_actor：指向自车参与者的共享指针
// - output_response：用于输出合适响应的对象
// - output_rss_state_snapshot：用于输出RSS状态快照的对象
// - output_situation_snapshot：用于输出情况快照的对象
// - output_world_model：用于输出世界模型的对象
// - output_rss_ego_dynamics_on_route：用于输出自车在路线上的RSS动力学信息的对象
// 返回值：表示检查结果的布尔值，成功为true，失败为false
bool RssCheck::CheckObjects(carla::client::Timestamp const &timestamp,
                            carla::SharedPtr<carla::client::ActorList> const &actors,
                            carla::SharedPtr<carla::client::Actor> const &carla_ego_actor,
                            ::ad::rss::state::ProperResponse &output_response,
                            ::ad::rss::state::RssStateSnapshot &output_rss_state_snapshot,
                            ::ad::rss::situation::SituationSnapshot &output_situation_snapshot,
                            ::ad::rss::world::WorldModel &output_world_model,
                            EgoDynamicsOnRoute &output_rss_ego_dynamics_on_route) {
  bool result = false;
  try {
 // 获取从纪元开始到当前检查开始的时间（以毫秒为单位）
    double const time_since_epoch_check_start_ms =
        std::chrono::duration<double, std::milli>(std::chrono::system_clock::now().time_since_epoch()).count();
#if DEBUG_TIMING
    // 在调试模式下，输出时间相关信息以及开始检查对象的提示信息
    std::cout << "--- time: " << timestamp.frame << ", " << timestamp.elapsed_seconds << std::endl;
    auto t_start = std::chrono::high_resolution_clock::now();
    auto t_end = std::chrono::high_resolution_clock::now();
    std::cout << "-> SC " << std::chrono::duration<double, std::milli>(t_end - t_start).count() << " start checkObjects"
              << std::endl;
#endif
// 将传入的自车参与者指针转换为车辆类型的共享指针，如果转换失败则返回nullptr
    const auto carla_ego_vehicle = boost::dynamic_pointer_cast<carla::client::Vehicle>(carla_ego_actor);
    if (carla_ego_vehicle == nullptr) {
     // 如果自车不是车辆类型，则在日志中输出错误信息，提示RSS传感器仅支持车辆作为自车
      _logger->error("RSS Sensor only support vehicles as ego.");
    }

#if DEBUG_TIMING
    t_end = std::chrono::high_resolution_clock::now();
    std::cout << "-> ME " << std::chrono::duration<double, std::milli>(t_end - t_start).count()
              << " before  MapMatching" << std::endl;
#endif

    //允许车辆距离路线至少 2.0 米，以免丧失
    //与路线的接触
    auto const ego_match_object = GetMatchObject(carla_ego_actor, ::ad::physics::Distance(2.0));

    if (::ad::map::point::isValid(_carla_rss_state.ego_match_object.enuPosition.centerPoint, false)) {
      // check for bigger position jumps of the ego vehicle
      auto const travelled_distance = ::ad::map::point::distance(
          _carla_rss_state.ego_match_object.enuPosition.centerPoint, ego_match_object.enuPosition.centerPoint);
      if (travelled_distance > ::ad::physics::Distance(10.)) {
        // 如果距离大于设定值（10.），则在日志中输出警告信息，提示检测到自车位置跳跃，并强制重新规划路线
        _logger->warn("Jump in ego vehicle position detected {} -> {}! Force reroute!",
                      _carla_rss_state.ego_match_object.enuPosition.centerPoint,
                      ego_match_object.enuPosition.centerPoint);
        DropRoute();
      }
    }
// 将计算得到的与自车匹配的对象赋值给_carla_rss_state中的对应成员变量
    _carla_rss_state.ego_match_object = ego_match_object;
// 在日志中以跟踪级别（trace）输出自车匹配对象的相关信息，方便调试查看具体匹配情况
    _logger->trace("MapMatch:: {}", _carla_rss_state.ego_match_object);

#if DEBUG_TIMING
    // 获取当前时间，用于记录时间相关的调试信息，此处表示地图匹配自车部分结束的时间记录
    t_end = std::chrono::high_resolution_clock::now();
    std::cout << "-> ME " << std::chrono::duration<double, std::milli>(t_end - t_start).count()
              << " after ego MapMatching" << std::endl;
#endif
// 根据当前状态更新路线信息，可能涉及根据匹配对象等重新规划、调整自车的行驶路线等操作
    UpdateRoute(_carla_rss_state);

#if DEBUG_TIMING
    t_end = std::chrono::high_resolution_clock::now();
    std::cout << "-> RU " << std::chrono::duration<double, std::milli>(t_end - t_start).count()
              << " after route update " << std::endl;
#endif
// 计算自车在路线上的动力学信息，传入多个相关参数，包括时间戳、起始检查时间、自车车辆信息、自车匹配对象、自车路线以及默认的自车动力学信息等，并结合之前的动力学信息进行更新计算
    _carla_rss_state.ego_dynamics_on_route = CalculateEgoDynamicsOnRoute(
        timestamp, time_since_epoch_check_start_ms, *carla_ego_vehicle, _carla_rss_state.ego_match_object,
        _carla_rss_state.ego_route, _carla_rss_state.default_ego_vehicle_dynamics,
        _carla_rss_state.ego_dynamics_on_route);
// 根据当前的_carla_rss_state状态更新默认的RSS动力学相关信息，可能涉及根据自车当前状态调整一些默认的动力学参数等
    UpdateDefaultRssDynamics(_carla_rss_state);
// 根据传入的时间戳、参与者列表、自车车辆信息以及当前的_carla_rss_state状态来创建世界模型，这个世界模型应该包含了整个场景中相关对象的信息等
    CreateWorldModel(timestamp, *actors, *carla_ego_vehicle, _carla_rss_state);

#if DEBUG_TIMING
    t_end = std::chrono::high_resolution_clock::now();
    std::cout << "-> WM " << std::chrono::duration<double, std::milli>(t_end - t_start).count()
              << " after create world model " << std::endl;
#endif
// 执行实际的检查操作，根据当前的_carla_rss_state状态来判断是否存在某些情况（比如安全风险等），返回检查结果（布尔值），并将结果赋值给result变量
    result = PerformCheck(_carla_rss_state);

#if DEBUG_TIMING
    t_end = std::chrono::high_resolution_clock::now();
    std::cout << "-> CH " << std::chrono::duration<double, std::milli>(t_end - t_start).count() << " end RSS check"
              << std::endl;
#endif
// 对检查结果进行分析，可能涉及更深入地查看结果中的各项指标、状态等，以便后续进一步处理或者记录相关信息
    AnalyseCheckResults(_carla_rss_state);

#if DEBUG_TIMING
    t_end = std::chrono::high_resolution_clock::now();
    std::cout << "-> AN " << std::chrono::duration<double, std::milli>(t_end - t_start).count()
              << " end analyze results" << std::endl;
#endif
// 记录自车动力学在检查结束时的时间戳（从纪元开始到当前的时间，以毫秒为单位），用于后续统计或者记录整个检查过程的时间相关信息
    _carla_rss_state.ego_dynamics_on_route.time_since_epoch_check_end_ms =
        std::chrono::duration<double, std::milli>(std::chrono::system_clock::now().time_since_epoch()).count();
// 将检查过程中得到的相关结果存储到输出参数中，方便外部调用者获取此次检查的各项信息，例如合适的响应、RSS状态快照、情况快照、世界模型以及自车在路线上的动力学信息等
// 存储合适响应信息
    // 存储结果
    output_response = _carla_rss_state.proper_response;
   // 存储RSS状态快照信息
    output_rss_state_snapshot = _carla_rss_state.rss_state_snapshot;
   // 存储情况快照信息
    output_situation_snapshot = _carla_rss_state.situation_snapshot;
   // 存储世界模型信息
    output_world_model = _carla_rss_state.world_model;
    output_rss_ego_dynamics_on_route = _carla_rss_state.ego_dynamics_on_route;
   // 存储自车在路线上的动力学信息
    if (_carla_rss_state.dangerous_state) {
      _logger->debug("===== ROUTE NOT SAFE =====");
    } else {
      _logger->debug("===== ROUTE SAFE =====");
    }

#if DEBUG_TIMING
    t_end = std::chrono::high_resolution_clock::now();
    std::cout << "-> EC " << std::chrono::duration<double, std::milli>(t_end - t_start).count() << " end check objects"
              << std::endl;
#endif
  } catch (...) {
    // 如果在检查过程中出现任何异常，在日志中记录错误信息，表示检查失败
    _logger->error("Exception -> Check failed");
  }
 // 返回最终的检查结果（布尔值），外部调用者可根据此结果判断整体检查情况
  return result;
}
// 获取与给定参与者（actor）匹配的对象信息，传入参与者指针以及采样距离作为参数，返回匹配后的对象信息
::ad::map::match::Object RssCheck::GetMatchObject(carla::SharedPtr<carla::client::Actor> const &actor,
                                                  ::ad::physics::Distance const &sampling_distance) const {
 // 创建一个待返回的匹配对象实例
  ::ad::map::match::Object match_object;
// 获取参与者（actor）的坐标变换信息，可能包含位置、旋转等信息
  auto const vehicle_transform = actor->GetTransform();
   // 将参与者在世界坐标系中的x坐标转换为ENU坐标格式，并赋值给匹配对象的中心坐标x分量
  match_object.enuPosition.centerPoint.x = ::ad::map::point::ENUCoordinate(vehicle_transform.location.x);
   // 将参与者在世界坐标系中的y坐标取反后转换为ENU坐标格式，并赋值给匹配对象的中心坐标y分量
  match_object.enuPosition.centerPoint.y = ::ad::map::point::ENUCoordinate(-1. * vehicle_transform.location.y);
  // 将参与者在世界坐标系中的z坐标转换为ENU坐标格式（此处原代码注释掉了直接使用vehicle_transform.location.z的部分，可能存在特殊考虑），并赋值给匹配对象的中心坐标z分量
  match_object.enuPosition.centerPoint.z = ::ad::map::point::ENUCoordinate(0.);  // vehicle_transform.location.z;
 // 根据参与者的旋转角度（yaw，偏航角）创建ENU坐标下的朝向信息，并赋值给匹配对象的朝向属性，这里进行了角度单位转换（可能是转换为弧度制，假设to_radians是角度转弧度的常量或者函数）
  match_object.enuPosition.heading =
      ::ad::map::point::createENUHeading(-1 * vehicle_transform.rotation.yaw * to_radians);
 // 将参与者指针尝试转换为车辆类型的共享指针，如果转换成功，表示参与者是车辆类型
  auto const vehicle = boost::dynamic_pointer_cast<carla::client::Vehicle>(actor);
 // 将参与者指针尝试转换为行人类型的共享指针，如果转换成功，表示参与者是行人类型
  auto const walker = boost::dynamic_pointer_cast<carla::client::Walker>(actor);
  if (vehicle != nullptr) {
   // 如果参与者是车辆类型，获取车辆的包围盒信息（bounding box），通常包含车辆在各个方向上的尺寸范围
    const auto &bounding_box = vehicle->GetBoundingBox();
   // 根据包围盒在x方向上的尺寸（extent.x）计算并设置匹配对象在ENU坐标下的长度属性，乘以2可能是考虑包围盒尺寸是从中心到边缘的距离，这里获取整体长度
    match_object.enuPosition.dimension.length = ::ad::physics::Distance(2 * bounding_box.extent.x);
    // 同理，根据包围盒在y方向上的尺寸计算并设置匹配对象在ENU坐标下的宽度属性
    match_object.enuPosition.dimension.width = ::ad::physics::Distance(2 * bounding_box.extent.y);
    // 根据包围盒在z方向上的尺寸计算并设置匹配对象在ENU坐标下的高度属性
    match_object.enuPosition.dimension.height = ::ad::physics::Distance(2 * bounding_box.extent.z);
  } else if (walker != nullptr) {
    // 如果参与者是行人类型，获取行人的包围盒信息
    const auto &bounding_box = walker->GetBoundingBox();
    // 按照与车辆类似的方式，根据行人包围盒在各个方向上的尺寸设置匹配对象相应的长度、宽度、高度属性
    match_object.enuPosition.dimension.length = ::ad::physics::Distance(2 * bounding_box.extent.x);
    match_object.enuPosition.dimension.width = ::ad::physics::Distance(2 * bounding_box.extent.y);
    match_object.enuPosition.dimension.height = ::ad::physics::Distance(2 * bounding_box.extent.z);
  } else {
    // 如果参与者既不是车辆也不是行人类型，在日志中记录错误信息，提示无法获取该参与者的包围盒信息，并输出参与者的ID，方便排查问题
    _logger->error("Could not get bounding box of actor {}", actor->GetId());
  }
  // 获取ENU坐标参考点信息，并赋值给匹配对象的ENU参考点属性，这个参考点可能用于后续的坐标转换、匹配等相关操作的基准
  match_object.enuPosition.enuReferencePoint = ::ad::map::access::getENUReferencePoint();
// 创建地图匹配相关的实例，用于执行地图匹配操作，获取匹配后的包围盒信息
  ::ad::map::match::AdMapMatching map_matching;
 // 通过地图匹配实例，根据匹配对象的ENU位置信息以及给定的采样距离，获取地图匹配后的包围盒信息，并赋值给匹配对象的相应属性
  match_object.mapMatchedBoundingBox =
      map_matching.getMapMatchedBoundingBox(match_object.enuPosition, sampling_distance);
 // 返回最终构建好的匹配对象信息，包含了坐标、尺寸、朝向以及地图匹配后的包围盒等相关信息
  return match_object;
}
// 根据给定的参与者（actor）获取其速度信息，返回对应的物理速度（Speed）类型的数值
::ad::physics::Speed RssCheck::GetSpeed(carla::client::Actor const &actor) const {
  // 获取参与者的速度向量信息，通常包含在各个坐标轴方向上的速度分量
  auto velocity = actor.GetVelocity();
  // 获取参与者的坐标变换信息（可能包含旋转等信息），用于后续对速度向量进行旋转操作
  auto const actor_transform = actor.GetTransform();
 // 根据参与者的旋转信息对速度向量进行反向旋转操作，可能是为了将速度转换到某个特定的参考坐标系下
  actor_transform.rotation.InverseRotateVector(velocity);
 // 根据旋转后的速度向量在x和y方向上的分量计算速度的大小（二维平面上的速度大小），通过勾股定理计算，创建一个物理速度类型（::ad::physics::Speed）的实例
  ::ad::physics::Speed speed(std::sqrt(velocity.x * velocity.x + velocity.y * velocity.y));
 // 如果速度在x方向上的分量小于0，表示速度方向与设定的正方向相反，将速度取反，以符合特定的速度正负表示规则
  if (velocity.x < 0.) {
    speed = -speed;
  }
 // 返回最终计算得到的参与者的速度信息，以符合要求的物理速度类型返回
  return speed;
}

::ad::physics::AngularVelocity RssCheck::GetHeadingChange(carla::client::Actor const &actor) const {
  auto const angular_velocity = actor.GetAngularVelocity();
  ::ad::physics::AngularVelocity heading_change(-1. * angular_velocity.z * to_radians);
  return heading_change;
}

::ad::physics::Angle RssCheck::GetSteeringAngle(carla::client::Vehicle const &actor) const {
  auto const steer_ratio = actor.GetControl().steer;
  ::ad::physics::Angle steering_angle(-1 * _maximum_steering_angle * steer_ratio);
  return steering_angle;
}

void RssCheck::UpdateRoute(CarlaRssState &carla_rss_state) {
  _logger->trace("Update route start: {}", carla_rss_state.ego_route);

  // 移除已走过的路线部分，尝试将路线段前置
  // (i.e. when driving backwards)
  // 尽量确保车辆的后部仍在路线范围内
  // 支持方向计算
  ::ad::map::point::ParaPointList all_lane_matches;
  for (auto reference_point :
       {::ad::map::match::ObjectReferencePoints::RearRight, ::ad::map::match::ObjectReferencePoints::RearLeft}) {
    auto const &reference_position =
        carla_rss_state.ego_match_object.mapMatchedBoundingBox.referencePointPositions[size_t(reference_point)];
    auto const para_points = ::ad::map::match::getParaPoints(reference_position);
    all_lane_matches.insert(all_lane_matches.end(), para_points.begin(), para_points.end());
  }

  auto shorten_route_result = ::ad::map::route::shortenRoute(
      all_lane_matches, carla_rss_state.ego_route,
      ::ad::map::route::ShortenRouteMode::DontCutIntersectionAndPrependIfSucceededBeforeRoute);
  if (shorten_route_result == ::ad::map::route::ShortenRouteResult::SucceededIntersectionNotCut) {
    shorten_route_result = ::ad::map::route::ShortenRouteResult::Succeeded;
  }

  bool routing_target_check_finished = false;
  while ((!_routing_targets.empty()) && (!routing_target_check_finished)) {
    auto const next_target = _routing_targets.front();
    auto const &distance_to_next_target =
        ::ad::map::point::distance(next_target, carla_rss_state.ego_match_object.enuPosition.centerPoint);
    if (distance_to_next_target < ::ad::physics::Distance(3.)) {
      _routing_targets.erase(_routing_targets.begin());
      _logger->debug("Next target reached: {}; remaining targets: {}", next_target, _routing_targets);
    } else {
      routing_target_check_finished = true;
    }
  }

  bool reroute_required = false;
  if (!_routing_targets_to_append.empty()) {
    reroute_required = true;
    _routing_targets.insert(_routing_targets.end(), _routing_targets_to_append.begin(),
                            _routing_targets_to_append.end());
    _logger->debug("Appending new routing targets: {}; resulting targets: {}", _routing_targets_to_append,
                   _routing_targets);
    _routing_targets_to_append.clear();
  }

  ::ad::physics::Distance const route_target_length(50.);

  if ((!reroute_required) && (shorten_route_result == ::ad::map::route::ShortenRouteResult::Succeeded)) {
    std::vector<::ad::map::route::FullRoute> additional_routes;
    auto const route_valid =
        ::ad::map::route::extendRouteToDistance(carla_rss_state.ego_route, route_target_length, additional_routes);

    if (route_valid) {
      if (additional_routes.size() > 0u) {
        // 对路由进行随机扩展
        std::size_t route_index = static_cast<std::size_t>(std::rand()) % (additional_routes.size() + 1);
        if (route_index < additional_routes.size()) {
          // 我们决定选择其中一条额外的路线
          _logger->debug("Additional Routes: {}->{}", additional_routes.size(), route_index);
          carla_rss_state.ego_route = additional_routes[route_index];
        } else {
          // 我们决定在路线内进行扩展，不进行任何改动
          _logger->debug("Additional Routes: expand current");
        }
      }
    } else {
      reroute_required = true;
    }
  } else {
    // 对于所有其他结果，我们重新创建路线
    reroute_required = true;
  }

  // 如有需要，创建路线
  if (reroute_required) {
    // try to create routes
    // 创建一个用于存储所有新路线的向量，每条路线的类型为::ad::map::route::FullRoute
    std::vector<::ad::map::route::FullRoute> all_new_routes;
   // 遍历自车匹配对象的地图匹配包围盒中的参考点位置（这里取的是中心位置对应的参考点集合）
    for (const auto &position :
         carla_rss_state.ego_match_object.mapMatchedBoundingBox
             .referencePointPositions[int32_t(::ad::map::match::ObjectReferencePoints::Center)]) {
            // 获取起始点信息，这里是从参考点位置中的车道点（lanePoint）里获取参数化点（paraPoint）作为起始点
      auto start_point = position.lanePoint.paraPoint;
          // 初始化一个投影后的起始点，初始值与原始起始点相同，后续可能根据一些条件进行更新
      auto projected_start_point = start_point;
      if (!::ad::map::lane::isHeadingInLaneDirection(start_point,
                                                     carla_rss_state.ego_match_object.enuPosition.heading)) {
        // 如果自车朝向与车道方向相反，在日志中输出调试信息提示
        _logger->debug("EgoVehicle heading in opposite lane direction");
       // 尝试将起始点投影到与自车朝向一致的车道方向上，若投影成功
        if (::ad::map::lane::projectPositionToLaneInHeadingDirection(
                start_point, carla_rss_state.ego_match_object.enuPosition.heading, projected_start_point)) {
          // 在日志中输出调试信息，显示投影后的车道ID，方便调试查看具体投影情况
          _logger->debug("Projected to lane {}", projected_start_point.laneId);
        }
      }
          // 在日志中输出调试信息，显示原始起始点和投影后的起始点信息，方便对比查看
      _logger->debug("Route start_point: {}, projected_start_point: {}", start_point, projected_start_point);
           // 根据投影后的起始点和自车的朝向信息创建一个用于路径规划的起始点（routing_start_point），用于后续生成路线
      auto routing_start_point = ::ad::map::route::planning::createRoutingPoint(
          projected_start_point, carla_rss_state.ego_match_object.enuPosition.heading);
          // 判断是否存在目标路由点（_routing_targets）且其是有效的
      if (!_routing_targets.empty() && ::ad::map::point::isValid(_routing_targets)) {
        // 根据起始点和目标路由点规划一条新的路线，使用指定的路线创建模式（AllRoutableLanes，可能表示可通行的所有车道）
        auto new_route = ::ad::map::route::planning::planRoute(routing_start_point, _routing_targets,
                                                               ::ad::map::route::RouteCreationMode::AllRoutableLanes);
        // 将新规划的路线添加到所有新路线的向量中
        all_new_routes.push_back(new_route);
      } else {
       // 如果没有指定目标路由点，则根据起始点、预设的路线目标长度以及指定的路线创建模式预测生成一系列路线（可能是基于一定距离延伸的多条路线）
        auto new_routes = ::ad::map::route::planning::predictRoutesOnDistance(
            routing_start_point, route_target_length, ::ad::map::route::RouteCreationMode::AllRoutableLanes);
// 遍历预测生成的每条新路线，并将它们都添加到所有新路线的向量中
        for (const auto &new_route : new_routes) {
          // 延长所有车道的路线
          all_new_routes.push_back(new_route);
        }
      }
    }
 // 在日志中输出调试信息，显示新生成的路线数量，方便查看规划情况
    _logger->debug("New routes: {}", all_new_routes.size());
// 判断是否成功生成了新的路线
    if (!all_new_routes.empty()) {
      // take a random route
     // 如果有新路线，随机选择一条路线作为最终采用的路线
      // 通过生成一个随机数作为索引来选择路线，将随机数转换为合适的size_t类型，确保索引在有效范围内（取模运算保证不会越界）
      std::size_t route_index = static_cast<std::size_t>(std::rand()) % (all_new_routes.size());
      // 将随机选择的路线赋值给自车的路线（carla_rss_state.ego_route），更新自车的行驶路线
      carla_rss_state.ego_route = all_new_routes[route_index];
    }
  }
 // 在日志中以跟踪级别（trace）输出更新路线后的结果信息，方便调试查看具体的路线情况
  _logger->trace("Update route result: {}", carla_rss_state.ego_route);
}
// 定义函数CalculateEgoDynamicsOnRoute，用于计算自车在路线上的动力学信息
// 参数包括当前时间戳、从纪元开始到检查开始的时间（以毫秒为单位）、自车车辆信息、自车匹配对象、完整的路线信息、默认的自车动力学信息以及上一次的自车动力学信息
EgoDynamicsOnRoute RssCheck::CalculateEgoDynamicsOnRoute(
    carla::client::Timestamp const &current_timestamp, double const &time_since_epoch_check_start_ms,
    carla::client::Vehicle const &carla_vehicle, ::ad::map::match::Object match_object,
    ::ad::map::route::FullRoute const &route, ::ad::rss::world::RssDynamics const &default_ego_vehicle_dynamics,
    EgoDynamicsOnRoute const &last_dynamics) const {
  // 创建一个新的自车动力学信息结构体实例，用于存储本次计算得到的动力学信息
  EgoDynamicsOnRoute new_dynamics;
 // 将传入的当前时间戳赋值给新的动力学信息结构体中的时间戳成员变量
  new_dynamics.timestamp = current_timestamp;
 // 将传入的从纪元开始到检查开始的时间赋值给新的动力学信息结构体中的对应成员变量
  new_dynamics.time_since_epoch_check_start_ms = time_since_epoch_check_start_ms;
 // 调用GetSpeed函数获取自车的速度信息，并赋值给新的动力学信息结构体中的速度成员变量
  new_dynamics.ego_speed = GetSpeed(carla_vehicle);
    // 将自车匹配对象的中心坐标赋值给新的动力学信息结构体中的自车中心坐标成员变量
  new_dynamics.ego_center = match_object.enuPosition.centerPoint;
  // 将自车匹配对象的朝向信息赋值给新的动力学信息结构体中的自车朝向成员变量
  new_dynamics.ego_heading = match_object.enuPosition.heading;
  // 调用GetHeadingChange函数获取自车的航向变化信息，并赋值给新的动力学信息结构体中的相应成员变量
  new_dynamics.ego_heading_change = GetHeadingChange(carla_vehicle);
 // 调用GetSteeringAngle函数获取自车的转向角度信息，并赋值给新的动力学信息结构体中的相应成员变量
  new_dynamics.ego_steering_angle = GetSteeringAngle(carla_vehicle);
// 根据自车匹配对象和完整路线信息获取与自车相关的路线区间信息，使用指定的路线区间创建模式（AllRouteLanes，可能表示所有车道相关的路线区间）
  auto object_route =
      ::ad::map::route::getRouteSection(match_object, route, ::ad::map::route::RouteSectionCreationMode::AllRouteLanes);
 // 获取该路线区间对应的ENU边界信息（可能是用于后续判断位置、方向等相关操作的边界数据）
  auto border = ::ad::map::route::getENUBorderOfRoute(object_route);
  new_dynamics.route_heading = ::ad::map::lane::getENUHeading(border, match_object.enuPosition.centerPoint);
 
// 根据自车匹配对象和相关路线区间查找路线的中心点信息（可能是车道中心等相关的点）
  auto const object_center = ::ad::map::route::findCenterWaypoint(match_object, object_route);
  if (object_center.isValid()) {
   // 如果找到的中心点信息是有效的
        // 获取中心点对应的位置信息（可能是车道上的某个位置点）
    auto lane_center_point = object_center.queryPosition;
   // 将该位置点转换为ENU坐标下的车道点信息
    auto lane_center_point_enu = ::ad::map::lane::getENULanePoint(lane_center_point);
   // 将该位置点转换为ENU坐标下的车道点信息
    if (std::fabs(new_dynamics.route_heading) > ::ad::map::point::ENUHeading(M_PI)) {
      // 如果实际中心点已经在外部，尝试使用这个扩展
      // 路线航向计算的物体中心
      new_dynamics.route_heading = ::ad::map::lane::getENUHeading(border, lane_center_point_enu);
    }

    if (object_center.laneSegmentIterator->laneInterval.wrongWay) {
      // 行驶在错误的车道上，因此我们必须投影到标准路线
      // 方向
      ::ad::map::lane::projectPositionToLaneInHeadingDirection(lane_center_point, new_dynamics.route_heading,
                                                               lane_center_point);
      lane_center_point_enu = ::ad::map::lane::getENULanePoint(lane_center_point);
    }
    new_dynamics.route_nominal_center = lane_center_point_enu;

  } else {
    // 自车完全偏离了路线，因此我们无法更新
    // 数值
    new_dynamics.route_nominal_center = last_dynamics.route_nominal_center;
    new_dynamics.route_heading = last_dynamics.route_heading;
  }
// 计算自车航向与路线航向的差值，并进行归一化处理（可能是将角度差值限制在一定范围内，比如 -π 到 π 之间），将结果赋值给新的动力学信息结构体中的航向差值成员变量
  new_dynamics.heading_diff =
      ::ad::map::point::normalizeENUHeading(new_dynamics.route_heading - new_dynamics.ego_heading);
 // 计算自车航向与路线航向的差值，并进行归一化处理（可能是将角度差值限制在一定范围内，比如 -π 到 π 之间），将结果赋值给新的动力学信息结构体中的航向差值成员变量
  new_dynamics.route_speed_lon =
      std::fabs(std::cos(static_cast<double>(new_dynamics.heading_diff))) * new_dynamics.ego_speed;
  // 根据航向差值和自车速度计算路线横向速度（通过三角函数，横向速度是速度在垂直于路线方向上的分量），并赋值给相应成员变量
  new_dynamics.route_speed_lat = std::sin(static_cast<double>(new_dynamics.heading_diff)) * new_dynamics.ego_speed;
 
 // 初始化一个标志变量，用于表示是否保留上一次的加速度信息，初始值设为true，表示默认保留
  bool keep_last_acceleration = true;
  // 判断上一次动力学信息中的时间戳的经过秒数是否大于0，即上一次记录是否有时间间隔
  if (last_dynamics.timestamp.elapsed_seconds > 0.) {
    // 计算本次时间戳与上一次时间戳之间的时间间隔，创建一个时间间隔类型（::ad::physics::Duration）的实例
    ::ad::physics::Duration const delta_time(current_timestamp.elapsed_seconds -
                                             last_dynamics.timestamp.elapsed_seconds);
    // 判断时间间隔是否大于一个极小值（0.0001，可能是为了避免除以极小时间间隔导致数值异常等情况）
    if (delta_time > ::ad::physics::Duration(0.0001)) {
      try {
        // 根据本次和上一次的横向路线速度以及时间间隔计算横向路线加速度，并赋值给相应成员变量
        new_dynamics.route_accel_lat = (new_dynamics.route_speed_lat - last_dynamics.route_speed_lat) / delta_time;
        // 计算横向路线平均加速度（可能是一种平滑处理方式，结合了上一次平均加速度和本次计算的加速度），并赋值给相应成员变量
        new_dynamics.avg_route_accel_lat =
            ((last_dynamics.avg_route_accel_lat * 2.) + new_dynamics.route_accel_lat) / 3.;
        // 同理，根据纵向路线速度计算纵向路线加速度，并赋值给相应成员变量
        new_dynamics.route_accel_lon = (new_dynamics.route_speed_lon - last_dynamics.route_speed_lon) / delta_time;
       // 计算纵向路线平均加速度，并赋值给相应成员变量
        new_dynamics.avg_route_accel_lon =
            ((last_dynamics.avg_route_accel_lon * 2.) + new_dynamics.route_accel_lon) / 3.;
 // 判断横向路线平均加速度是否等于0，如果等于0，为了防止出现数值下溢等问题，将其设置为0（可能是在特定的逻辑处理中避免出现意外情况）
        if (new_dynamics.avg_route_accel_lat == ::ad::physics::Acceleration(0.)) {
          // prevent from underrun
          new_dynamics.avg_route_accel_lat = ::ad::physics::Acceleration(0.);
        }
       // 同样地，判断纵向路线平均加速度是否等于0，如果等于0，进行相应的处理，将其设置为0
        if (new_dynamics.avg_route_accel_lon == ::ad::physics::Acceleration(0.)) {
          // prevent from underrun
          new_dynamics.avg_route_accel_lon = ::ad::physics::Acceleration(0.);
        }
       // 如果成功计算了加速度相关信息，将标志变量设为false，表示不需要保留上一次的加速度信息了
        keep_last_acceleration = false;
      } catch (...) {
      }
    }
  }

  if (keep_last_acceleration) {
    new_dynamics.route_accel_lat = last_dynamics.route_accel_lat;
    new_dynamics.avg_route_accel_lat = last_dynamics.avg_route_accel_lat;
    new_dynamics.route_accel_lon = last_dynamics.route_accel_lon;
    new_dynamics.avg_route_accel_lon = last_dynamics.avg_route_accel_lon;
  }

  // check if the center point (and only the center point) is still found on the
  // route
  ::ad::map::point::ParaPointList in_lane_matches;
  for (auto &match_position : match_object.mapMatchedBoundingBox
                                  .referencePointPositions[int32_t(::ad::map::match::ObjectReferencePoints::Center)]) {
    if (match_position.type == ::ad::map::match::MapMatchedPositionType::LANE_IN) {
      in_lane_matches.push_back(match_position.lanePoint.paraPoint);
    }
  }
  auto const object_in_lane_center = ::ad::map::route::findNearestWaypoint(in_lane_matches, route);
  new_dynamics.ego_center_within_route = object_in_lane_center.isValid();
  // evaluated by AnalyseResults
  new_dynamics.crossing_border = false;

  // calculate the ego stopping distance, to be able to reduce the effort

  ::ad::rss::map::RssObjectData ego_object_data;
  ego_object_data.id = ::ad::rss::world::ObjectId(0u);
  ego_object_data.type = ::ad::rss::world::ObjectType::EgoVehicle;
  ego_object_data.matchObject = match_object;
  ego_object_data.speed = new_dynamics.ego_speed;
  ego_object_data.yawRate = new_dynamics.ego_heading_change;
  ego_object_data.steeringAngle = new_dynamics.ego_steering_angle;
  ego_object_data.rssDynamics = default_ego_vehicle_dynamics;

  ad::rss::map::RssObjectConversion object_conversion(ego_object_data);
  if (!object_conversion.calculateMinStoppingDistance(new_dynamics.min_stopping_distance)) {
    _logger->error(
        "CalculateEgoDynamicsOnRoute: calculation of min stopping distance "
        "failed. Setting to 100. ({} {} {} {})",
        match_object, new_dynamics.ego_speed, new_dynamics.ego_speed, new_dynamics.ego_heading_change,
        default_ego_vehicle_dynamics);
    new_dynamics.min_stopping_distance = ::ad::physics::Distance(100.);
  }

  _logger->trace("CalculateEgoDynamicsOnRoute: route-section {} -> dynamics: {}", object_route, new_dynamics);
  return new_dynamics;
}

void RssCheck::UpdateDefaultRssDynamics(CarlaRssState &carla_rss_state) {
  ::ad::map::match::Object other_match_object;
  carla::SharedPtr<ActorConstellationData> default_constellation_data{
      new ActorConstellationData{carla_rss_state.ego_match_object, carla_rss_state.ego_route,
                                 carla_rss_state.ego_dynamics_on_route, other_match_object, nullptr}};
  auto const default_constellation_result = _actor_constellation_callback(default_constellation_data);
  carla_rss_state.default_ego_vehicle_dynamics = default_constellation_result.ego_vehicle_dynamics;
}

::ad::map::landmark::LandmarkIdSet RssCheck::GetGreenTrafficLightsOnRoute(
    std::vector<SharedPtr<carla::client::TrafficLight>> const &traffic_lights,
    ::ad::map::route::FullRoute const &route) const {
  ::ad::map::landmark::LandmarkIdSet green_traffic_lights;

  auto next_intersection = ::ad::map::intersection::Intersection::getNextIntersectionOnRoute(route);
  if (next_intersection &&
      (next_intersection->intersectionType() == ::ad::map::intersection::IntersectionType::TrafficLight)) {
    // try to guess the the relevant traffic light with the rule: nearest
    // traffic light in respect to the incoming lane.
    // @todo: when OpenDrive maps have the traffic lights incorporated, we only
    // have to fill all green traffic lights into the green_traffic_lights list
    auto incoming_lanes = next_intersection->incomingLanesOnRoute();
    // since our route spans the whole street, we have to filter out the
    // incoming lanes with wrong way flag
    auto incoming_lanes_iter = incoming_lanes.begin();
    while (incoming_lanes_iter != incoming_lanes.end()) {
      auto find_waypoint = ::ad::map::route::findWaypoint(*incoming_lanes_iter, route);
      if (find_waypoint.isValid() && find_waypoint.laneSegmentIterator->laneInterval.wrongWay) {
        incoming_lanes_iter = incoming_lanes.erase(incoming_lanes_iter);
      } else {
        incoming_lanes_iter++;
      }
    }

    ::ad::map::match::AdMapMatching traffic_light_map_matching;
    bool found_relevant_traffic_light = false;
    for (const auto &traffic_light : traffic_lights) {
      auto traffic_light_state = traffic_light->GetState();
      carla::geom::BoundingBox trigger_bounding_box = traffic_light->GetTriggerVolume();

      auto traffic_light_transform = traffic_light->GetTransform();
      auto trigger_box_location = trigger_bounding_box.location;
      traffic_light_transform.TransformPoint(trigger_box_location);

      ::ad::map::point::ENUPoint trigger_box_position;
      trigger_box_position.x = ::ad::map::point::ENUCoordinate(trigger_box_location.x);
      trigger_box_position.y = ::ad::map::point::ENUCoordinate(-1 * trigger_box_location.y);
      trigger_box_position.z = ::ad::map::point::ENUCoordinate(0.);

      _logger->trace("traffic light[{}] Position: {}", traffic_light->GetId(), trigger_box_position);
      auto traffic_light_map_matched_positions = traffic_light_map_matching.getMapMatchedPositions(
          trigger_box_position, ::ad::physics::Distance(0.25), ::ad::physics::Probability(0.1));

      _logger->trace("traffic light[{}] Map Matched Position: {}", traffic_light->GetId(),
                     traffic_light_map_matched_positions);

      for (auto matched_position : traffic_light_map_matched_positions) {
        if (incoming_lanes.find(matched_position.lanePoint.paraPoint.laneId) != incoming_lanes.end()) {
          if (found_relevant_traffic_light &&
              (green_traffic_lights.empty() && (traffic_light_state == carla::rpc::TrafficLightState::Green))) {
            _logger->warn("found another relevant traffic light on lane {}; {} state {}",
                          matched_position.lanePoint.paraPoint.laneId, traffic_light->GetId(),
                          (traffic_light_state == carla::rpc::TrafficLightState::Green) ? "green" : "not green");
          } else {
            _logger->debug("found relevant traffic light on lane {}; {} state {}",
                           matched_position.lanePoint.paraPoint.laneId, traffic_light->GetId(),
                           (traffic_light_state == carla::rpc::TrafficLightState::Green) ? "green" : "not green");
          }

          found_relevant_traffic_light = true;

          // found matching traffic light
          if (traffic_light_state == carla::rpc::TrafficLightState::Green) {
            // @todo: currently there is only this workaround because of missign
            // OpenDrive map support for actual traffic light ids
            green_traffic_lights.insert(::ad::map::landmark::LandmarkId::getMax());
          } else {
            // if the light is not green, we don't have priority
            green_traffic_lights.clear();
          }
          break;
        }
      }
    }
  }
  return green_traffic_lights;
}

RssCheck::RssObjectChecker::RssObjectChecker(RssCheck const &rss_check,
                                             ::ad::rss::map::RssSceneCreation &scene_creation,
                                             carla::client::Vehicle const &carla_ego_vehicle,
                                             CarlaRssState const &carla_rss_state,
                                             ::ad::map::landmark::LandmarkIdSet const &green_traffic_lights)
  : _rss_check(rss_check),
    _scene_creation(scene_creation),
    _carla_ego_vehicle(carla_ego_vehicle),
    _carla_rss_state(carla_rss_state),
    _green_traffic_lights(green_traffic_lights) {}

void RssCheck::RssObjectChecker::operator()(
    const carla::SharedPtr<carla::client::Actor> other_traffic_participant) const {
  try {
    auto other_match_object = _rss_check.GetMatchObject(other_traffic_participant, ::ad::physics::Distance(2.0));

    _rss_check._logger->trace("OtherVehicleMapMatching: {} {}", other_traffic_participant->GetId(),
                              other_match_object.mapMatchedBoundingBox);

    carla::SharedPtr<ActorConstellationData> actor_constellation_data{new ActorConstellationData{
        _carla_rss_state.ego_match_object, _carla_rss_state.ego_route, _carla_rss_state.ego_dynamics_on_route,
        other_match_object, other_traffic_participant}};
    auto const actor_constellation_result = _rss_check._actor_constellation_callback(actor_constellation_data);

    auto other_speed = _rss_check.GetSpeed(*other_traffic_participant);
    auto other_heading_change = _rss_check.GetHeadingChange(*other_traffic_participant);
    auto other_steering_angle = ::ad::physics::Angle(0.);

    ::ad::rss::map::RssObjectData ego_object_data;
    ego_object_data.id = _carla_ego_vehicle.GetId();
    ego_object_data.type = ::ad::rss::world::ObjectType::EgoVehicle;
    ego_object_data.matchObject = _carla_rss_state.ego_match_object;
    ego_object_data.speed = _carla_rss_state.ego_dynamics_on_route.ego_speed;
    ego_object_data.yawRate = _carla_rss_state.ego_dynamics_on_route.ego_heading_change;
    ego_object_data.steeringAngle = _carla_rss_state.ego_dynamics_on_route.ego_steering_angle;
    ego_object_data.rssDynamics = actor_constellation_result.ego_vehicle_dynamics;

    ::ad::rss::map::RssObjectData other_object_data;
    other_object_data.id = ::ad::rss::world::ObjectId(other_traffic_participant->GetId());
    other_object_data.type = actor_constellation_result.actor_object_type;
    other_object_data.matchObject = other_match_object;
    other_object_data.speed = other_speed;
    other_object_data.yawRate = other_heading_change;
    other_object_data.steeringAngle = other_steering_angle;
    other_object_data.rssDynamics = actor_constellation_result.actor_dynamics;

    _scene_creation.appendScenes(ego_object_data, _carla_rss_state.ego_route, other_object_data,
                                 actor_constellation_result.restrict_speed_limit_mode, _green_traffic_lights,
                                 actor_constellation_result.rss_calculation_mode);

  } catch (...) {
    _rss_check._logger->error("Exception processing other traffic participant {} -> Ignoring it",
                              other_traffic_participant->GetId());
  }
}

void RssCheck::CreateWorldModel(carla::client::Timestamp const &timestamp, carla::client::ActorList const &actors,
                                carla::client::Vehicle const &carla_ego_vehicle, CarlaRssState &carla_rss_state) const {
  // only loop once over the actors since always the respective objects are created
  std::vector<SharedPtr<carla::client::TrafficLight>> traffic_lights;
  std::vector<SharedPtr<carla::client::Actor>> other_traffic_participants;
  for (const auto &actor : actors) {
    const auto traffic_light = boost::dynamic_pointer_cast<carla::client::TrafficLight>(actor);
    if (traffic_light != nullptr) {
      traffic_lights.push_back(traffic_light);
      continue;
    }

    if ((boost::dynamic_pointer_cast<carla::client::Vehicle>(actor) != nullptr) ||
        (boost::dynamic_pointer_cast<carla::client::Walker>(actor) != nullptr)) {
      if (actor->GetId() == carla_ego_vehicle.GetId()) {
        continue;
      }
      auto const relevant_distance =
          std::max(static_cast<double>(carla_rss_state.ego_dynamics_on_route.min_stopping_distance), 100.);
      if (actor->GetTransform().location.Distance(carla_ego_vehicle.GetTransform().location) < relevant_distance) {
        other_traffic_participants.push_back(actor);
      }
    }
  }

  ::ad::map::landmark::LandmarkIdSet green_traffic_lights =
      GetGreenTrafficLightsOnRoute(traffic_lights, carla_rss_state.ego_route);

  ::ad::rss::map::RssSceneCreation scene_creation(timestamp.frame, carla_rss_state.default_ego_vehicle_dynamics);

#ifdef RSS_USE_TBB
  tbb::parallel_for_each(
      other_traffic_participants.begin(), other_traffic_participants.end(),
      RssObjectChecker(*this, scene_creation, carla_ego_vehicle, carla_rss_state, green_traffic_lights));
#else
  for (auto const traffic_participant : other_traffic_participants) {
    auto checker = RssObjectChecker(*this, scene_creation, carla_ego_vehicle, carla_rss_state, green_traffic_lights);
    checker(traffic_participant);
  }
#endif

  if (_road_boundaries_mode != RoadBoundariesMode::Off) {
    // add artifical objects on the road boundaries for "stay-on-road" feature
    // use 'smart' dynamics
    auto ego_vehicle_dynamics = carla_rss_state.default_ego_vehicle_dynamics;
    ego_vehicle_dynamics.alphaLat.accelMax = ::ad::physics::Acceleration(0.);

    ::ad::rss::map::RssObjectData ego_object_data;
    ego_object_data.id = carla_ego_vehicle.GetId();
    ego_object_data.type = ::ad::rss::world::ObjectType::EgoVehicle;
    ego_object_data.matchObject = _carla_rss_state.ego_match_object;
    ego_object_data.speed = _carla_rss_state.ego_dynamics_on_route.ego_speed;
    ego_object_data.yawRate = _carla_rss_state.ego_dynamics_on_route.ego_heading_change;
    ego_object_data.steeringAngle = _carla_rss_state.ego_dynamics_on_route.ego_steering_angle;
    ego_object_data.rssDynamics = ego_vehicle_dynamics;
    scene_creation.appendRoadBoundaries(ego_object_data, carla_rss_state.ego_route,
                                        // since the route always expanded, route isn't required to expand any
                                        // more
                                        ::ad::rss::map::RssSceneCreation::AppendRoadBoundariesMode::RouteOnly);
  }

  carla_rss_state.world_model = scene_creation.getWorldModel();
}

bool RssCheck::PerformCheck(CarlaRssState &carla_rss_state) const {
	// 调用carla_rss_state中的rss_check对象的calculateProperResponse函数（具体功能依赖其实现），传入世界模型、情况快照、RSS 状态快照以及用于存储最终合适响应的对象等参数
    // 执行相应的计算和判断，获取检查结果（布尔值，表示是否成功进行了合适响应的计算等）
  bool result = carla_rss_state.rss_check.calculateProperResponse(
      carla_rss_state.world_model, carla_rss_state.situation_snapshot, carla_rss_state.rss_state_snapshot,
      carla_rss_state.proper_response);

// 如果检查结果为false，即计算合适响应失败，记录警告日志，提示calculateProperResponse失败的情况
  if (!result) {
    _logger->warn("calculateProperResponse failed!");
  }
  // 如果检查结果为true，但是最终的合适响应表示当前路线不安全（isSafe为false）
  else if (!carla_rss_state.proper_response.isSafe) {
  	// 记录信息日志，输出当前不安全的路线对应的响应信息，提示当前情况不安全
    _logger->info("Unsafe route: {}", carla_rss_state.proper_response);
  }
  // 返回检查结果（布尔值），用于告知调用者此次 RSS 检查的执行情况
  return result;
}

// RssCheck类中的函数，用于分析 RSS 检查的结果，根据各种响应状态等信息判断不同的危险情况，并更新CarlaRssState对象中的相关危险状态标志等内容
void RssCheck::AnalyseCheckResults(CarlaRssState &carla_rss_state) const {
	// 初始将CarlaRssState对象中的表示整体危险状态的标志设为false，后续根据具体情况判断是否变为true
  carla_rss_state.dangerous_state = false;
  // 初始将CarlaRssState对象中的表示有危险车辆的标志设为false，后续根据具体情况判断是否变为true
  carla_rss_state.dangerous_vehicle = false;
  // 将CarlaRssState对象中表示存在相反方向危险情况的标志初始化为false，同样后续依据具体情况判断是否出现此类危险
  carla_rss_state.dangerous_opposite_state = false;
  // 初始化一个标志变量，表示左侧道路边界是否处于危险状态，初始为false，后续在分析过程中更新
  bool left_border_is_dangerous = false;
  // 初始化一个标志变量，表示右侧道路边界是否处于危险状态，初始为false，后续根据分析情况改变其值
  bool right_border_is_dangerous = false;
  // 初始化一个标志变量，用于表示是否是车辆触发了左侧响应，初始为false，会根据具体的响应状态来确定
  bool vehicle_triggered_left_response = false;
  // 初始化一个标志变量，用于表示是否是车辆触发了右侧响应，初始设为false，后续根据分析情况更新
  bool vehicle_triggered_right_response = false;
  // 初始化一个标志变量，用于表示是否是车辆触发了纵向响应，初始为false，在遍历分析响应状态时会改变其值
  bool vehicle_triggered_longitudinal_response = false;
  // 遍历CarlaRssState对象中RSS状态快照里的每个单独响应状态
  for (auto const state : carla_rss_state.rss_state_snapshot.individualResponses) {
  	// 调用::ad::rss::state::isDangerous函数（其具体实现应该是判断给定的状态是否处于危险情况）来检查当前响应状态是否危险
    if (::ad::rss::state::isDangerous(state)) {
    	// 如果当前响应状态是危险的，将CarlaRssState对象中的表示整体危险状态的标志设为true
      carla_rss_state.dangerous_state = true;
      // 记录跟踪日志，输出当前处于危险的状态信息（可能用于调试、查看具体危险情况等目的）
      _logger->trace("DangerousState: {}", state);
      // 在情况快照（situation_snapshot）中的所有情况里查找与当前危险状态对应的情况信息
        // 使用std::find_if算法结合lambda表达式来查找，lambda表达式的逻辑是比较情况的ID与当前危险状态的情况ID是否相等
      auto dangerous_sitation_iter = std::find_if(carla_rss_state.situation_snapshot.situations.begin(),
                                                  carla_rss_state.situation_snapshot.situations.end(),
                                                  [&state](::ad::rss::situation::Situation const &situation) {
                                                    return situation.situationId == state.situationId;
                                                  });
      // 如果找到了对应的情况信息（即迭代器不等于结束迭代器）
	  if (dangerous_sitation_iter != carla_rss_state.situation_snapshot.situations.end()) {
	  	// 记录跟踪日志，输出找到的对应情况信息（可能用于进一步查看相关情况细节）
        _logger->trace("Situation: {}", *dangerous_sitation_iter);
        // 如果该情况对应的对象ID与右侧道路边界对象的ID相等（通过::ad::rss::map::RssSceneCreator::getRightBorderObjectId获取右侧边界对象ID）
		if (dangerous_sitation_iter->objectId == ::ad::rss::map::RssSceneCreator::getRightBorderObjectId()) {
			// 则将右侧道路边界处于危险状态的标志设为true，表示右侧边界存在危险情况
          right_border_is_dangerous = true;
          // 如果该情况对应的对象ID与左侧道路边界对象的ID相等（通过::ad::rss::map::RssSceneCreator::getLeftBorderObjectId获取左侧边界对象ID）
        } else if (dangerous_sitation_iter->objectId == ::ad::rss::map::RssSceneCreator::getLeftBorderObjectId()) {
          // 则将左侧道路边界处于危险状态的标志设为true，意味着左侧边界有危险情况
		  left_border_is_dangerous = true;
		  // 如果情况对应的对象不是道路边界（即可能是其他交通参与者等情况）
        } else {
        	// 将表示存在危险车辆的标志设为true，说明有车辆相关的危险情况存在
          carla_rss_state.dangerous_vehicle = true;
          // 如果当前危险状态中的纵向状态响应不是无响应（::ad::rss::state::LongitudinalResponse::None表示无响应）
          if (state.longitudinalState.response != ::ad::rss::state::LongitudinalResponse::None) {
            // 将表示车辆触发纵向响应的标志设为true，说明车辆的纵向运动情况触发了相应响应
			vehicle_triggered_longitudinal_response = true;
          }
          // 如果当前危险状态中的左侧横向状态响应不是无响应
          if (state.lateralStateLeft.response != ::ad::rss::state::LateralResponse::None) {
            // 将表示车辆触发左侧响应的标志设为true，意味着车辆的左侧横向运动情况触发了响应
			vehicle_triggered_left_response = true;
          }
           // 如果当前危险状态中的右侧横向状态响应不是无响应
          if (state.lateralStateRight.response != ::ad::rss::state::LateralResponse::None) {
            // 将表示车辆触发右侧响应的标志设为true，表明车辆的右侧横向运动情况触发了响应
			vehicle_triggered_right_response = true;
          }
        }
        // 如果该情况的类型是相反方向情况（::ad::rss::situation::SituationType::OppositeDirection表示相反方向情况类型）
        if (dangerous_sitation_iter->situationType == ::ad::rss::situation::SituationType::OppositeDirection) {
          // 将表示存在相反方向危险情况的标志设为true，提示有来自相反方向的危险情况出现
		  carla_rss_state.dangerous_opposite_state = true;
        }
      }
    }
  }

  // border are restricting potentially too much, fix this
  // 如果不是车辆触发纵向响应（vehicle_triggered_longitudinal_response为false），并且最终合适响应中的纵向响应不是无响应（即有纵向响应限制）
  if (!vehicle_triggered_longitudinal_response &&
      (carla_rss_state.proper_response.longitudinalResponse != ::ad::rss::state::LongitudinalResponse::None)) {
    // 记录调试日志，提示纵向响应只是由边界触发的情况，这种情况需要忽略该纵向响应（可能因为不符合预期的合理触发条件等原因）
	_logger->debug("!! longitudinalResponse only triggered by borders: ignore !!");
    // 将最终合适响应中的纵向响应设为无响应，即去除不合理的纵向响应限制
	carla_rss_state.proper_response.longitudinalResponse = ::ad::rss::state::LongitudinalResponse::None;
    // 将最终合适响应中的纵向加速度限制范围的最大值设为默认自身车辆动力学参数中的纵向最大加速度值
    // 这样就恢复到默认的纵向加速度可允许的最大值，避免了过度限制
	carla_rss_state.proper_response.accelerationRestrictions.longitudinalRange.maximum =
        carla_rss_state.default_ego_vehicle_dynamics.alphaLon.accelMax;
  }
  // 如果不是车辆触发左侧响应（vehicle_triggered_left_response为false），并且左侧边界不是处于危险状态（left_border_is_dangerous为false），同时最终合适响应中的左侧横向响应不是无响应（即有左侧横向响应限制）
  if (!vehicle_triggered_left_response && !left_border_is_dangerous &&
      (carla_rss_state.proper_response.lateralResponseLeft != ::ad::rss::state::LateralResponse::None)) {
    // 记录调试日志，提示左侧横向响应只是由右侧边界触发的情况，需要忽略该左侧横向响应（可能是不合理的触发导致的限制）
	_logger->debug("!! lateralResponseLeft only triggered by right border: ignore !!");
     // 将最终合适响应中的左侧横向响应设为无响应，去除该不合理的左侧横向响应限制
	carla_rss_state.proper_response.lateralResponseLeft = ::ad::rss::state::LateralResponse::None;
    // 将最终合适响应中的左侧横向加速度限制范围的最大值设为默认自身车辆动力学参数中的横向最大加速度值
    // 恢复到默认的横向加速度可允许的最大值，避免因不合理触发导致的过度限制
	carla_rss_state.proper_response.accelerationRestrictions.lateralLeftRange.maximum =
        carla_rss_state.default_ego_vehicle_dynamics.alphaLat.accelMax;
    // 将车辆在路线上的动力学参数中的跨越边界标志设为true，表示出现了可能跨越边界相关的不合理情况（具体含义需结合整体关于边界跨越的逻辑来看）
	carla_rss_state.ego_dynamics_on_route.crossing_border = true;
  }
  // 如果不是车辆触发右侧响应（vehicle_triggered_right_response为false），并且右侧边界不是处于危险状态（right_border_is_dangerous为false），同时最终合适响应中的右侧横向响应不是无响应（即有右侧横向响应限制）
  if (!vehicle_triggered_right_response && !right_border_is_dangerous &&
      (carla_rss_state.proper_response.lateralResponseRight != ::ad::rss::state::LateralResponse::None)) {
    // 记录调试日志，提示右侧横向响应只是由左侧边界触发的情况，需要忽略该右侧横向响应（同样是因为可能是不合理触发导致的限制）
	_logger->debug("!! lateralResponseRight only triggered by left border: ignore !!");
    // 将最终合适响应中的右侧横向响应设为无响应，去除不合理的右侧横向响应限制
	carla_rss_state.proper_response.lateralResponseRight = ::ad::rss::state::LateralResponse::None;
	// 将最终合适响应中的右侧横向加速度限制范围的最大值设为默认自身车辆动力学参数中的横向最大加速度值
    // 恢复到默认的横向加速度可允许的最大值，避免过度限制
    carla_rss_state.proper_response.accelerationRestrictions.lateralRightRange.maximum =
        carla_rss_state.default_ego_vehicle_dynamics.alphaLat.accelMax;
        // 将车辆在路线上的动力学参数中的跨越边界标志设为true，意味着出现了可能跨越边界相关的不合理情况
    carla_rss_state.ego_dynamics_on_route.crossing_border = true;
  }
// 记录跟踪日志，输出最终的路线响应信息（可能用于查看最终整体的响应情况，方便调试、分析等）
  _logger->trace("RouteResponse: {}", carla_rss_state.proper_response);
}


}  // namespace rss
}  // namespace carla
