// Copyright (c) 2019-2020 Intel Corporation
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
// 预处理指令，确保头文件只被编译一次，避免重复定义等问题
#include <spdlog/spdlog.h>
// 引入spdlog库的头文件，用于日志记录功能
#include <ad/map/landmark/LandmarkIdSet.hpp>
#include <ad/map/match/Object.hpp>
#include <ad/map/route/FullRoute.hpp>
#include <ad/rss/core/RssCheck.hpp>
#include <ad/rss/map/RssSceneCreation.hpp>
#include <ad/rss/situation/SituationSnapshot.hpp>
#include <ad/rss/state/ProperResponse.hpp>
#include <ad/rss/state/RssStateSnapshot.hpp>
// 引入一系列与地图、RSS（Responsibility-Sensitive Safety，一种安全相关的概念）相关的自定义头文件，
// 包含地标、地图匹配对象、完整路线、RSS检查核心、RSS场景创建、情况快照、合适响应、RSS状态快照等相关类型的定义
#include <iostream>
// 引入输入输出流相关的标准库头文件，用于后续进行控制台等输出操作
#include <memory>
// 引入内存管理相关的标准库头文件，用于智能指针等操作
#include <mutex>
// 引入互斥锁相关的标准库头文件，用于多线程环境下的资源保护等操作
#include "carla/client/ActorList.h"
#include "carla/client/Vehicle.h"
#include "carla/road/Map.h"
// 引入CARLA客户端相关的头文件，用于处理CARLA中的演员列表、车辆以及地图等相关对象和功能
namespace carla {
namespace rss {
// 在carla命名空间内再定义rss命名空间，进一步细分组织代码元素，这里用于存放和RSS相关的代码逻辑
///结构体定义道路边界的不同支持处理
enum class RoadBoundariesMode {
  Off,  ///  RSS 检查不考虑道路边界
  On    /// 当前路线的道路边界会在RSS检查中考虑
};

/// @brief struct defining the ego vehicles current dynamics in respect to the
/// current route
///
/// Especially the velocity of the vehicle is split into lateral and
/// longitudinal components
/// according to the current route
///
struct EgoDynamicsOnRoute {
  /// @brief constructor
  EgoDynamicsOnRoute();
// 结构体的默认构造函数声明，用于初始化该结构体的对象，具体的初始化逻辑可能在对应的.cpp文件中定义
  /// 最后一次计算的Carla时间戳
  carla::client::Timestamp timestamp;
  ///在checkObjects调用开始时，从epoch开始的时间，单位为毫秒
  double time_since_epoch_check_start_ms;
  ///在checkObjects调用结束时，从epoch开始的时间，单位为毫秒
  double time_since_epoch_check_end_ms;
  /// 自主车辆的速度
  ::ad::physics::Speed ego_speed;
  /// 当前最小停止距离
  ::ad::physics::Distance min_stopping_distance;
  /// 考虑自主车辆的位置
  ::ad::map::point::ENUPoint ego_center;
  /// 考虑自主车辆的航向
  ::ad::map::point::ENUHeading ego_heading;
  /// 考虑自主车辆的方向变化
  ::ad::physics::AngularVelocity ego_heading_change;
  /// 考虑自主车辆的转向角度
  ::ad::physics::Angle ego_steering_angle;
  /// 检查自我中心是否在路线内
  bool ego_center_within_route;
  /// @brief flag indicating if the current state is already crossing one of the
  /// borders
  /// this is only evaluated if the border checks are active!
  /// It is a hint to oversteer a bit on countersteering
  bool crossing_border;
// 布尔值类型的标志位，用于指示当前状态是否已经越过了（路线的）边界之一，不过只有在边界检查处于激活状态时才会进行评估，
  // 可以作为在反向转向时适当多转一点角度的提示信息（比如车辆快压线了，可能需要多打点方向来纠正）
  /// @brief the considered heading of the route
  ::ad::map::point::ENUHeading route_heading;
// 表示所考虑的路线的航向，也就是路线本身的方向角度情况，同样基于ENU坐标系，对于分析车辆与路线的相对方向关系很重要
  /// @brief the considered nominal center of the current route
  ::ad::map::point::ENUPoint route_nominal_center;
  /// @brief the considered heading diff towards the route
  ::ad::map::point::ENUHeading heading_diff;
// 表示当前路线所考虑的名义中心位置，一般是路线的一个理论上的中心参考点，以ENU坐标系下的坐标点形式存在，
  // 有助于确定路线在地图中的大致位置等情况
  /// @brief the ego speed component lat in respect to a route
  ::ad::physics::Speed route_speed_lat;
// 表示车辆相对于路线的横向速度分量，即将车辆速度分解到与路线垂直方向上的速度部分，对于分析车辆在路线横向的运动情况有帮助
  /// @brief the ego speed component lon in respect to a route
  ::ad::physics::Speed route_speed_lon;
// 表示车辆相对于路线的纵向速度分量，即将车辆速度分解到与路线平行方向上的速度部分，用于了解车辆沿着路线前进或后退的速度情况
  /// @brief the ego acceleration component lat in respect to a route
  ::ad::physics::Acceleration route_accel_lat;
  /// @brief the ego acceleration component lon in respect to a route
  ::ad::physics::Acceleration route_accel_lon;
  /// @brief the ego acceleration component lat in respect to a route
  /// smoothened by an average filter
  ::ad::physics::Acceleration avg_route_accel_lat;
// 表示车辆相对于路线的横向加速度分量，经过平均滤波器平滑处理后的结果，平滑处理后的加速度数据可能更稳定，
  // 能减少噪声等因素对加速度分析的影响
  /// @brief the ego acceleration component lon in respect to a route
  /// smoothened by an average filter
  ::ad::physics::Acceleration avg_route_accel_lon;
// 表示车辆相对于路线的纵向加速度分量，经过平均滤波器平滑处理后的结果
};

/// @brief Struct defining the configuration for RSS processing of a given actor
///
/// The RssSensor implementation allows to configure the actors individually
/// for every frame
///
struct ActorConstellationResult {
  ///要应用于参与者的计算模式
  ::ad::rss::map::RssMode rss_calculation_mode{::ad::rss::map::RssMode::NotRelevant};

