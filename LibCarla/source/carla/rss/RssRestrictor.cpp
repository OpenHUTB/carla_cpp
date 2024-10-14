// Copyright (c) 2019-2020 Intel Corporation
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/rss/RssRestrictor.h"
#include "carla/rpc/VehicleControl.h"
#include "carla/rpc/VehiclePhysicsControl.h"
#include "carla/rss/RssCheck.h"

#include <spdlog/sinks/stdout_color_sinks.h>
#include <ad/rss/state/ProperResponse.hpp>
#include <ad/rss/unstructured/Geometry.hpp>
#include <ad/rss/world/Velocity.hpp>

namespace carla {
namespace rss {
// RSS限制器类的定义
RssRestrictor::RssRestrictor() {
  std::string logger_name = "RssRestrictor";
  // 获取名为"RssRestrictor"的日志记录器，如果不存在则创建一个
  _logger = spdlog::get(logger_name);
  if (!_logger) {
    _logger = spdlog::create<spdlog::sinks::stdout_color_sink_st>(logger_name);
  }
 // 设置日志级别为警告级别
  SetLogLevel(spdlog::level::warn);
}
// 析构函数，默认实现
RssRestrictor::~RssRestrictor() = default;
// 设置日志级别
void RssRestrictor::SetLogLevel(const uint8_t log_level) {
  if (log_level < spdlog::level::n_levels) {
  	// 将传入的日志级别转换为spdlog的日志级别枚举类型
    const auto log_level_value = static_cast<spdlog::level::level_enum>(log_level);
    // 设置日志记录器的日志级别
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
  float radius = 1.f;
  for (auto const &wheel : vehicle_physics.wheels) {
    sum_max_brake_torque += wheel.max_brake_torque;
    radius = wheel.radius;
    max_steer_angle_deg = std::max(max_steer_angle_deg, wheel.max_steer_angle);
  }

  // do not apply any restrictions when in reverse gear
  if (!vehicle_control.reverse) {
  	// 输出调试信息，包括RSS响应中的加速度限制、车辆在路线上的横向速度、加速度等信息
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

    // decelerate
    if (accel_lon < ::ad::physics::Acceleration(0.0)) {
    	// 将油门设置为0，计算制动加速度并设置制动值
      restricted_vehicle_control.throttle = 0.0f;

      double brake_acceleration =
          std::fabs(static_cast<double>(proper_response.accelerationRestrictions.longitudinalRange.minimum));
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
