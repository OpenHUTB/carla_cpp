
#pragma once

#include "carla/trafficmanager/DataStructures.h" // 引入交通管理模块的数据结构定义
#include "carla/trafficmanager/Parameters.h" // 引入交通管理模块的参数定义
#include "carla/trafficmanager/RandomGenerator.h" // 引入交通管理模块的随机数生成器定义
#include "carla/trafficmanager/SimulationState.h" // 引入交通管理模块的模拟状态定义
#include "carla/trafficmanager/Stage.h" // 引入交通管理模块的阶段定义

namespace carla {
namespace traffic_manager {

/// VehicleLightStage类负责根据车辆当前的状态和周围环境来开启或关闭车辆的灯光
///
class VehicleLightStage: Stage {
private:
  const std::vector<ActorId> &vehicle_id_list; // 车辆ID列表的引用
  const BufferMap &buffer_map;  // 一个常量引用，包含了交通管理模块的缓冲区映射
  const Parameters &parameters; // 一个常量引用，包含了交通管理模块的参数
  const cc::World &world; // 一个常量引用，指向当前的仿真世界，用于获取环境信息
  ControlFrame& control_frame; // 一个引用，指向当前的控制帧，用于更新车辆控制信息
  /// 一个列表，包含了所有车辆的灯光状态，用于管理和更新车辆的灯光
  rpc::VehicleLightStateList all_light_states;
  /// 当前的天气参数，用于根据天气情况调整车辆灯光
  rpc::WeatherParameters weather;

public:
  VehicleLightStage(const std::vector<ActorId> &vehicle_id_list, // VehicleLightStage类的构造函数，初始化成员变量
                    const BufferMap &buffer_map,
                    const Parameters &parameters,
                    const cc::World &world,
                    ControlFrame& control_frame);

  void UpdateWorldInfo(); // 更新世界信息

  void Update(const unsigned long index) override; // 根据给定的索引更新特定车辆的灯光状态

  void RemoveActor(const ActorId actor_id) override; // 当车辆被移除时，从车辆灯光控制列表中移除该车辆

  void Reset() override;  // 重置车辆灯光控制阶段，可能在仿真重置或重新开始时调用
};

} // namespace traffic_manager
} // namespace carla
