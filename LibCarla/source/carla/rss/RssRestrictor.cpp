// Copyright (c) 2019-2020 Intel Corporation
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.
// 包含Carla项目中RSS（Responsive Safety Shell，一种安全机制相关概念）限制器（RssRestrictor）的头文件，
// 推测其中定义了RssRestrictor类的声明等内容，用于实现对车辆控制的限制功能，基于RSS相关规则
#include "carla/rss/RssRestrictor.h" // 包含Carla项目中远程过程调用（RPC）相关的车辆控制（VehicleControl）头文件，用于定义车辆控制相关的数据结构和操作，
// 比如油门、刹车、转向等控制信息
#include "carla/rpc/VehicleControl.h"// 包含Carla项目中远程过程调用（RPC）相关的车辆物理控制（VehiclePhysicsControl）头文件，
// 用于定义车辆物理属性相关的控制信息，如车辆质量、车轮参数等
#include "carla/rpc/VehiclePhysicsControl.h" // 包含Carla项目中RSS检查（RssCheck）相关的头文件，可能用于进行RSS相关的检查、验证等操作，与整体的安全机制相关
#include "carla/rss/RssCheck.h"// 包含spdlog库中用于将日志输出到标准输出（控制台）且带有颜色显示的日志接收器（sink）相关头文件，
// spdlog用于在程序中进行日志记录，方便调试和查看运行状态信息

