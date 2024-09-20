// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/NonCopyable.h"

#include "carla/client/TrafficLight.h"
#include "carla/client/World.h"
#include "carla/geom/Location.h"
#include "carla/nav/WalkerEvent.h"
#include "carla/rpc/ActorId.h"
#include "carla/rpc/TrafficLightState.h"

namespace carla {
namespace nav {

    class Navigation;

    // 定义行人状态的枚举类型
    enum WalkerState {
        WALKER_IDLE,
        WALKER_WALKING,
        WALKER_IN_EVENT,
        WALKER_STOP
    };

    // 定义一个结构体用于表示行人路线中的某一点
    struct WalkerRoutePoint {
        WalkerEvent event;// 事件类型
        carla::geom::Location location;// 地点位置
        unsigned char areaType;// 区域类型

        // 构造函数初始化所有成员
        WalkerRoutePoint(WalkerEvent ev, carla::geom::Location loc, unsigned char area) : event(ev), location(loc), areaType(area) {};
    };
    // 定义一个结构体用于表示一个行人的信息
    struct WalkerInfo {
        carla::geom::Location from;
        carla::geom::Location to;
        unsigned int currentIndex { 0 };
        WalkerState state;
        std::vector<WalkerRoutePoint> route;
    };

  // 定义 WalkerManager 类用于管理行人及其路径
  class WalkerManager : private NonCopyable {

  public:

    WalkerManager();
    ~WalkerManager();

    // 设置导航模块的函数
    void SetNav(Navigation *nav);

    // 设置模拟器的引用，允许访问 API 函数
    void SetSimulator(std::weak_ptr<carla::client::detail::Simulator> simulator);

    // 创建新的行人路线
    bool AddWalker(ActorId id);

    // 移除现有的行人路线
    bool RemoveWalker(ActorId id);

    // 更新所有的行人路线
    bool Update(double delta);

    // 从当前路径点设置新的行人路线
    bool SetWalkerRoute(ActorId id);
    bool SetWalkerRoute(ActorId id, carla::geom::Location to);

    // 设置路径中的下一个点
    bool SetWalkerNextPoint(ActorId id);
  
    // 获取路径中的下一个点
    bool GetWalkerNextPoint(ActorId id, carla::geom::Location &location);

    // 获取路径中当前人行横道的结束点
    bool GetWalkerCrosswalkEnd(ActorId id, carla::geom::Location &location);
    
    // 返回导航对象
    Navigation *GetNavigation() { return _nav; };

    // 返回影响特定位置的交通灯
    SharedPtr<carla::client::TrafficLight> GetTrafficLightAffecting(carla::geom::Location UnrealPos, float max_distance = -1.0f);

    private:

    // 获取所有交通灯的路径点
    void GetAllTrafficLightWaypoints();

    // 执行特定事件的处理
    EventResult ExecuteEvent(ActorId id, WalkerInfo &info, double delta);

    std::unordered_map<ActorId, WalkerInfo> _walkers;
    std::vector<std::pair<SharedPtr<carla::client::TrafficLight>, carla::geom::Location>> _traffic_lights;
    Navigation *_nav { nullptr };
    std::weak_ptr<carla::client::detail::Simulator> _simulator;
  };

} // namespace nav
} // namespace carla
