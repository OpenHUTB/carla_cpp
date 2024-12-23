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
// 函数声明，用于丢弃当前的路线信息，具体实现可能是清除与当前路线相关的数据、重置路线相关的状态等操作，
// 后续可能会根据需要重新选择或生成新的路线（例如在存在多条可选路线的情况下随机选择等）
  void DropRoute();

  /// @returns the default vehicle dynamics
// 静态函数声明，返回默认的车辆动力学信息，以 ::ad::rss::world::RssDynamics 类型返回，这里的默认车辆动力学应该是指在没有特殊设置情况下车辆所遵循的动力学相关属性配置，
// 比如速度、加速度等方面的默认参数设置情况，供其他地方使用该默认值进行相关计算或判断等操作
  static ::ad::rss::world::RssDynamics GetDefaultVehicleDynamics();

  /// @returns the default pedestrian dynamics
// 静态函数声明，返回默认的行人动力学信息，同样以 ::ad::rss::world::RssDynamics 类型返回，虽然行人与车辆在动力学表现上有很大不同，但在这里统一用类似的类型表示，
// 该函数提供了在没有特别配置时行人所对应的动力学相关属性默认值，方便在涉及行人的RSS相关处理中使用这些默认设置来进行计算等操作
  static ::ad::rss::world::RssDynamics GetDefaultPedestrianDynamics();

  /// @returns the default road boundaries mode
  static RoadBoundariesMode GetDefaultRoadBoundariesMode() {
    return RoadBoundariesMode::Off;
  }

private:
  /// @brief standard logger
// 定义一个标准的日志记录器对象的智能指针，用于记录一般性的日志信息，方便在代码执行过程中输出调试信息、运行状态信息、错误信息等，
    // 通过 spdlog 库来实现日志功能，不同的日志级别（如 debug、info、warn、error 等）可以控制输出哪些内容
  std::shared_ptr<spdlog::logger> _logger;
  /// @brief logger for timing log messages
// 定义一个专门用于记录时间相关日志消息的日志记录器对象的智能指针，可能用于记录代码中各个操作、函数调用等所花费的时间情况，
    // 便于进行性能分析、时间消耗统计等，同样借助 spdlog 库实现
  std::shared_ptr<spdlog::logger> _timing_logger;

  /// @brief maximum steering angle
// 定义一个浮点数变量，用于表示最大转向角度，这个角度值可能会在车辆操控、路径规划、RSS 相关安全检查等涉及车辆转向情况的计算中起到限制或参考作用，
    // 比如判断车辆是否超过了允许的最大转向范围等
  float _maximum_steering_angle;

  /// @brief current used vehicle dynamics for ego vehicle by the default actor constellation callback
// 定义一个 ::ad::rss::world::RssDynamics 类型的变量，用于存储在默认的演员星座回调（default actor constellation callback）中应用于自主车辆（ego vehicle）的当前车辆动力学信息，
    // 也就是在默认回调机制下自主车辆所使用的动力学相关属性设置情况，像速度、加速度等具体参数的当前取值情况
  ::ad::rss::world::RssDynamics _default_actor_constellation_callback_ego_vehicle_dynamics;
  /// @brief current used vehicle dynamics for other vehicle by the default actor constellation callback
  ::ad::rss::world::RssDynamics _default_actor_constellation_callback_other_vehicle_dynamics;
  /// @brief current used vehicle dynamics for pedestrians by the default actor constellation callback
  ::ad::rss::world::RssDynamics _default_actor_constellation_callback_pedestrian_dynamics;

  /// @brief the current actor constellation callback
// 定义一个函数类型的变量（通过类型别名 ActorConstellationCallbackFunctionType 定义的函数指针类型），用于存储当前的演员星座回调函数，
    // 这个回调函数可以根据传入的相关演员配置数据（ActorConstellationData 类型）返回对应的演员星座结果（ActorConstellationResult 类型），
    // 在不同的场景或配置下可以灵活地替换这个回调函数来实现不同的 RSS 相关处理逻辑
  ActorConstellationCallbackFunctionType _actor_constellation_callback;

  /// @brief current used road boundaries mode
