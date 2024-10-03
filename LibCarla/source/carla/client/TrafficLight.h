// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/client/TrafficSign.h"
#include "carla/rpc/TrafficLightState.h"
#include "carla/client/Waypoint.h"
#include "carla/client/Map.h"
#include "carla/geom/BoundingBox.h"

namespace carla {
namespace client {

  class TrafficLight : public TrafficSign {

  public:

    explicit TrafficLight(ActorInitializer init) : TrafficSign(std::move(init)) {}

    void SetState(rpc::TrafficLightState state);

    /// 返回交通灯的当前状态。
    ///
    /// @note 此函数不调用模拟器，它返回最后一个节拍中接收到的数据。
    rpc::TrafficLightState GetState() const;

    void SetGreenTime(float green_time);

    /// @note 此函数不调用模拟器，它返回最后一个节拍中接收到的数据。
    float GetGreenTime() const;

    void SetYellowTime(float yellow_time);

    /// @note 此函数不调用模拟器，它返回最后一个节拍中接收到的数据。
    float GetYellowTime() const;

    void SetRedTime(float red_time);

    /// @note 此函数不调用模拟器，它返回最后一个节拍中接收到的数据。
    float GetRedTime() const;

    /// @note 此函数不调用模拟器，它返回最后一个节拍中接收到的数据。
    float GetElapsedTime() const;

    void Freeze(bool freeze);

    /// @note 此函数不调用模拟器，它返回最后一个节拍中接收到的数据。
    bool IsFrozen() const;

    /// 返回交通信号灯组中灯杆的索引
    uint32_t GetPoleIndex();

    /// 返回该交通灯所属组中的所有交通灯。
    ///
    /// @note 该函数调用模拟器
    std::vector<SharedPtr<TrafficLight>> GetGroupTrafficLights();

    // 重置所有组的计时器和状态
    void ResetGroup();

    std::vector<SharedPtr<Waypoint>> GetAffectedLaneWaypoints() const;

    std::vector<geom::BoundingBox> GetLightBoxes() const;

    road::SignId GetOpenDRIVEID() const;

    std::vector<SharedPtr<Waypoint>> GetStopWaypoints() const;

  };

} // namespace client
} // namespace carla
