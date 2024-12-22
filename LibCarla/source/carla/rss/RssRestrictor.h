// Copyright (c) 2019-2020 Intel Corporation
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once  // 防止头文件被重复包含

#include <spdlog/spdlog.h>  // 引入 spdlog 日志库
#include <memory>  // 引入智能指针库

namespace ad {
namespace rss {
namespace state {

// 前向声明 RSS 正确响应
struct ProperResponse;

}  // namespace world
}  // namespace rss
}  // namespace ad

namespace carla {
namespace rpc {
class VehicleControl;  // 前向声明车辆控制类
class VehiclePhysicsControl;  // 前向声明车辆物理控制类
}  // namespace rpc

namespace rss {

// 前向声明自我车辆在当前路线上的动态
struct EgoDynamicsOnRoute;

// 实现 CARLA 中 RSS 限制的类
class RssRestrictor {
public:
  // 构造函数
  RssRestrictor();

  // 析构函数
  ~RssRestrictor();

  // 实际上限制给定车辆控制输入以模拟
  // 通过制动来符合 RSS 的行为
  // 横向制动通过反向转向实现，因此只是一个非常粗略的解决方案
  carla::rpc::VehicleControl RestrictVehicleControl(const carla::rpc::VehicleControl &vehicle_control,  // 限制车辆控制
                                                    const ::ad::rss::state::ProperResponse &proper_response,  // 正确响应
                                                    const carla::rss::EgoDynamicsOnRoute &ego_dynamics_on_route,  // 自我车辆动态
                                                    const carla::rpc::VehiclePhysicsControl &vehicle_physics);  // 车辆物理控制

  void SetLogLevel(const uint8_t log_level);  // 设置日志级别

private:
  // 日志记录器实例
  std::shared_ptr<spdlog::logger> _logger;  // 智能指针管理的日志记录器
};

}  // namespace rss
}  // namespace carla
