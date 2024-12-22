// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/client/Actor.h"
#include "carla/road/RoadTypes.h"

namespace carla {
namespace client {

  class TrafficSign : public Actor { // 定义一个名为TrafficSign的类，它继承自Actor类。
  public: // 公共成员区域。

    explicit TrafficSign(ActorInitializer init) : Actor(std::move(init)) {} // TrafficSign类的构造函数，使用ActorInitializer初始化基类Actor。

    const geom::BoundingBox &GetTriggerVolume() const {
      return ActorState::GetBoundingBox(); 
    }// 获取触发体积的成员函数。

    carla::road::SignId GetSignId() const; // 获取交通标志ID的成员函数。


  };

} // namespace client
} // namespace carla
