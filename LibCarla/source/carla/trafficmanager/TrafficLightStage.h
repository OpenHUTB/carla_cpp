
#pragma once

#include "carla/trafficmanager/DataStructures.h"
#include "carla/trafficmanager/Parameters.h"
#include "carla/trafficmanager/RandomGenerator.h"
#include "carla/trafficmanager/SimulationState.h"
#include "carla/trafficmanager/Stage.h"

namespace carla {   // 命名空间 carla 开始
namespace traffic_manager {    // 命名空间 traffic_manager 开始

// 这个类具有响应交通信号灯以及管理进入无信号灯路口的功能。
class TrafficLightStage: Stage {        // 定义名为 TrafficLightStage 的类，继承自 Stage 类
private:
  const std::vector<ActorId> &vehicle_id_list;   // 车辆 ID 列表的常量引用
  const SimulationState &simulation_state;   // 模拟状态的常量引用
  const BufferMap &buffer_map;       // 缓冲区映射的常量引用
  const Parameters &parameters;   // 参数的常量引用
  const cc::World &world;          // 世界对象的常量引用

  // 用于处理无信号灯路口的变量

  // 包含按到达时间顺序进入特定路口的车辆的映射。
  std::unordered_map<JunctionID, std::deque<ActorId>> entering_vehicles_map;        // 路口 ID 到车辆 ID 队列的无序映射
  // 将车辆与其当前路口链接起来的映射。用于方便地访问前面的两个映射。
  std::unordered_map<ActorId, JunctionID> vehicle_last_junction;     // 车辆 ID 到路口 ID 的无序映射
  // 包含参与者首次在停车标志处停止的时间戳的映射。
  std::unordered_map<ActorId, cc::Timestamp> vehicle_stop_time;    // 车辆 ID 到时间戳的无序映射
  TLFrame &output_array;   // 输出数组的引用
  RandomGenerator &random_device;        // 随机数生成器的引用
  cc::Timestamp current_timestamp; // 当前时间戳

  // 这个函数控制所有车辆在无信号灯路口的交互。优先级按照到达顺序确定，并且没有两辆车会同时进入路口。只有当前一辆车离开后，下一辆车才能进入。此外，所有车辆在停车标志处总是会刹车一段时间。
  bool HandleNonSignalisedJunction(const ActorId ego_actor_id, const JunctionID junction_id,
                                   cc::Timestamp timestamp);

  // 将车辆初始化为无信号灯路口映射
  void AddActorToNonSignalisedJunction(const ActorId ego_actor_id, const JunctionID junction_id);

  // 获取车辆当前受影响的路口 ID
  JunctionID GetAffectedJunctionId(const ActorId ego_actor_id);

public:
  TrafficLightStage(const std::vector<ActorId> &vehicle_id_list,
                    const SimulationState &Simulation_state,
                    const BufferMap &buffer_map,
                    const Parameters &parameters,
                    const cc::World &world,
                    TLFrame &output_array,
                    RandomGenerator &random_device);
// 构造函数

  void Update(const unsigned long index) override;     // 重写的更新函数

  void RemoveActor(const ActorId actor_id) override;      // 重写的移除参与者函数

  void Reset() override;   // 重写的重置函数

};

} // namespace traffic_manager   // 命名空间 traffic_manager 结束
} // namespace carla      // 命名空间 carla 结束