// 定义一个 RoadBoundariesMode 类型的变量，用于存储当前所使用的道路边界模式，即当前 RSS 相关检查和计算是按照考虑道路边界还是不考虑道路边界的模式在运行，
    // 其取值会根据实际设置情况在 RoadBoundariesMode 枚举定义的 Off 和 On 之间变化
  ::carla::rss::RoadBoundariesMode _road_boundaries_mode;
  /// @brief current used routing targets
  std::vector<::ad::map::point::ENUPoint> _routing_targets;
  /// @brief routing targets to be appended next run
// 定义一个同样是向量类型的变量，元素类型也是 ::ad::map::point::ENUPoint，用于存储下一次运行时要添加的路由目标点，
    // 可以理解为是一个待添加的目标点队列，在合适的时机（比如下一轮路径规划、更新路由等操作时）会将这些目标点添加到当前的路由目标集合中
  std::vector<::ad::map::point::ENUPoint> _routing_targets_to_append;

  /// @brief struct collecting the rss states required
  struct CarlaRssState {
    /// @brief the actual RSS checker object
// 定义一个 ::ad::rss::core::RssCheck 类型的变量，代表实际的 RSS 检查器对象，用于执行 RSS 相关的核心检查逻辑，
        // 比如判断车辆之间是否存在碰撞风险、是否符合安全规则等 RSS 相关的安全检查操作
    ::ad::rss::core::RssCheck rss_check;

    /// @brief the ego map matched information
// 定义一个 ::ad::map::match::Object 类型的变量，用于存储自主车辆（ego）在地图上匹配得到的相关信息，比如车辆在地图中的位置、与地图中地标等元素的关联情况等，
        // 这对于确定车辆的准确位置以及与周围环境的关系很重要
    ::ad::map::match::Object ego_match_object;

    /// @brief the ego route
    ::ad::map::route::FullRoute ego_route;

    /// @brief the ego dynamics on the route
// 定义一个 EgoDynamicsOnRoute 类型的变量，用于存储自主车辆在当前行驶路线上的各种动力学相关属性情况，
        // 像车辆的速度、加速度、位置、航向等方面的具体数值和状态，反映车辆在路线上的运动特性
    EgoDynamicsOnRoute ego_dynamics_on_route;

    /// @brief current used default vehicle dynamics for ego vehicle
// 定义一个 ::ad::rss::world::RssDynamics 类型的变量，用于存储当前应用于自主车辆（ego vehicle）的默认车辆动力学信息，
        // 也就是在没有特殊设置时自主车辆所遵循的动力学相关属性配置情况，比如速度、加速度等方面的默认参数设置情况
    ::ad::rss::world::RssDynamics default_ego_vehicle_dynamics;

    /// @brief check input: the RSS world model
    ::ad::rss::world::WorldModel world_model;

    /// @brief check result: the situation snapshot
    ::ad::rss::situation::SituationSnapshot situation_snapshot;
    /// @brief check result: the rss state snapshot
    ::ad::rss::state::RssStateSnapshot rss_state_snapshot;
    /// @brief check result: the proper response
// 定义一个 ::ad::rss::state::ProperResponse 类型的变量，用于存储 RSS 检查后得到的合适响应信息，
        // 例如在检测到潜在危险时应该采取的合理应对措施（如减速、变道等建议操作）等相关信息
    ::ad::rss::state::ProperResponse proper_response;
    /// @brief flag indicating if the current state is overall dangerous
// 定义一个布尔类型的变量，用于指示当前的整体状态是否处于危险状态，通过 RSS 相关检查和计算后根据一定的规则判断当前整个交通场景是否存在危险情况，
    bool dangerous_state;
    /// @brief flag indicating if the current state is dangerous because of a
    /// vehicle
// 定义一个布尔类型的变量，用于指示当前状态是否因为某辆车（可能是其他车辆与自主车辆之间的交互情况等）而处于危险状态，
    bool dangerous_vehicle;
    /// @brief flag indicating if the current state is dangerous because of an
    /// opposite vehicle
// 定义一个布尔类型的变量，用于指示当前状态是否因为对面来车（opposite vehicle，通常指行驶方向相反的车辆）而处于危险状态，
        // 对于分析相向行驶车辆之间的安全情况、危险原因等很有帮助
    bool dangerous_opposite_state;
  };

  class RssObjectChecker {
  public:
    RssObjectChecker(RssCheck const &rss_check, ::ad::rss::map::RssSceneCreation &scene_creation,
                     carla::client::Vehicle const &carla_ego_vehicle, CarlaRssState const &carla_rss_state,
                     ::ad::map::landmark::LandmarkIdSet const &green_traffic_lights);
// 定义 RssObjectChecker 类的构造函数，接受多个参数，包括一个 RssCheck 类型的常量引用（可能用于获取 RSS 相关的检查设置等信息）、
        // 一个 ::ad::rss::map::RssSceneCreation 类型的引用（用于 RSS 场景创建相关操作）、一个 carla::client::Vehicle 类型的常量引用（代表自主车辆对象，用于获取车辆相关信息）、
        // 一个 CarlaRssState 类型的常量引用（提供当前 RSS 相关的状态信息）以及一个 ::ad::map::landmark::LandmarkIdSet 类型的常量引用（存储绿灯交通信号灯的地标集合信息，
        // 可能用于判断交通信号灯状态对 RSS 计算的影响等），通过这些参数来初始化该类对象，以便后续进行针对交通参与者的 RSS 相关检查操作
    void operator()(const carla::SharedPtr<carla::client::Actor> other_traffic_participant) const;
// 定义函数调用运算符重载函数，接受一个指向 carla::client::Actor 类型的智能指针参数（代表其他交通参与者对象），
        // 用于对传入的其他交通参与者执行具体的 RSS 相关检查操作，并且该函数声明为 const 类型，表示不会修改类的成员变量状态，是一个只读操作的函数
  private:
    RssCheck const &_rss_check;
// 定义一个 RssCheck 类型的常量引用成员变量，用于在类内部获取外部传入的 RSS 检查相关的设置、功能等信息，
        // 方便在执行针对其他交通参与者的检查操作时使用统一的 RSS 检查配置情况
    ::ad::rss::map::RssSceneCreation &_scene_creation;
    carla::client::Vehicle const &_carla_ego_vehicle;
    CarlaRssState const &_carla_rss_state;
    ::ad::map::landmark::LandmarkIdSet const &_green_traffic_lights;
  };

  friend class RssObjectChecker;