  ///限速模式
  ::ad::rss::map::RssSceneCreation::RestrictSpeedLimitMode restrict_speed_limit_mode{
      ::ad::rss::map::RssSceneCreation::RestrictSpeedLimitMode::None};

   ///应用于ego车辆的Rss动态
  ::ad::rss::world::RssDynamics ego_vehicle_dynamics;

  ///要用于参与者的Rss对象类型
  ::ad::rss::world::ObjectType actor_object_type;

  ///应用于参与者的Rss动态
  ::ad::rss::world::RssDynamics actor_dynamics;
};

struct ActorConstellationData {
  ///  ego map匹配的信息
  ::ad::map::match::Object ego_match_object;

  /// @brief the ego route
  ::ad::map::route::FullRoute ego_route;
// 存储自主车辆（ego）的完整路线信息，通过::ad::map::route::FullRoute类型表示，包含路线的起点、终点、途经点等详细路线相关内容

  //通过EgoDynamicsOnRoute结构体来体现自主车辆在当前行驶路线上的各种动力学相关属性情况，
  // 如速度、加速度、位置、航向等方面的具体数值和状态
  //路线上自主车辆的动力学
  EgoDynamicsOnRoute ego_dynamics_on_route;

  /// @brief the other object's map matched information
  ::ad::map::match::Object other_match_object;
// 存储其他对象（除自主车辆外）在地图上匹配得到的相关对象信息，格式和自主车辆的地图匹配对象信息类似，用于分析其他交通参与者的情况
  carla::SharedPtr<carla::client::Actor> other_actor;
// 以智能指针（SharedPtr）的形式指向carla::client::Actor类型的其他参与者对象，方便对其他参与者进行操作并管理其内存生命周期
};

/// @brief class implementing the actual RSS checks based on CARLA world
/// description
class RssCheck {
public:
  using ActorConstellationCallbackFunctionType =
      std::function<::carla::rss::ActorConstellationResult(carla::SharedPtr<ActorConstellationData>)>;
// 定义一个函数类型别名ActorConstellationCallbackFunctionType，它表示一个函数指针类型，
  // 该函数接受一个指向ActorConstellationData类型的智能指针参数，返回ActorConstellationResult类型的结果，
  // 通常可用于定义回调函数的类型规范，方便在不同地方按照此类型要求传入相应的回调函数
  /// @brief default constructor with default internal default actor constellation callback
  RssCheck(float max_steering_angle);
// 默认构造函数声明，接受一个表示最大转向角度的浮点数参数，用于初始化类对象，内部可能会设置一些默认的与演员星座
  // 这里应该是和参与RSS检查的相关参与者配置等情况相关的概念）回调相关的默认行为，具体实现在.cpp文件中
  /// @brief constructor with actor constellation callback
  RssCheck(float max_steering_angle, ActorConstellationCallbackFunctionType rss_actor_constellation_callback,
           carla::SharedPtr<carla::client::Actor> const &carla_ego_actor);
// 另一个构造函数声明，除了最大转向角度参数外，还接受一个前面定义的函数类型的回调函数参数以及一个指向CARLA客户端中自主车辆（ego actor）
  // 的智能指针参数，用于更灵活地根据传入的回调函数等设置来初始化类对象
  /// @brief destructor
  ~RssCheck();
// 析构函数声明，用于在类对象销毁时进行资源释放等清理操作，比如释放一些在类内部动态分配的内存等
  /// @brief get the logger of this
  std::shared_ptr<spdlog::logger> GetLogger() {
    return _logger;
  }
// 获取类内部的日志记录器（logger）对象的函数，返回一个指向spdlog::logger类型的共享智能指针
  /// @brief main function to trigger the RSS check at a certain point in time
  ///
  /// This function has to be called cyclic with increasing timestamps to ensure
  /// proper RSS evaluation.
  ///
  bool CheckObjects(carla::client::Timestamp const &timestamp, carla::SharedPtr<carla::client::ActorList> const &actors,
                    carla::SharedPtr<carla::client::Actor> const &carla_ego_actor,
                    ::ad::rss::state::ProperResponse &output_response,
                    ::ad::rss::state::RssStateSnapshot &output_rss_state_snapshot,
                    ::ad::rss::situation::SituationSnapshot &output_situation_snapshot,
                    ::ad::rss::world::WorldModel &output_world_model,
                    EgoDynamicsOnRoute &output_rss_ego_dynamics_on_route);
// 主要的函数声明，用于在特定时间点触发RSS检查操作，该函数需要按照时间戳递增的方式周期性调用，以确保RSS评估的正确性，
  // 接受多个参数，包括时间戳、演员列表智能指针、自主车辆智能指针以及多个用于输出不同RSS相关检查结果的引用参数（如合适响应、RSS状态快照、
  // 情况快照、世界模型以及自主车辆在路线上的动力学情况等），返回一个布尔值表示检查结果是否符合某种条件（可能是是否安全等情况）
  /// @returns the used vehicle dynamics for ego vehicle
  const ::ad::rss::world::RssDynamics &GetDefaultActorConstellationCallbackEgoVehicleDynamics() const;
  /// @brief sets the vehicle dynamics to be used for the ego vehicle
  void SetDefaultActorConstellationCallbackEgoVehicleDynamics(
      const ::ad::rss::world::RssDynamics &ego_vehicle_dynamics);
  /// @returns the used vehicle dynamics for other vehicles
  const ::ad::rss::world::RssDynamics &GetDefaultActorConstellationCallbackOtherVehicleDynamics() const;
  /// @brief sets the vehicle dynamics to be used for other vehicles
  void SetDefaultActorConstellationCallbackOtherVehicleDynamics(
      const ::ad::rss::world::RssDynamics &other_vehicle_dynamics);
  /// @returns the used vehicle dynamics for pedestrians
  const ::ad::rss::world::RssDynamics &GetDefaultActorConstellationCallbackPedestrianDynamics() const;
  /// @brief sets the dynamics to be used for pedestrians
  void SetDefaultActorConstellationCallbackPedestrianDynamics(const ::ad::rss::world::RssDynamics &pedestrian_dynamics);

