// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4583)
#pragma warning(disable:4582)
#include <boost/variant2/variant.hpp>
#pragma warning(pop)
#else
#include <boost/variant2/variant.hpp>
#endif

#include "carla/rpc/ActorId.h"

namespace carla {
namespace nav {

    class Navigation;
    class WalkerManager;

    /// 事件的结果
    enum class EventResult : uint8_t {
        Continue,
        End,
        TimeOut
    };

    /// 忽略的空事件
    struct WalkerEventIgnore {
    };

    /// 等待一段时间的事件
    struct WalkerEventWait {
        double time;
        WalkerEventWait(double duration) : time(duration) {};
    };

    /// 暂停并检查附近车辆的事件
    struct WalkerEventStopAndCheck {
        double time;
        bool check_for_trafficlight;
        SharedPtr<carla::client::TrafficLight> actor;
        WalkerEventStopAndCheck(double duration) : time(duration),
                                                   check_for_trafficlight(true)
                                                   {};
    };

    /// 行人事件变体
    using WalkerEvent = boost::variant2::variant<WalkerEventIgnore, WalkerEventWait, WalkerEventStopAndCheck>;

    /// 访问者类
    class WalkerEventVisitor {
        public:
            WalkerEventVisitor(WalkerManager *manager, ActorId id, double delta) : _manager(manager), _id(id), _delta(delta) {};
            EventResult operator()(WalkerEventIgnore &event);
            EventResult operator()(WalkerEventWait &event);
            EventResult operator()(WalkerEventStopAndCheck &event);
        private:
            WalkerManager *_manager { nullptr };
            ActorId _id { 0 };
            double _delta { 0 };
    };

} // namespace nav
} // namespace carla