// 声明 `RssObjectChecker` 类为友元类，意味着 `RssObjectChecker` 类的成员函数可以访问当前类（包含此声明的类，从上下文看应该是 `RssCheck` 类）的私有成员变量和私有函数，
// 方便 `RssObjectChecker` 类在执行相关操作时能获取到当前类内部的一些私有信息，从而实现更紧密的协作，例如使用当前类内部的状态来进行一些与 `RSS` 相关的检查操作等。
  /// @brief the current state of the ego vehicle
  CarlaRssState _carla_rss_state;
// 定义了一个 `CarlaRssState` 类型的变量 `_carla_rss_state`，用于存储自主车辆当前与 `RSS`（Responsibility-Sensitive Safety，责任敏感安全）相关的各种状态信息，
// 例如车辆的行驶路线、动力学信息、检查结果等内容，通过这个变量可以在类的不同成员函数中传递和操作自主车辆的相关状态数据。
  /// @brief calculate the map matched object from the actor
// 函数功能的简要注释，说明该函数用于根据传入的 `actor`（演员，在这里可能代表交通场景中的某个参与者，比如车辆、行人等）计算出与之对应的地图匹配对象，
// 也就是将参与者在实际场景中的位置等信息与地图上的元素进行匹配，进而得到在地图层面相关的对象表示，方便后续基于地图进行更多的操作和分析，比如判断位置关系、规划路径等。
  ::ad::map::match::Object GetMatchObject(carla::SharedPtr<carla::client::Actor> const &actor,
                                          ::ad::physics::Distance const &sampling_distance) const;
