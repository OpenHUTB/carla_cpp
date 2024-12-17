// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
// 说明该代码遵循MIT许可协议，可通过对应链接查看协议详情
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

// 针对微软编译器（_MSC_VER宏定义表示使用的是微软编译器环境）进行特定的警告处理
// 将当前的警告状态压入栈中，以便后续恢复
#ifdef _MSC_VER
#pragma warning(push)
// 禁用编号为4583的警告，具体该警告含义取决于微软编译器相关定义，通常是和类定义等相关的某种潜在问题提示
#pragma warning(disable:4583)
// 禁用编号为4582的警告，同样具体含义由微软编译器决定，可能也是代码结构等方面的相关提示
#pragma warning(disable:4582)
#endif

// 引入Boost库中variant2相关的头文件，用于定义可以存储多种不同类型值的变体类型，在这里用于定义行人事件可以具有的不同类型情况
#include <boost/variant2/variant.hpp>

#pragma warning(pop)
// 如果前面针对微软编译器做了警告处理，这里恢复之前压入栈的警告状态
#ifdef _MSC_VER
#pragma warning(pop)	
#else
#include <boost/variant2/variant.hpp>
#endif

// 引入Carla项目中rpc相关的ActorId头文件，可能用于标识不同的Actor（如游戏场景中的角色、物体等实体），在行人事件相关操作中可能会用到
#include "carla/rpc/ActorId.h"

namespace carla {
namespace nav {

    // 前置声明Navigation类，具体定义在后续代码中（可能在其他源文件或者此处代码后面的部分），这里先声明以便在当前代码中可以使用该类的指针等情况（比如作为函数参数、成员变量类型等）
    class Navigation;
    // 前置声明WalkerManager类，同理，后续应有其具体定义，此处先声明方便代码结构搭建等操作
    class WalkerManager;

    // 定义EventResult枚举类型，用于表示事件的结果状态，是一个8位无符号整数类型的枚举，各枚举值含义如下：
    /// 事件的结果
    enum class EventResult : uint8_t {
        // 表示事件继续进行，尚未结束
        Continue,
        // 表示事件结束
        End,
        // 表示事件因超时而结束
        TimeOut
    };

    // 定义WalkerEventIgnore结构体，用于表示一种被忽略的空事件情况，该结构体目前没有成员变量，只是一种特定的“空”事件标识
    /// 忽略的空事件
    struct WalkerEventIgnore {
    };

    // 定义WalkerEventWait结构体，用于表示需要等待一段时间的事件，包含一个表示等待时间的成员变量。
    /// 等待一段时间的事件
    struct WalkerEventWait {
        // 表示需要等待的时间，单位根据具体应用场景确定，类型为双精度浮点数
        double time;
        // 构造函数，用于初始化等待时间成员变量，接收一个表示持续时间的参数来设置等待时间
        WalkerEventWait(double duration) : time(duration) {};
    };

    // 定义WalkerEventStopAndCheck结构体，用于表示暂停并检查附近车辆（或交通信号灯等情况）的事件，包含等待时间、是否检查交通信号灯以及相关Actor的指针等成员变量。
    /// 暂停并检查附近车辆的事件
    struct WalkerEventStopAndCheck {
        // 表示暂停等待的时间，单位根据具体应用场景确定，类型为双精度浮点数
        double time;
        // 表示是否检查交通信号灯，类型为布尔值，true表示要检查，false表示不检查
        bool check_for_trafficlight;
        // 指向carla::client::TrafficLight类型的共享指针，用于指向相关的交通信号灯Actor（如果有的话），方便后续操作与之相关的实体
        SharedPtr<carla::client::TrafficLight> actor;
        // 构造函数，用于初始化成员变量，默认设置为要检查交通信号灯，并接收一个表示持续时间的参数来设置等待时间
        WalkerEventStopAndCheck(double duration) : time(duration),
                                                   check_for_trafficlight(true)
                                                   {};
    };

    // 使用Boost库的variant2功能定义WalkerEvent类型，它是一个变体类型，可以存储WalkerEventIgnore、WalkerEventWait、WalkerEventStopAndCheck这几种不同类型的值，
    /// 行人事件变体
    using WalkerEvent = boost::variant2::variant<WalkerEventIgnore, WalkerEventWait, WalkerEventStopAndCheck>;

    // 定义WalkerEventVisitor类，它是一个访问者类，用于根据不同的行人事件类型执行相应的操作，并返回事件结果（EventResult类型），其内部通过重载函数调用运算符针对不同的事件类型进行定制化处理。
    /// 访问者类
    class WalkerEventVisitor {
    public:
        // 构造函数，用于初始化访问者对象的相关成员变量，接收WalkerManager指针、ActorId以及一个时间差值（具体含义根据应用场景确定）作为参数，以便在后续操作中使用这些信息。
        WalkerEventVisitor(WalkerManager *manager, ActorId id, double delta) : _manager(manager), _id(id), _delta(delta) {};
        // 重载函数调用运算符，用于处理WalkerEventIgnore类型的事件，返回对应的事件结果（EventResult类型），具体实现应该在对应的源文件中（此处仅为声明）。
        EventResult operator()(WalkerEventIgnore &event);
        // 重载函数调用运算符，用于处理WalkerEventWait类型的事件，返回对应的事件结果（EventResult类型），具体实现应该在对应的源文件中（此处仅为声明）。
        EventResult operator()(WalkerEventWait &event);
        // 重载函数调用运算符，用于处理WalkerEventStopAndCheck类型的事件，返回对应的事件结果（EventResult类型），具体实现应该在对应的源文件中（此处仅为声明）。
        EventResult operator()(WalkerEventStopAndCheck &event);
    private:
        // 指向WalkerManager类的指针，用于在访问者类中操作与之相关的行人管理等功能，初始化为nullptr，后续应在合适的地方进行赋值等操作。
        WalkerManager *_manager { nullptr };
        // 用于标识某个Actor（如行人等实体）的ID，类型为ActorId，初始值设为0，根据具体应用场景会被赋予相应的值。
        ActorId _id { 0 };
        // 表示一个时间差值，类型为双精度浮点数，初始值为0，其具体含义和用途取决于具体的业务逻辑，比如可能用于计算时间相关的判断等操作。
        double _delta { 0 };
    };

} // namespace nav
} // namespace carla