  /// @brief sets the current log level
  void SetLogLevel(const spdlog::level::level_enum &log_level);

  /// @brief sets the current log level
  void SetMapLogLevel(const spdlog::level::level_enum &map_log_level);

  /// @returns the current mode for respecting the road boundaries (@see also
  /// RssSensor::GetRoadBoundariesMode())
  const ::carla::rss::RoadBoundariesMode &GetRoadBoundariesMode() const;
  /// @brief sets the current mode for respecting the road boundaries (@see also
  /// RssSensor::SetRoadBoundariesMode())
  void SetRoadBoundariesMode(const ::carla::rss::RoadBoundariesMode &road_boundaries_mode);

  /// @returns the current routing targets (@see also
  /// RssSensor::GetRoutingTargets())
  const std::vector<::carla::geom::Transform> GetRoutingTargets() const;
  /// @brief appends a routing target to the current routing target list (@see
  /// also RssSensor::AppendRoutingTargets())
  void AppendRoutingTarget(const ::carla::geom::Transform &routing_target);
  /// @brief resets the current routing targets (@see also
  /// RssSensor::ResetRoutingTargets())
  void ResetRoutingTargets();

  /// @brief drop the current route
  ///
  /// Afterwards a new route is selected randomly (if multiple routes are
  /// possible).
  ///
  void DropRoute();