// 函数声明，它是一个 `const` 成员函数（意味着不会修改类的成员变量状态），接受两个参数：一个是指向 `carla::client::Actor` 类型的智能指针 `actor`，
// 代表要进行地图匹配操作的交通参与者对象；另一个是 `::ad::physics::Distance` 类型的常量参数 `sampling_distance`，可能用于在地图匹配过程中确定采样距离等相关操作，
// 函数的返回值类型为 `::ad::map::match::Object`，即返回匹配后的地图相关对象信息。
  /// @brief calculate the speed from the actor
// 函数功能注释，表明该函数的作用是根据传入的 `actor`（交通参与者对象）来计算其速度信息，方便获取不同参与者（如车辆、行人等）在交通场景中的速度情况，
// 进而基于速度进行各种分析，例如判断是否超速、进行碰撞风险评估等与 `RSS` 相关的计算操作。
  ::ad::physics::Speed GetSpeed(carla::client::Actor const &actor) const;
// 函数声明，同样是 `const` 成员函数，接受一个 `carla::client::Actor` 类型的常量参数 `actor`，代表要获取速度信息的交通参与者对象
  /// @brief calculate the heading change from the actor
// 函数功能注释，说明此函数用于根据传入的 `actor`（交通参与者对象）计算其航向变化信息，航向变化可以反映参与者在行驶过程中方向角度的改变情况，
// 对于分析车辆操控、判断是否偏离预期路线以及评估与其他参与者之间的相对位置关系等方面都很有帮助，尤其在 `RSS` 相关的安全分析场景中是重要的依据之一。
  ::ad::physics::AngularVelocity GetHeadingChange(carla::client::Actor const &actor) const;
// 函数声明，为 `const` 成员函数，参数 `actor` 是 `carla::client::Actor` 类型的常量，代表交通参与者对象，返回值类型为 `::ad::physics::AngularVelocity`，
// 用于返回计算得出的参与者的航向变化速率等相关信息。
  /// @brief calculate the steering angle from the actor
// 函数功能注释，指出该函数的功能是根据传入的 `actor`（这里明确要求是 `carla::client::Vehicle` 类型，即车辆对象）来计算其转向角度信息，
// 转向角度对于了解车辆的操控状态、判断车辆是否在合理的转向范围内以及在进行路径规划和 `RSS` 相关安全检查等操作时都具有重要意义，
// 比如可以通过转向角度判断车辆是否有异常转向行为，是否可能导致碰撞风险等。
  ::ad::physics::Angle GetSteeringAngle(carla::client::Vehicle const &actor) const;
// 函数声明，是 `const` 成员函数，参数 `actor` 限定为 `carla::client::Vehicle` 类型的常量，代表车辆对象，返回值类型为 `::ad::physics::Angle`，
// 即返回计算得到的车辆的转向角度信息。
  /// @brief update the desired ego vehicle route
// 函数功能注释，说明此函数用于更新期望的自主车辆（ego vehicle）行驶路线，可能会根据交通场景的变化、车辆当前状态以及其他相关因素，
// 对自主车辆原本规划的路线进行调整，确保车辆沿着更合适、更安全的路线行驶，例如避开拥堵、避开危险区域等情况时会用到该函数来更新路线信息。
  void UpdateRoute(CarlaRssState &carla_rss_state);
// 函数声明，接受一个 `CarlaRssState` 类型的引用参数 `carla_rss_state`，这个参数可能包含了自主车辆当前的各种状态信息（如位置、速度、已有路线等），
// 通过传入这个参数，函数内部可以基于当前状态来更新车辆的行驶路线，由于是引用传递，函数内部对该参数所代表的状态信息的修改会反映到外部传入的实际对象上。
  /// @brief calculate ego vehicle dynamics on the route
