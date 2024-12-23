// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.
// 预处理指令，确保头文件只被包含一次，避免重复定义问题
#pragma once
// 包含交通标志（TrafficSign）相关的头文件，可能定义了交通标志相关的类、接口等基础内容，因为TrafficLight类继承自TrafficSign，所以需要引入其定义
#include "carla/client/TrafficSign.h" // 包含交通灯状态（TrafficLightState）相关的定义，用于表示交通灯的不同状态（如红灯、绿灯、黄灯等），在设置和获取交通灯状态等操作中会用到这个类型
#include "carla/rpc/TrafficLightState.h" // 包含路点（Waypoint）相关的头文件，路点通常用于表示道路上的关键位置点等信息，在获取与交通灯相关的路点等操作中会涉及该类型
#include "carla/client/Waypoint.h" // 包含地图（Map）相关的头文件，可能提供了地图相关的操作接口、地图数据结构等定义，例如获取交通灯在地图中的相关信息等操作可能会依赖地图相关功能
#include "carla/client/Map.h" // 包含包围盒（BoundingBox）相关的头文件，用于表示物体在空间中的范围等信息，这里可能用于获取交通灯的空间范围相关操作
#include "carla/geom/BoundingBox.h"
// 定义在carla命名空间下的client命名空间中，用于组织和限定交通灯相关代码的作用域，避免命名冲突
namespace carla {
namespace client {
// 定义TrafficLight类，它继承自TrafficSign类，表示交通灯是一种特殊的交通标志，继承关系可以复用TrafficSign类中的部分功能和属性等
  class TrafficLight : public TrafficSign {

  public:
// 显式构造函数，接受一个ActorInitializer类型的参数init，用于初始化TrafficLight对象，通过调用基类（TrafficSign）的移动构造函数（std::move(init)）来完成初始化过程，将相关初始化任务传递给基类
    explicit TrafficLight(ActorInitializer init) : TrafficSign(std::move(init)) {}
// 设置交通灯状态的函数，参数state是要设置的交通灯状态，类型为rpc::TrafficLightState，用于改变交通灯当前的显示状态（如设置为红灯、绿灯等）
    void SetState(rpc::TrafficLightState state);

    /// 返回交通灯的当前状态。
    ///
    /// @note 此函数不调用模拟器，它返回最后一个节拍中接收到的数据。 // 获取交通灯当前状态的函数，返回一个rpc::TrafficLightState类型的状态值，表示当前交通灯处于何种状态（红灯、绿灯或黄灯等），注意该函数不会主动向模拟器发送请求，只是返回之前接收到的数据
    rpc::TrafficLightState GetState() const;
// 设置交通灯绿灯持续时间的函数，参数green_time是要设置的绿灯持续时间，类型为float，用于配置交通灯绿灯亮起的时长
    void SetGreenTime(float green_time);

    /// @note 此函数不调用模拟器，它返回最后一个节拍中接收到的数据。 // 获取交通灯绿灯持续时间的函数，返回一个float类型的时间值，表示当前交通灯绿灯的持续时间，同样该函数不会向模拟器发起请求，而是返回之前获取到的数据
    float GetGreenTime() const;
// 设置交通灯黄灯持续时间的函数，参数yellow_time是要设置的黄灯持续时间，类型为float，用于配置交通灯黄灯亮起的时长
    void SetYellowTime(float yellow_time);

    /// @note 此函数不调用模拟器，它返回最后一个节拍中接收到的数据。// 获取交通灯黄灯持续时间的函数，返回一个float类型的时间值，表示当前交通灯黄灯的持续时间，该函数也只是返回之前记录的数据，不会与模拟器进行交互
    float GetYellowTime() const;
// 设置交通灯红灯持续时间的函数，参数red_time是要设置的红灯持续时间，类型为float，用于配置交通灯红灯亮起的时长
    void SetRedTime(float red_time);

    /// @note 此函数不调用模拟器，它返回最后一个节拍中接收到的数据。// 获取交通灯红灯持续时间的函数，返回一个float类型的时间值，表示当前交通灯红灯的持续时间，和前面类似，它是基于之前获取的数据返回结果，不会主动联系模拟器
    float GetRedTime() const;

    /// @note 此函数不调用模拟器，它返回最后一个节拍中接收到的数据。// 获取交通灯从当前状态开始已经过去的时间的函数，返回一个float类型的时间值，表示交通灯处于当前状态已经持续的时长，同样依赖之前记录的数据，不向模拟器查询
    float GetElapsedTime() const;
// 设置交通灯是否冻结的函数，参数freeze是一个布尔值，用于决定是否冻结交通灯，冻结后交通灯状态将不再自动切换等
    void Freeze(bool freeze);

    /// @note 此函数不调用模拟器，它返回最后一个节拍中接收到的数据。// 判断交通灯是否处于冻结状态的函数，返回一个布尔值，表示交通灯当前是否被冻结，也是基于之前获取的数据来判断，不会向模拟器请求验证
    bool IsFrozen() const;

    /// 返回交通信号灯组中灯杆的索引 // 获取交通信号灯组中灯杆的索引值的函数，返回一个uint32_t类型的索引值，用于标识交通灯在所属灯组中的具体位置等相关信息
    uint32_t GetPoleIndex();

    /// 返回该交通灯所属组中的所有交通灯。
    ///
    /// @note 该函数调用模拟器
    std::vector<SharedPtr<TrafficLight>> GetGroupTrafficLights();

    // 重置所有组的计时器和状态
    void ResetGroup();
// 获取受该交通灯影响的车道路点的函数，返回一个包含指向Waypoint类型智能指针（SharedPtr<Waypoint>）的向量，用于获取道路上受该交通灯影响的关键位置点信息，返回值是const类型，表示该函数不会修改类的成员变量
    std::vector<SharedPtr<Waypoint>> GetAffectedLaneWaypoints() const;
// 获取交通灯的包围盒（BoundingBox）的函数，返回一个包含geom::BoundingBox类型元素的向量，用于获取交通灯在空间中的范围信息，返回值是const类型，表示该函数不会修改类的成员变量
    std::vector<geom::BoundingBox> GetLightBoxes() const;
// 获取交通灯对应的OpenDRIVE标识（ID）的函数，返回一个road::SignId类型的标识值，用于在OpenDRIVE相关的地图数据等场景中标识该交通灯
    road::SignId GetOpenDRIVEID() const;
// 获取交通灯对应的停车路点的函数，返回一个包含指向Waypoint类型智能指针（SharedPtr<Waypoint>）的向量，用于获取在该交通灯前需要停车的关键位置点信息，返回值是const类型，表示该函数不会修改类的成员变量
    std::vector<SharedPtr<Waypoint>> GetStopWaypoints() const;

  };

} // namespace client
} // namespace carla