  /// @returns the default vehicle dynamics
  static ::ad::rss::world::RssDynamics GetDefaultVehicleDynamics();

  /// @returns the default pedestrian dynamics
  static ::ad::rss::world::RssDynamics GetDefaultPedestrianDynamics();

  /// @returns the default road boundaries mode
  static RoadBoundariesMode GetDefaultRoadBoundariesMode() {
    return RoadBoundariesMode::Off;
  }

private:
  /// @brief standard logger
  std::shared_ptr<spdlog::logger> _logger;
  /// @brief logger for timing log messages
  std::shared_ptr<spdlog::logger> _timing_logger;

  /// @brief maximum steering angle
  float _maximum_steering_angle;

  /// @brief current used vehicle dynamics for ego vehicle by the default actor constellation callback
  ::ad::rss::world::RssDynamics _default_actor_constellation_callback_ego_vehicle_dynamics;
  /// @brief current used vehicle dynamics for other vehicle by the default actor constellation callback
  ::ad::rss::world::RssDynamics _default_actor_constellation_callback_other_vehicle_dynamics;
  /// @brief current used vehicle dynamics for pedestrians by the default actor constellation callback
  ::ad::rss::world::RssDynamics _default_actor_constellation_callback_pedestrian_dynamics;

  /// @brief the current actor constellation callback
  ActorConstellationCallbackFunctionType _actor_constellation_callback;

  /// @brief current used road boundaries mode
  ::carla::rss::RoadBoundariesMode _road_boundaries_mode;
  /// @brief current used routing targets
  std::vector<::ad::map::point::ENUPoint> _routing_targets;
  /// @brief routing targets to be appended next run
  std::vector<::ad::map::point::ENUPoint> _routing_targets_to_append;

  /// @brief struct collecting the rss states required
  struct CarlaRssState {
    /// @brief the actual RSS checker object
    ::ad::rss::core::RssCheck rss_check;

    /// @brief the ego map matched information
    ::ad::map::match::Object ego_match_object;

    /// @brief the ego route
    ::ad::map::route::FullRoute ego_route;

    /// @brief the ego dynamics on the route
    EgoDynamicsOnRoute ego_dynamics_on_route;

