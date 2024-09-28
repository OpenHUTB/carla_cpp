// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/nav/Navigation.h"
#include "carla/nav/WalkerManager.h"
#include "carla/nav/WalkerEvent.h"
#include "carla/rpc/TrafficLightState.h"

namespace carla {
namespace nav {

    EventResult WalkerEventVisitor::operator()(WalkerEventIgnore &) {
        return EventResult::End;
    }

    EventResult WalkerEventVisitor::operator()(WalkerEventWait &event) {
        // 刷新时间并检查
        event.time -= _delta;
        if (event.time <= 0.0)
            return EventResult::End;
        else
            return EventResult::Continue;
    }

    EventResult WalkerEventVisitor::operator()(WalkerEventStopAndCheck &event) {
        event.time -= _delta;
        if (event.time <= 0.0) {
            return EventResult::TimeOut;
        } else {
            carla::geom::Location currentUnrealPos;
            _manager->GetNavigation()->PauseAgent(_id, true);
            _manager->GetNavigation()->GetWalkerPosition(_id, currentUnrealPos);
            // 检查是否需要检查那里的交通信号灯（仅限第一次）
            if (event.check_for_trafficlight) {
                event.actor = _manager->GetTrafficLightAffecting(currentUnrealPos);
                event.check_for_trafficlight = false;
            }
            // 检查是否需要等待红绿灯
            if (event.actor) {
                auto state = event.actor->GetState();
                if (state == carla::rpc::TrafficLightState::Green || 
                    state == carla::rpc::TrafficLightState::Yellow) {
                    return EventResult::Continue;
                }
            }
            _manager->GetNavigation()->PauseAgent(_id, false);
            // 计算寻找车辆的方向
            carla::geom::Location crosswalkEnd;
            _manager->GetWalkerCrosswalkEnd(_id, crosswalkEnd);
            carla::geom::Location direction;
            direction.x = crosswalkEnd.x - currentUnrealPos.x;
            direction.y = crosswalkEnd.y - currentUnrealPos.y;
            direction.z = crosswalkEnd.z - currentUnrealPos.z;
            // 检查代理附近是否有车辆
            if (_manager && !(_manager->GetNavigation()->HasVehicleNear(_id, 6.0f, direction))) {
                return EventResult::End;
            } else {
                return EventResult::Continue;
            }
        }
    }

} // namespace nav
} // namespace carla
