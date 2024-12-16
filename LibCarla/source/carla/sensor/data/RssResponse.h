// Copyright (c) 2019-2020 Intel Corporation
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

// 引入 RSS 检查相关的头文件，可能包含了进行 RSS 相关计算、判断等功能的定义
#include "carla/rss/RssCheck.h"
// 引入传感器数据相关的头文件，用于处理传感器采集到的数据结构等内容
#include "carla/sensor/SensorData.h"

namespace carla {
namespace sensor {
namespace data {

// RssResponse 类继承自 SensorData，代表了一个 RSS（Responsive Safety System，可能是一种安全响应相关系统）的响应信息类
// 它用于封装与 RSS 响应相关的各种数据，例如响应是否有效、具体的响应内容、相关的状态快照以及世界模型等信息
class RssResponse : public SensorData {
public:
    // 构造函数，用于初始化 RssResponse 对象
    // 参数解释：
    // - frame_number：表示数据对应的帧编号，用于标识不同时刻采集的数据帧
    // - timestamp：时间戳，记录了该响应对应的时间点，方便确定数据产生的先后顺序等
    // - sensor_transform：传感器的变换信息，可能涉及传感器在世界坐标系中的位置、朝向等变换情况
    // - response_valid：表示 RSS 计算结果是否有效的布尔值，用于判断此次 RSS 响应是否可靠
    // - response：具体的合适的（Proper）RSS 响应内容，包含了应对某种情况所采取的具体响应动作等详细信息
    // - rss_state_snapshot：RSS 状态的快照，记录了 RSS 系统在某个特定时刻的状态情况，例如各个相关实体的状态等
    // - situation_snapshot：情景快照，描述了产生该 RSS 响应时所处的具体情景信息，比如周边环境、相关对象关系等
    // - world_model：世界模型，可能包含了整个模拟世界的相关信息，如各个物体的位置、属性等，为 RSS 响应提供全局的环境参考
    // - ego_dynamics_on_route：表示自身（Ego，通常指车辆自身等主体）在行驶路线上的动力学相关信息，比如速度、加速度等动态情况
    explicit RssResponse(size_t frame_number, double timestamp, const rpc::Transform &sensor_transform,
                          const bool &response_valid, const ::ad::rss::state::ProperResponse &response,
                          const ::ad::rss::state::RssStateSnapshot &rss_state_snapshot,
                          const ::ad::rss::situation::SituationSnapshot &situation_snapshot,
                          const ::ad::rss::world::WorldModel &world_model,
                          const carla::rss::EgoDynamicsOnRoute &ego_dynamics_on_route)
        : SensorData(frame_number, timestamp, sensor_transform),
          _response_valid(response_valid),
          _response(response),
          _rss_state_snapshot(rss_state_snapshot),
          _situation_snapshot(situation_snapshot),
          _world_model(world_model),
          _ego_dynamics_on_route(ego_dynamics_on_route) {}

    // 获取 RSS 计算结果是否有效的函数，返回值为布尔类型
    // 外部代码可以通过调用此函数来判断此次 RSS 响应是否是有效的、可信赖的
    bool GetResponseValid() const {
        return _response_valid;
    }

    // 获取具体的合适的（Proper）RSS 响应内容的函数
    // 返回的是 ::ad::rss::state::ProperResponse 类型的常量引用，外部代码可以通过此函数获取 RSS 系统针对当前情况制定的具体响应措施等详细信息
    const ::ad::rss::state::ProperResponse &GetProperResponse() const {
        return _response;
    }

    // 获取 RSS 状态快照的函数
    // 返回的是 ::ad::rss::state::RssStateSnapshot 类型的常量引用，调用者可以借此获取 RSS 系统在特定时刻的状态详情，例如各个相关实体的状态信息等
    const ::ad::rss::state::RssStateSnapshot &GetRssStateSnapshot() const {
        return _rss_state_snapshot;
    }

    // 获取情景快照的函数
    // 返回的是 ::ad::rss::situation::SituationSnapshot 类型的常量引用，外部可通过此函数获取产生该 RSS 响应时所处的具体情景相关信息，比如周边环境、相关对象关系等
    const ::ad::rss::situation::SituationSnapshot &GetSituationSnapshot() const {
        return _situation_snapshot;
    }

    // 获取世界模型的函数
    // 返回的是 ::ad::rss::world::WorldModel 类型的常量引用，外界可以通过此函数拿到整个模拟世界的相关信息，例如各个物体的位置、属性等，以便进行更多基于全局环境的分析或操作
    const ::ad::rss::world::WorldModel &GetWorldModel() const {
        return _world_model;
    }

    // 获取自身（Ego）在行驶路线上的动力学相关信息的函数
    // 返回的是 carla::rss::EgoDynamicsOnRoute 类型的常量引用，外部代码可通过此函数获取自身主体（如车辆）在行驶过程中的速度、加速度等动态情况信息
    const carla::rss::EgoDynamicsOnRoute &GetEgoDynamicsOnRoute() const {
        return _ego_dynamics_on_route;
    }

private:
    // 成员变量，表示 RSS 计算的有效性，用于记录此次 RSS 响应是否可靠，true 表示有效，false 表示无效
    bool _response_valid;

    // 成员变量，具体的合适的（Proper）RSS 响应内容，包含了应对某种情况所采取的具体响应动作等详细信息
    ::ad::rss::state::ProperResponse _response;

    // 成员变量，RSS 状态的快照，记录了 RSS 系统在某个特定时刻的状态情况，例如各个相关实体的状态等
    ::ad::rss::state::RssStateSnapshot _rss_state_snapshot;

    // 成员变量，情景快照，描述了产生该 RSS 响应时所处的具体情景信息，比如周边环境、相关对象关系等
    ::ad::rss::situation::SituationSnapshot _situation_snapshot;

    // 成员变量，世界模型，可能包含了整个模拟世界的相关信息，如各个物体的位置、属性等，为 RSS 响应提供全局的环境参考
    ::ad::rss::world::WorldModel _world_model;

    // 成员变量，表示自身（Ego，通常指车辆自身等主体）在行驶路线上的动力学相关信息，比如速度、加速度等动态情况
    carla::rss::EgoDynamicsOnRoute _ego_dynamics_on_route;
};

}  // namespace data
}  // namespace sensor
}  // namespace carla
