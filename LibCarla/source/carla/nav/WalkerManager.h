// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.
// 这是一个头文件保护指令，确保该头文件在被多次包含时，其内容只会被编译一次
#pragma once
// 包含Carla项目中定义的不可复制（NonCopyable）相关的头文件，可能用于禁止类对象被拷贝，
// 以此来确保某些对象在使用过程中的唯一性或者符合特定的资源管理逻辑
#include "carla/NonCopyable.h"
// 包含Carla客户端中与交通信号灯（TrafficLight）相关的头文件，用于处理交通信号灯的状态、属性等相关操作
#include "carla/client/TrafficLight.h" // 包含Carla客户端中与整个虚拟世界（World）相关的头文件，可能用于访问世界中的各种实体、获取世界相关的属性等操作
#include "carla/client/World.h"// 包含Carla项目中几何位置（Location）相关的头文件，用于表示虚拟世界中的点坐标等几何信息，比如行人、车辆等的位置
#include "carla/geom/Location.h"// 包含Carla项目中导航（nav）相关的行人事件（WalkerEvent）头文件，可能用于定义行人在行走过程中遇到的各种事件类型
#include "carla/nav/WalkerEvent.h"// 包含Carla项目中远程过程调用（RPC）相关的演员（Actor）标识符（ActorId）头文件，用于唯一标识虚拟世界中的各种实体（如行人、车辆等）
#include "carla/rpc/ActorId.h"// 包含Carla项目中远程过程调用（RPC）相关的交通信号灯状态（TrafficLightState）头文件，用于表示交通信号灯的不同状态（如红灯、绿灯等）
#include "carla/rpc/TrafficLightState.h"
// 定义在Carla项目的nav命名空间下，表明这些类、结构体和函数是与导航相关功能实现的一部分，特别是针对行人导航方面
namespace carla {
namespace nav {
// 前向声明Navigation类，告知编译器后续会定义该类，但在此处先不用知道其具体实现细节，
    // 这样可以解决类之间相互包含时可能出现的编译顺序问题
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

    WalkerManager();   // 构造函数，用于创建WalkerManager对象，初始化该对象的内部状态，
                         // 可能会对成员变量进行初始化或者设置一些默认值，为后续管理行人及其路径做好准备。
  
    ~WalkerManager();    // WalkerManager 的析构函数。
    // 它是一个虚函数（尽管这里没有使用 virtual 关键字，但在实际设计中可能会是虚的，
    // 特别是如果 WalkerManager 是多态基类的话），用于在 WalkerManager 对象被销毁时执行清理操作。
    // 析构函数体可能释放了对象在生命周期中分配的资源，或者执行了其他必要的清理工作。

 // 设置导航模块的函数，接受一个Navigation类型的指针参数，用于将外部定义的导航模块关联到WalkerManager对象中，
        // 使得WalkerManager可以利用该导航模块来进行行人路径规划等相关操作
    void SetNav(Navigation *nav);
// 设置模拟器的引用，接受一个指向weak_ptr<carla::client::detail::Simulator>类型的参数，
        // 通过这种弱引用的方式，可以访问模拟器相关的API函数，同时避免了循环引用等可能导致的资源管理问题，
        // 方便在管理行人时与模拟器进行交互，比如获取世界状态、更新实体信息等操作
   
    void SetSimulator(std::weak_ptr<carla::client::detail::Simulator> simulator);
// 创建新的行人路线，接受一个ActorId类型的参数，用于唯一标识要创建路线的行人，
        // 函数内部会根据相关逻辑为该行人创建合适的行走路线，返回一个布尔值表示创建操作是否成功
   
    bool AddWalker(ActorId id);
// 移除现有的行人路线，同样接受一个ActorId类型的参数，用于指定要移除路线的行人，
        // 函数会清理与该行人路线相关的各种资源和信息，返回布尔值表示移除操作是否成功
  
    bool RemoveWalker(ActorId id);
// 更新所有的行人路线，接受一个双精度浮点数类型的参数delta，该参数可能表示时间间隔等信息，
        // 函数会根据这个时间间隔以及行人的当前状态、路线等信息，对所有行人的路线进行更新，比如调整行人位置、处理事件等操作，
        // 返回布尔值表示更新操作是否成功

    bool Update(double delta);
// 从当前路径点设置新的行人路线，有两个重载版本：

        // 第一个重载版本只接受一个ActorId类型的参数，用于指定要设置路线的行人，
        // 函数会基于当前路径点等相关信息为该行人重新规划路线，返回布尔值表示操作是否成功
   
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
// 执行特定事件的处理，接受一个ActorId类型的参数用于指定行人，一个WalkerInfo类型的引用参数用于获取和更新行人的信息，
        // 以及一个双精度浮点数类型的参数delta（可能表示时间间隔等信息），
        // 函数会根据行人当前遇到的事件以及相关状态，执行相应的处理逻辑，比如等待交通灯、通过路口等操作，返回处理结果（EventResult类型，具体类型定义可能在别处）
  
    EventResult ExecuteEvent(ActorId id, WalkerInfo &info, double delta);
// 使用无序映射（unordered_map）数据结构来存储每个行人（以ActorId作为键）对应的行人信息（WalkerInfo结构体），
        // 方便快速查找、添加、删除和更新每个行人的相关信息
    std::unordered_map<ActorId, WalkerInfo> _walkers;// 使用向量（vector）数据结构来存储交通灯相关的信息，每个元素是一个包含交通灯共享指针（SharedPtr<carla::client::TrafficLight>）
        // 和对应的地理位置（carla::geom::Location）的pair结构体，用于管理和查询交通灯及其位置信息
    std::vector<std::pair<SharedPtr<carla::client::TrafficLight>, carla::geom::Location>> _traffic_lights;// 指向Navigation对象的指针，用于关联外部的导航模块，初始化为nullptr，后续通过SetNav函数进行赋值
    Navigation *_nav { nullptr };// 使用弱引用（weak_ptr）来存储指向模拟器（Simulator）对象的指针，避免强引用可能导致的循环引用问题，
        // 同时又能通过该弱引用在需要时访问模拟器相关的API函数
    std::weak_ptr<carla::client::detail::Simulator> _simulator;
  };

} // namespace nav
} // namespace carla