    /// @brief current used default vehicle dynamics for ego vehicle
    ::ad::rss::world::RssDynamics default_ego_vehicle_dynamics;

    /// @brief check input: the RSS world model
    ::ad::rss::world::WorldModel world_model;

    /// @brief check result: the situation snapshot
    ::ad::rss::situation::SituationSnapshot situation_snapshot;
    /// @brief check result: the rss state snapshot
    ::ad::rss::state::RssStateSnapshot rss_state_snapshot;
    /// @brief check result: the proper response
    ::ad::rss::state::ProperResponse proper_response;
    /// @brief flag indicating if the current state is overall dangerous
    bool dangerous_state;
    /// @brief flag indicating if the current state is dangerous because of a
    /// vehicle
    bool dangerous_vehicle;
    /// @brief flag indicating if the current state is dangerous because of an
    /// opposite vehicle
    bool dangerous_opposite_state;
  };

  class RssObjectChecker {
  public:
    RssObjectChecker(RssCheck const &rss_check, ::ad::rss::map::RssSceneCreation &scene_creation,
                     carla::client::Vehicle const &carla_ego_vehicle, CarlaRssState const &carla_rss_state,
                     ::ad::map::landmark::LandmarkIdSet const &green_traffic_lights);
    void operator()(const carla::SharedPtr<carla::client::Actor> other_traffic_participant) const;

  private:
    RssCheck const &_rss_check;
    ::ad::rss::map::RssSceneCreation &_scene_creation;
    carla::client::Vehicle const &_carla_ego_vehicle;
    CarlaRssState const &_carla_rss_state;
    ::ad::map::landmark::LandmarkIdSet const &_green_traffic_lights;
  };

  friend class RssObjectChecker;

  /// @brief the current state of the ego vehicle
  CarlaRssState _carla_rss_state;

  /// @brief calculate the map matched object from the actor
  ::ad::map::match::Object GetMatchObject(carla::SharedPtr<carla::client::Actor> const &actor,
                                          ::ad::physics::Distance const &sampling_distance) const;

  /// @brief calculate the speed from the actor
  ::ad::physics::Speed GetSpeed(carla::client::Actor const &actor) const;

  /// @brief calculate the heading change from the actor
  ::ad::physics::AngularVelocity GetHeadingChange(carla::client::Actor const &actor) const;

  /// @brief calculate the steering angle from the actor
  ::ad::physics::Angle GetSteeringAngle(carla::client::Vehicle const &actor) const;

  /// @brief update the desired ego vehicle route
  void UpdateRoute(CarlaRssState &carla_rss_state);

  /// @brief calculate ego vehicle dynamics on the route
  EgoDynamicsOnRoute CalculateEgoDynamicsOnRoute(carla::client::Timestamp const &current_timestamp,
                                                 double const &time_since_epoch_check_start_ms,
                                                 carla::client::Vehicle const &carla_vehicle,
                                                 ::ad::map::match::Object match_object,
                                                 ::ad::map::route::FullRoute const &route,
                                                 ::ad::rss::world::RssDynamics const &default_ego_vehicle_dynamics,
                                                 EgoDynamicsOnRoute const &last_dynamics) const;

  void UpdateDefaultRssDynamics(CarlaRssState &carla_rss_state);

  /// @brief collect the green traffic lights on the current route
  ::ad::map::landmark::LandmarkIdSet GetGreenTrafficLightsOnRoute(
      std::vector<SharedPtr<carla::client::TrafficLight>> const &traffic_lights,
      ::ad::map::route::FullRoute const &route) const;

  /// @brief Create the RSS world model
  void CreateWorldModel(carla::client::Timestamp const &timestamp, carla::client::ActorList const &actors,
                        carla::client::Vehicle const &carla_ego_vehicle, CarlaRssState &carla_rss_state) const;

  /// @brief Perform the actual RSS check
  bool PerformCheck(CarlaRssState &carla_rss_state) const;

  /// @brief Analyse the RSS check results
  void AnalyseCheckResults(CarlaRssState &carla_rss_state) const;
};

}  // namespace rss
}  // namespace carla