#include <spdlog/sinks/stdout_color_sinks.h>// 包含ad/rss库中与状态相关的合适响应（ProperResponse）头文件，可能定义了在RSS机制下车辆应做出的合适响应的数据结构等内容
#include <ad/rss/state/ProperResponse.hpp>// 包含ad/rss库中与非结构化场景相关的几何（Geometry）头文件，可能涉及到场景中的几何形状、位置等信息，用于RSS相关的计算和判断
#include <ad/rss/unstructured/Geometry.hpp>// 包含ad/rss库中与世界相关的速度（Velocity）头文件，用于定义和处理车辆在虚拟世界中的速度相关信息
#include <ad/rss/world/Velocity.hpp>
// 定义在Carla项目的rss命名空间下，表明这些类和函数是与RSS相关功能实现的一部分
namespace carla {
namespace rss {

RssRestrictor::RssRestrictor() {
  std::string logger_name = "RssRestrictor";
  // 获取名为"RssRestrictor"的日志记录器，如果不存在则创建一个
  _logger = spdlog::get(logger_name);
  if (!_logger) {
    _logger = spdlog::create<spdlog::sinks::stdout_color_sink_st>(logger_name);
  }
// 设置日志记录器的日志级别为警告级别（warn），意味着只有警告及更严重级别的日志信息才会被记录和输出
  SetLogLevel(spdlog::level::warn);
}
// 析构函数，默认的析构函数实现，这里没有额外的资源释放等特殊操作，所以使用默认的编译器生成的析构函数即可
RssRestrictor::~RssRestrictor() = default;
// 设置日志级别
void RssRestrictor::SetLogLevel(const uint8_t log_level) {
  if (log_level < spdlog::level::n_levels) {
  	// 将传入的无符号8位整数类型的日志级别参数转换为spdlog库中定义的日志级别枚举类型（level_enum），
            // 以便能够正确设置日志记录器的日志级别
    const auto log_level_value = static_cast<spdlog::level::level_enum>(log_level);
   // 通过调用日志记录器的set_level函数，将其日志级别设置为转换后的日志级别枚举值对应的级别，
            // 从而控制日志记录的详细程度
    _logger->set_level(log_level_value);
  }
}
// 根据RSS响应限制车辆控制
carla::rpc::VehicleControl RssRestrictor::RestrictVehicleControl(
    const carla::rpc::VehicleControl &vehicle_control, const ::ad::rss::state::ProperResponse &proper_response,
    const carla::rss::EgoDynamicsOnRoute &ego_dynamics_on_route,
    const carla::rpc::VehiclePhysicsControl &vehicle_physics) {
    	// 创建一个受限的车辆控制对象，初始化为传入的车辆控制对象
  carla::rpc::VehicleControl restricted_vehicle_control(vehicle_control);

  // Pretty basic implementation of a RSS restrictor modifying the
  // VehicleControl according to the given
  // restrictions. Basic braking and countersteering actions are applied.
  // In case countersteering is not possible anymore (i.e. the lateral velocity
  // reached zero),
  // as a fallback longitudinal braking is applied instead (escalation
  // strategy).
  float mass = vehicle_physics.mass;
  float max_steer_angle_deg = 0.f;
  float sum_max_brake_torque = 0.f;
  float radius = 1.f;// 遍历车辆物理控制信息中的每个车轮参数信息，计算最大制动扭矩总和、获取车轮半径以及找到最大的转向角度
  for (auto const &wheel : vehicle_physics.wheels) {
    sum_max_brake_torque += wheel.max_brake_torque;
    radius = wheel.radius;
    max_steer_angle_deg = std::max(max_steer_angle_deg, wheel.max_steer_angle);
  }
// 如果车辆不在倒车档（reverse为false），则进行以下的控制限制操作，
        // 因为倒车情况下可能有不同的处理逻辑或者不需要进行这些基于RSS的常规限制操作
  if (!vehicle_control.reverse) {
  	// 输出调试信息，记录RSS响应中的纵向加速度限制范围、横向左右方向的加速度限制范围，
            // 以及车辆自身在路线上的横向速度、横向加速度、平均横向加速度、车辆航向和允许的航向范围等信息，
            // 方便调试时查看相关参数情况，判断限制操作是否合理
    _logger->debug("Lon {}, L {}, R {}; LatSpeed {}, Accel {}, Avg {}, Hdg {}, AllowedHeadingRanges {}",
                   proper_response.accelerationRestrictions.longitudinalRange,
                   proper_response.accelerationRestrictions.lateralLeftRange,
                   proper_response.accelerationRestrictions.lateralRightRange, ego_dynamics_on_route.route_speed_lat,
                   ego_dynamics_on_route.route_accel_lat, ego_dynamics_on_route.avg_route_accel_lat,
                   ego_dynamics_on_route.ego_heading, proper_response.headingRanges);
                   // 如果向左的横向加速度限制最大值小于等于0
    if (proper_response.accelerationRestrictions.lateralLeftRange.maximum <= ::ad::physics::Acceleration(0.0)) {
    	// 如果车辆横向速度小于0（向左行驶）且纵向速度不为0
      if (ego_dynamics_on_route.route_speed_lat < ::ad::physics::Speed(0.0)) {
        // driving to the left
        if (ego_dynamics_on_route.route_speed_lon != ::ad::physics::Speed(0.0)) {
          double angle_rad = std::atan(ego_dynamics_on_route.route_speed_lat / ego_dynamics_on_route.route_speed_lon);
          float desired_steer_ratio = -180.f * static_cast<float>(angle_rad / M_PI) / max_steer_angle_deg;
         // 如果车辆正在跨越边界，则增加一点转向比例
          if (ego_dynamics_on_route.crossing_border) {
            desired_steer_ratio += 0.1f;
          }
          // 记录原始转向值，更新受限的车辆控制的转向值，并确保在合理范围内
          float orig_steer = restricted_vehicle_control.steer;
          restricted_vehicle_control.steer = std::max(restricted_vehicle_control.steer, desired_steer_ratio);
          restricted_vehicle_control.steer = std::min(restricted_vehicle_control.steer, 1.0f);
          // 输出调试信息
          _logger->debug("EgoVelocity: {}", ego_dynamics_on_route);
          _logger->debug("Countersteer left to right: {} -> {}", orig_steer, restricted_vehicle_control.steer);
        }
      }
    }
// 如果向右的横向加速度限制最大值小于等于0
    if (proper_response.accelerationRestrictions.lateralRightRange.maximum <= ::ad::physics::Acceleration(0.0)) {
    	// 如果车辆横向速度大于0（向右行驶）且纵向速度不为0
      if (ego_dynamics_on_route.route_speed_lat > ::ad::physics::Speed(0.0)) {
        // driving to the right
        if (ego_dynamics_on_route.route_speed_lon != ::ad::physics::Speed(0.0)) {
          double angle_rad = std::atan(ego_dynamics_on_route.route_speed_lat / ego_dynamics_on_route.route_speed_lon);
          float desired_steer_ratio = -180.f * static_cast<float>(angle_rad / M_PI) / max_steer_angle_deg;
          // 如果车辆正在跨越边界，则减少一点转向比例
          if (ego_dynamics_on_route.crossing_border) {
            desired_steer_ratio -= 0.1f;
          }
          // 记录原始转向值，更新受限的车辆控制的转向值，并确保在合理范围内
          float orig_steer = restricted_vehicle_control.steer;
          restricted_vehicle_control.steer = std::min(restricted_vehicle_control.steer, desired_steer_ratio);
          restricted_vehicle_control.steer = std::max(restricted_vehicle_control.steer, -1.0f);
          _logger->debug("EgoVelocity: {}", ego_dynamics_on_route);
          _logger->debug("Countersteer right to left: {} -> {}", orig_steer, restricted_vehicle_control.steer);
        }
      }
    }

    // restrict longitudinal acceleration
    auto accel_lon = proper_response.accelerationRestrictions.longitudinalRange.maximum;
    if (proper_response.unstructuredSceneResponse == ad::rss::state::UnstructuredSceneResponse::DriveAway) {
      // drive away is only allowed in certain direction
      auto heading_allowed = false;
      if (!proper_response.headingRanges.empty()) {
      	// 计算最大转向角度（弧度）
        auto max_steer_angle = max_steer_angle_deg * (ad::physics::cPI / ad::physics::Angle(180.0));
        // 计算当前转向角度
        auto current_steering_angle = static_cast<double>(ego_dynamics_on_route.ego_heading) - vehicle_control.steer * max_steer_angle;
        // 检查当前转向角度是否在允许的范围内
        for (auto it = proper_response.headingRanges.cbegin(); (it != proper_response.headingRanges.cend() && !heading_allowed); ++it) {
          heading_allowed = ad::rss::unstructured::isInsideHeadingRange(current_steering_angle, *it);
        }
      }
// 如果不允许在当前方向上行驶，则将纵向加速度设置为最小值
      if (!heading_allowed) {
        accel_lon = proper_response.accelerationRestrictions.longitudinalRange.minimum;
      }
    }
// 如果纵向加速度大于0（加速），TODO：确定加速度并限制油门
    if (accel_lon > ::ad::physics::Acceleration(0.0)) {
      // TODO: determine acceleration and limit throttle
    }
// 如果纵向加速度小于0（表示车辆需要减速），进行以下的减速操作
   
    if (accel_lon < ::ad::physics::Acceleration(0.0)) {
    	// 将油门设置为0，计算制动加速度并设置制动值
      restricted_vehicle_control.throttle = 0.0f;
// 计算制动加速度，取纵向加速度限制范围最小值的绝对值作为制动加速度数值（转换为双精度浮点数类型）
      double brake_acceleration =
          std::fabs(static_cast<double>(proper_response.accelerationRestrictions.longitudinalRange.minimum));// 根据车辆质量、制动加速度以及车轮半径等信息，计算总的制动扭矩数值，
                // 这里涉及一些物理公式的应用，用于确定需要施加多大的制动力来实现减速
      double sum_brake_torque = mass * brake_acceleration * radius / 100.0;
      restricted_vehicle_control.brake = std::min(static_cast<float>(sum_brake_torque / sum_max_brake_torque), 1.0f);
    }
  }
  // 如果受限的车辆控制与原始车辆控制不同，则输出调试信息
  if (restricted_vehicle_control != vehicle_control) {
    _logger->debug(
        "Restrictor active: throttle({} -> {}), brake ({} -> {}). steer ({} -> "
        "{})",
        vehicle_control.throttle, restricted_vehicle_control.throttle, vehicle_control.brake,
        restricted_vehicle_control.brake, vehicle_control.steer, restricted_vehicle_control.steer);
  }
   // 返回受限的车辆控制对象
  return restricted_vehicle_control;
}

}  // namespace rss
}  // namespace carla
