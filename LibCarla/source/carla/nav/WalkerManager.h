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
        WALKER_IDLE,// 行人空闲状态，可能表示行人当前没有进行任何明显的移动或活动。
        WALKER_WALKING,// 行人行走状态，表示行人正在移动中。
        WALKER_IN_EVENT,// 行人处于事件中的状态，这个状态的具体含义可能依赖于上下文，
// 但通常表示行人正在参与某种特定的事件或活动，比如过马路、等待交通灯等。
        WALKER_STOP// 行人空闲状态，可能表示行人当前没有进行任何明显的移动或活动。
    };

    // 定义一个结构体用于表示行人路线中的某一点
    struct WalkerRoutePoint {
        WalkerEvent event;// 事件类型
        carla::geom::Location location;// 地点位置
        unsigned char areaType;// 区域类型

        // 构造函数初始化所有成员
        WalkerRoutePoint(WalkerEvent ev, carla::geom::Location loc, unsigned char area) : event(ev), location(loc), areaType(area) {};
    };// 使用传入的 WalkerEvent 参数来初始化成员变量 event
     // 使用传入的 carla::geom::Location 参数来初始化成员变量 location
    // 使用传入的 unsigned char 参数来初始化成员变量 areaType
    // 构造函数体为空，因为所有的初始化工作都已经在初始化列表中完成了。
    // 初始化列表（即冒号后面的部分）是一种在构造函数体执行之前初始化成员变量的高效方式。
    // 定义一个结构体用于表示一个行人的信息
    struct WalkerInfo {// WalkerInfo 结构体，用于存储行人的相关信息。
        carla::geom::Location from;   // from 成员变量，表示行人出发的地点。
    // 它是一个 carla::geom::Location 类型的对象，该类型很可能在 CARLA 仿真环境中定义，用于表示二维或三维空间中的点。
        carla::geom::Location to; // to 成员变量，表示行人到达的地点。
    // 它也是一个 carla::geom::Location 类型的对象，与 from 成员变量类似，用于指定行人的目的地。
        unsigned int currentIndex { 0 };   // currentIndex 成员变量，表示行人当前在其路线中的索引位置。
    // 它是一个 unsigned int 类型的值，初始化为 0，意味着默认情况下行人处于路线的起点。
        WalkerState state;    // state 成员变量，表示行人的当前状态。
    // WalkerState 是一个枚举类型（或可能是另一个结构体/类），用于定义行人可能处于的不同状态，如行走、空闲、停止等。
        std::vector<WalkerRoutePoint> route;    // route 成员变量，表示行人的路线。
    // 它是一个 std::vector<WalkerRoutePoint> 类型的容器，其中 WalkerRoutePoint 是一个结构体或类，用于存储路线上的单个点（或事件）的信息。
    // std::vector 是C++标准库中的一个动态数组容器，能够根据需要自动调整其大小。
    };

  // 定义 WalkerManager 类用于管理行人及其路径
  class WalkerManager : private NonCopyable {// WalkerManager 类，用于管理行人的行为和状态。
// 它私有地继承（或更可能是“包含”了一个被简化为NonCopyable的类，用于禁止拷贝）了 NonCopyable 类，
// 以防止 WalkerManager 对象被意外地拷贝或赋值。
  public:

    WalkerManager();    // WalkerManager 的构造函数。
    // 它是一个无参构造函数，用于初始化 WalkerManager 对象。
    // 构造函数体可能包含了对成员变量的初始化，或者设置了某些初始状态。
    ~WalkerManager();    // WalkerManager 的析构函数。
    // 它是一个虚函数（尽管这里没有使用 virtual 关键字，但在实际设计中可能会是虚的，
    // 特别是如果 WalkerManager 是多态基类的话），用于在 WalkerManager 对象被销毁时执行清理操作。
    // 析构函数体可能释放了对象在生命周期中分配的资源，或者执行了其他必要的清理工作。

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
    bool SetWalkerRoute(ActorId id);// SetWalkerRoute 函数的一个重载版本，它接受一个参数：
// ActorId id - 一个标识符，用于唯一指定要设置路线的行人（Walker）。
// 这个函数的作用是设置指定行人的路线，但具体的路线信息（如起点、途经点、终点等）
// 可能是在函数内部以某种方式预定义的，或者是在之前的某个时刻已经设置的。
// 函数返回一个布尔值，表示操作是否成功。
    bool SetWalkerRoute(ActorId id, carla::geom::Location to);// SetWalkerRoute 函数的另一个重载版本，它接受两个参数：
// ActorId id - 同样是一个标识符，用于唯一指定要设置路线的行人。
// carla::geom::Location to - 一个地点，表示行人路线的终点。
// 这个函数的作用是设置指定行人的路线，其中终点是明确指定的。
// 起点可能是默认的、之前设置的，或者是通过其他方式确定的。
// 函数同样返回一个布尔值，表示操作是否成功。

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
