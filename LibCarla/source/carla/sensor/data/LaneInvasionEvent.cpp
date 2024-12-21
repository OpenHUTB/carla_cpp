// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
// 说明该代码遵循MIT许可协议，可通过对应链接查看协议详情
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

// 引入Carla项目中传感器数据相关的LaneInvasionEvent头文件，可能包含了车道入侵事件相关的数据结构定义等内容，
// 该文件应该定义了LaneInvasionEvent类，在此处只是使用该类的定义，而不负责其具体实现（类的定义在其他文件中，这里通过包含头文件获取其声明）
#include "carla/sensor/data/LaneInvasionEvent.h"

// 引入Carla项目中的异常处理相关的Exception头文件，用于在代码中抛出和处理异常情况，比如在获取演员（Actor）失败时抛出合适的异常
#include "carla/Exception.h"
// 引入Carla项目中客户端细节相关的Simulator头文件，可能包含了模拟器相关的功能和数据结构，
// 在这里用于获取与模拟器相关的场景（Episode）信息以及通过ID查找演员（Actor）等操作
#include "carla/client/detail/Simulator.h"

namespace carla {
namespace sensor {
namespace data {

  // 定义LaneInvasionEvent类的GetActor函数，该函数用于获取与车道入侵事件相关的演员（Actor）的共享指针（SharedPtr），
  // 这里的演员可能是触发车道入侵事件的车辆或其他相关实体，函数返回值类型为SharedPtr<client::Actor>，表示指向客户端（client）模块中定义的Actor类型的共享指针
  SharedPtr<client::Actor> LaneInvasionEvent::GetActor() const {
    // 调用GetEpisode函数（该函数应该是LaneInvasionEvent类从其父类或其他地方继承而来的成员函数，用于获取当前事件所属的场景（Episode）信息），
    // 并使用Lock函数（具体功能取决于其定义，可能是用于在多线程或异步环境下安全地获取场景对象的某种锁机制，以确保对场景对象的访问是线程安全的），
    // 获取一个指向当前场景的智能指针（episode），如果获取失败（例如场景已经被销毁等情况），episode将为nullptr或者处于无效状态
    auto episode = GetEpisode().Lock();
    // 通过episode智能指针调用GetActorById函数（该函数应该是在Simulator头文件中定义的与模拟器相关的函数，用于根据演员ID查找对应的演员描述信息），
    // 传入当前车道入侵事件的父对象ID（_parent，该成员变量应该是LaneInvasionEvent类中的一个成员变量，用于标识与该事件相关的父对象，比如触发事件的车辆等），
    // 查找对应的演员描述信息，并将结果存储在description变量中，description的类型应该是一个可空的某种类型（根据代码中的has_value函数推测，可能是std::optional或者类似的可表示可选值的类型），
    // 如果找到了对应的演员，description将包含该演员的详细描述信息，否则将处于无值状态
    auto description = episode->GetActorById(_parent);
    // 使用if语句检查description是否有值，如果没有值（意味着没有找到对应的演员，可能是因为该演员已经被销毁或者ID无效等原因），
    // 则通过throw_exception函数（在Exception头文件中定义，用于抛出异常）抛出一个std::runtime_error类型的异常，异常信息为 "LaneInvasionEvent: parent already dead"，
    // 表示在车道入侵事件中，相关的父对象（如触发事件的车辆）已经不存在了，这可能会导致后续操作无法正常进行，所以通过抛出异常来通知调用者出现了错误情况
    if (!description.has_value()) {
      throw_exception(std::runtime_error("LaneInvasionEvent: parent already dead"));
    }
    // 如果description有值，说明找到了对应的演员描述信息，通过episode智能指针调用MakeActor函数（该函数应该是在客户端相关的模块中定义的用于创建演员对象的函数，
    // 可能会根据传入的演员描述信息构建一个实际的演员对象），传入description中的演员描述信息，创建一个对应的演员对象，并返回一个指向该演员对象的共享指针，
    // 这样调用者就可以通过返回的共享指针操作与车道入侵事件相关的演员对象，比如获取演员的属性、执行演员相关的操作等
    return episode->MakeActor(*description);
  }

} // namespace data
} // namespace sensor
} // namespace carla