namespace std {
/**
 * \brief standard ostream operator
 *
 * \param[in/out] os The output stream to write to
 * \param[in] ego_dynamics_on_route the ego dynamics on route to stream out
 *
 * \returns The stream object.
 *
 */
inline std::ostream &operator<<(std::ostream &out, const ::carla::rss::EgoDynamicsOnRoute &ego_dynamics_on_route) {
  out << "EgoDynamicsOnRoute(timestamp=" << ego_dynamics_on_route.timestamp
      << ", time_since_epoch_check_start_ms=" << ego_dynamics_on_route.time_since_epoch_check_start_ms
      << ", time_since_epoch_check_end_ms=" << ego_dynamics_on_route.time_since_epoch_check_end_ms
      << ", ego_speed=" << ego_dynamics_on_route.ego_speed
      << ", min_stopping_distance=" << ego_dynamics_on_route.min_stopping_distance
      << ", ego_center=" << ego_dynamics_on_route.ego_center << ", ego_heading=" << ego_dynamics_on_route.ego_heading
      << ", ego_heading_change=" << ego_dynamics_on_route.ego_heading_change
      << ", ego_steering_angle=" << ego_dynamics_on_route.ego_steering_angle
      << ", ego_center_within_route=" << ego_dynamics_on_route.ego_center_within_route
      << ", crossing_border=" << ego_dynamics_on_route.crossing_border
      << ", route_heading=" << ego_dynamics_on_route.route_heading
      << ", route_nominal_center=" << ego_dynamics_on_route.route_nominal_center
      << ", heading_diff=" << ego_dynamics_on_route.heading_diff
      << ", route_speed_lat=" << ego_dynamics_on_route.route_speed_lat
      << ", route_speed_lon=" << ego_dynamics_on_route.route_speed_lon
      << ", route_accel_lat=" << ego_dynamics_on_route.route_accel_lat
      << ", route_accel_lon=" << ego_dynamics_on_route.route_accel_lon
      << ", avg_route_accel_lat=" << ego_dynamics_on_route.avg_route_accel_lat
      << ", avg_route_accel_lon=" << ego_dynamics_on_route.avg_route_accel_lon << ")";
  return out;
}

/**
 * \brief standard ostream operator
 *
 * \param[in/out] os The output stream to write to
 * \param[in] actor_constellation_result the actor constellation result to stream out
 *
 * \returns The stream object.
 *
 */
inline std::ostream &operator<<(std::ostream &out,
                                const ::carla::rss::ActorConstellationResult &actor_constellation_result) {
  out << "ActorConstellationResult(rss_calculation_mode=" << actor_constellation_result.rss_calculation_mode
      << ", restrict_speed_limit_mode=" << actor_constellation_result.restrict_speed_limit_mode
      << ", ego_vehicle_dynamics=" << actor_constellation_result.ego_vehicle_dynamics
      << ", actor_object_type=" << actor_constellation_result.actor_object_type
      << ", actor_dynamics=" << actor_constellation_result.actor_dynamics << ")";
  return out;
}

/**
 * \brief standard ostream operator
 *
 * \param[in/out] os The output stream to write to
 * \param[in] actor_constellation_result the actor constellation result to stream out
 *
 * \returns The stream object.
 *
 */
inline std::ostream &operator<<(std::ostream &out,
                                const ::carla::rss::ActorConstellationData &actor_constellation_data) {
  out << "ActorConstellationData(";
  if (actor_constellation_data.other_actor != nullptr) {
    out << "actor_id=" << actor_constellation_data.other_actor->GetId()
        << ", actor_dynamics=" << actor_constellation_data.other_match_object << ", ";
  }
  out << "ego_match_object=" << actor_constellation_data.ego_match_object
      << ", ego_route=" << actor_constellation_data.ego_route
      << ", ego_dynamics_on_route=" << actor_constellation_data.ego_dynamics_on_route << ")";
  return out;
}

}  // namespace std