// 函数功能注释，表明该函数的作用是计算自主车辆（ego vehicle）在行驶路线上的动力学相关信息，动力学信息包括车辆的速度、加速度、方向变化等多个方面，
// 这些信息对于全面了解车辆在路线上的运动状态、进行 `RSS` 相关的安全分析以及后续的路径规划和决策等操作都非常关键。
  EgoDynamicsOnRoute CalculateEgoDynamicsOnRoute(carla::client::Timestamp const &current_timestamp,
                                                 double const &time_since_epoch_check_start_ms,
                                                 carla::client::Vehicle const &carla_vehicle,
                                                 ::ad::map::match::Object match_object,
                                                 ::ad::map::route::FullRoute const &route,
                                                 ::ad::rss::world::RssDynamics const &default_ego_vehicle_dynamics,
                                                 EgoDynamicsOnRoute const &last_dynamics) const;
// 函数声明，是一个 `const` 成员函数，接受多个参数：
// - `carla::client::Timestamp` 类型的常量参数 `current_timestamp`，代表当前的时间戳，用于确定计算的时间点以及基于时间顺序进行相关的分析；
// - `double` 类型的常量参数 `time_since_epoch_check_start_ms`，表示从某个起始时间点（epoch，通常可以理解为程序启动时刻等）开始到检查开始时所经过的时间，单位是毫秒，
//  有助于分析时间相关的信息，比如不同阶段的时间消耗、操作的时效性等；
// - `carla::client::Vehicle` 类型的常量参数 `carla_vehicle`，代表自主车辆对象，用于获取车辆自身的相关属性信息，如位置、速度等基础数据；
// - `::ad::map::match::Object` 类型的常量参数 `match_object`，是前面提到的地图匹配对象，包含车辆在地图上的匹配信息，可辅助分析车辆与地图元素的关系以及位置情况；
// - `::ad::map::route::FullRoute` 类型的常量参数 `route`，表示完整的行驶路线信息，是计算车辆在路线上动力学情况的基础，比如沿着路线的不同位置速度如何变化等；
// - `::ad::rss::world::RssDynamics` 类型的常量参数 `default_ego_vehicle_dynamics`，提供了默认的自主车辆动力学相关设置信息，例如默认的速度、加速度等参数，
//  用于在计算中参考或使用这些默认值；
// - `EgoDynamicsOnRoute` 类型的常量参数 `last_dynamics`，可能代表上一次计算得到的车辆动力学信息，便于对比分析车辆动力学的变化情况等。
// 函数返回值类型为 `EgoDynamicsOnRoute`，即返回计算得出的自主车辆在当前路线上的最新动力学相关信息。
  void UpdateDefaultRssDynamics(CarlaRssState &carla_rss_state);
// 函数声明，接受一个 `CarlaRssState` 类型的引用参数 `carla_rss_state`，该函数的作用可能是更新 `CarlaRssState` 中与默认 `RSS` 动力学相关的信息，
// 例如根据交通场景的变化、新的配置要求或者其他相关因素，对默认情况下的 `RSS` 动力学参数（如速度、加速度等方面的默认设置）进行调整，
// 并且通过引用传递参数，函数内部的更新操作会直接影响外部传入的实际 `CarlaRssState` 对象中的相关数据。
  /// @brief collect the green traffic lights on the current route
