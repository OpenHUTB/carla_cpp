// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/sensor/SensorData.h" // 包含传感器数据类的定义

#include "carla/client/Actor.h"
#include "carla/road/element/LaneMarking.h" // 包含车道标记类的定义
#include "carla/rpc/ActorId.h"

#include <vector> // 包含标准模板库中的向量容器

namespace carla {
namespace sensor {
namespace data {

  // 定义一个车道变更事件类
  class LaneInvasionEvent : public SensorData {
  public:

    using LaneMarking = road::element::LaneMarking; // 定义一个类型别名，方便使用

    explicit LaneInvasionEvent( // 构造函数，初始化车道入侵事件
        size_t frame, // 当前帧数
        double timestamp, // 时间戳
        const rpc::Transform &sensor_transform, // 传感器的变换
        ActorId parent,
        std::vector<LaneMarking> crossed_lane_markings) // 被穿越的车道标记列表
      : SensorData(frame, timestamp, sensor_transform), // 调用基类构造函数
        _parent(parent),
        _crossed_lane_markings(std::move(crossed_lane_markings)) {} // 初始化成员变量

    /// 获取入侵车道的Actor
    SharedPtr<client::Actor> GetActor() const;

    /// 获取被穿越的车道标记列表
    const std::vector<LaneMarking> &GetCrossedLaneMarkings() const {
      return _crossed_lane_markings;
    }

  private:

    ActorId _parent; // 父Actor的ID

    std::vector<LaneMarking> _crossed_lane_markings; // 被穿越的车道标记列表
  };

} // namespace data
} // namespace sensor
} // namespace carla