// 函数功能注释，说明此函数用于收集当前行驶路线上的绿灯交通信号灯信息，在交通场景中，交通信号灯的状态对于车辆的行驶决策、路径规划以及 `RSS` 相关的安全分析等都有重要影响，
// 通过收集绿灯信息，可以帮助车辆判断是否能够顺利通行、是否需要停车等待等情况，从而做出更合理的行驶操作。
  ::ad::map::landmark::LandmarkIdSet GetGreenTrafficLightsOnRoute(
      std::vector<SharedPtr<carla::client::TrafficLight>> const &traffic_lights,
// 函数声明，是一个 `const` 成员函数，接受两个参数：
// - `std::vector<SharedPtr<carla::client::TrafficLight>>` 类型的常量参数 `traffic_lights`，它是一个智能指针向量，每个元素指向 `carla::client::TrafficLight` 类型的交通信号灯对象，
//  代表交通场景中所有的交通信号灯集合，函数会从中筛选出与当前路线相关的信号灯；
// - `::ad::map::route::FullRoute` 类型的常量参数 `route`，表示完整的行驶路线信息，用于确定哪些交通信号灯处于当前路线上，以便准确收集对应的绿灯信息。
// 函数返回值类型为 `::ad::map::landmark::LandmarkIdSet`，返回的是一个包含绿灯交通信号灯地标标识的集合，方便后续基于这些标识进行进一步的操作和判断。      ::ad::map::route::FullRoute const &route) const;

  /// @brief Create the RSS world model
  void CreateWorldModel(carla::client::Timestamp const &timestamp, carla::client::ActorList const &actors,
                        carla::client::Vehicle const &carla_ego_vehicle, CarlaRssState &carla_rss_state) const;
// 函数声明，是一个 `const` 成员函数，接受四个参数：
// - `carla::client::Timestamp` 类型的常量参数 `timestamp`，代表当前的时间戳，用于标记世界模型创建的时间点，同时也方便基于时间顺序对模型中的数据进行更新和分析；
// - `carla::client::ActorList` 类型的常量参数 `actors`，它是一个演员列表，包含了交通场景中所有的参与者对象（如各种车辆、行人等），是构建世界模型中参与者相关信息的基础数据来源；
// - `carla::client::Vehicle` 类型的常量参数 `carla_ego_vehicle`，特指自主车辆对象，用于在世界模型中明确表示自主车辆的相关信息，例如位置、速度、行驶方向等，
//  它在整个 `RSS` 世界模型中是核心关注对象之一；
// - `CarlaRssState` 类型的引用参数 `carla_rss_state`，这个参数可能用于存储创建好的世界模型以及相关的中间状态信息等，通过引用传递方便函数内部对其进行修改和赋值操作，
//  使得外部传入的实际对象能获取到最新的世界模型数据。
  /// @brief Perform the actual RSS check
  bool PerformCheck(CarlaRssState &carla_rss_state) const;
// 函数声明，是一个 `const` 成员函数，接受一个 `CarlaRssState` 类型的引用参数 `carla_rss_state`，这个参数包含了进行 `RSS` 检查所需要的各种数据，
// 如世界模型、车辆状态等信息，函数会基于这些数据进行检查操作，并返回一个布尔值，用于表示检查结果，通常 `true` 表示通过检查（即当前状态符合 `RSS` 安全要求），
// `false` 表示未通过检查（即存在潜在安全风险）。
  /// @brief Analyse the RSS check results
// 函数功能注释，表明该函数用于分析 `RSS` 检查的结果，在执行完 `RSS` 检查后，得到的结果可能只是一个简单的通过或未通过的标识，
// 而此函数则会进一步深入分析结果中的具体情况，例如判断风险的具体来源（是因为车辆超速、距离过近还是其他原因等）、危险程度的量化评估等，
// 以便后续基于分析结果采取更合适的应对措施，如发出警告、调整车辆行驶策略等操作。
  void AnalyseCheckResults(CarlaRssState &carla_rss_state) const;
// 函数声明，是一个 `const` 成员函数，同样接受 `CarlaRssState` 类型的引用参数 `carla_rss_state`，这个参数包含了 `RSS` 检查后的结果信息，
// 函数会对这些结果数据进行详细分析，虽然函数没有返回值，但通过对传入参数所代表的结果数据的内部分析和修改（例如设置一些表示分析结果的标志位等），
// 可以使外部在获取这个参数对象时能获取到分析后的详细情况，从而做出相应的决策和操作。
};

}  // namespace rss
}  // namespace carla

namespace std {
// 进入 `std` 标准命名空间，这里是在标准命名空间内对一些操作符进行重载定义，方便对自定义类型进行像内置类型一样的输出等操作，
// 使得代码在使用这些自定义类型时更加方便和直观，符合 `C++` 语言的常规使用习惯。
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
