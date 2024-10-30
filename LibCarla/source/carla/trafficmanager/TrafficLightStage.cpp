
#include "carla/trafficmanager/Constants.h"
#include "carla/trafficmanager/LocalizationUtils.h"

#include "carla/trafficmanager/TrafficLightStage.h"

namespace carla {
namespace traffic_manager {

// 引入一些常量
using constants::TrafficLight::EXIT_JUNCTION_THRESHOLD; // 退出交叉口的阈值
using constants::TrafficLight::MINIMUM_STOP_TIME; // 最小停止时间
using constants::WaypointSelection::JUNCTION_LOOK_AHEAD; // 交叉口前瞻距离
using constants::MotionPlan::EPSILON_RELATIVE_SPEED; // 相对速度的微小值

// TrafficLightStage 构造函数
TrafficLightStage::TrafficLightStage(
  const std::vector<ActorId> &vehicle_id_list, // 车辆 ID 列表
  const SimulationState &simulation_state, // 仿真状态
  const BufferMap &buffer_map, // 缓存映射
  const Parameters &parameters, // 参数设置
  const cc::World &world, // 世界对象
  TLFrame &output_array, // 输出数组
  RandomGenerator &random_device) // 随机数生成器
  : vehicle_id_list(vehicle_id_list), // 初始化车辆 ID 列表
    simulation_state(simulation_state), // 初始化仿真状态
    buffer_map(buffer_map), // 初始化缓存映射
    parameters(parameters), // 初始化参数
    world(world), // 初始化世界对象
    output_array(output_array), // 初始化输出数组
    random_device(random_device) {} // 初始化随机数生成器

// 更新函数
void TrafficLightStage::Update(const unsigned long index) {
  bool traffic_light_hazard = false; // 交通信号灯危险标志

  const ActorId ego_actor_id = vehicle_id_list.at(index); // 获取当前车辆 ID
  if (!simulation_state.IsDormant(ego_actor_id)) { // 如果车辆不处于休眠状态

    JunctionID current_junction_id = -1; // 当前交叉口 ID 初始化为 -1
    if (vehicle_last_junction.find(ego_actor_id) != vehicle_last_junction.end()) {
      current_junction_id = vehicle_last_junction.at(ego_actor_id); // 获取上次的交叉口 ID
    }
    auto affected_junction_id = GetAffectedJunctionId(ego_actor_id); // 获取受影响的交叉口 ID

    current_timestamp = world.GetSnapshot().GetTimestamp(); // 获取当前时间戳

    const TrafficLightState tl_state = simulation_state.GetTLS(ego_actor_id); // 获取交通信号灯状态
    const TLS traffic_light_state = tl_state.tl_state; // 交通信号灯当前状态
    const bool is_at_traffic_light = tl_state.at_traffic_light; // 判断是否在交通信号灯处

    // 如果车辆在交通信号灯处且信号灯为黄灯或红灯
    if (is_at_traffic_light &&
        traffic_light_state != TLS::Green &&
        traffic_light_state != TLS::Off &&
        parameters.GetPercentageRunningLight(ego_actor_id) <= random_device.next()) {
      // 如果车辆在受交通信号灯影响的非信号交叉口，移除车辆
      if (current_junction_id != -1) {
        RemoveActor(ego_actor_id);
      }
      traffic_light_hazard = true; // 设置交通信号灯危险标志为真
    }
    // 车辆在非信号交叉口，处理其优先级
    // 不要使用下一个条件，因为边界框可能会变为绿色
    else if (current_junction_id != -1) {
      if (affected_junction_id == -1 || affected_junction_id != current_junction_id) {
        RemoveActor(ego_actor_id); // 移除车辆
      } else {
        traffic_light_hazard = HandleNonSignalisedJunction(ego_actor_id, affected_junction_id, current_timestamp); // 处理非信号交叉口
      }
    }
    // 如果在受影响的交叉口且不在交通信号灯处
    else if (affected_junction_id != -1 &&
            !is_at_traffic_light &&
            traffic_light_state != TLS::Green &&
            parameters.GetPercentageRunningSign(ego_actor_id) <= random_device.next()) {

      AddActorToNonSignalisedJunction(ego_actor_id, affected_junction_id); // 将车辆添加到非信号交叉口
      traffic_light_hazard = true; // 设置交通信号灯危险标志为真
    }
  }
  output_array.at(index) = traffic_light_hazard; // 将结果输出到数组
}

// 将车辆添加到非信号交叉口的函数
void TrafficLightStage::AddActorToNonSignalisedJunction(const ActorId ego_actor_id, const JunctionID junction_id) {

  if (entering_vehicles_map.find(junction_id) == entering_vehicles_map.end()) {
    // 初始化新的交叉口映射条目，使用空的车辆双端队列
    std::deque<ActorId> entry_deque;
    entering_vehicles_map.insert({junction_id, entry_deque});
  }

  auto& entering_vehicles = entering_vehicles_map.at(junction_id); // 获取进入该交叉口的车辆列表
  if (std::find(entering_vehicles.begin(), entering_vehicles.end(), ego_actor_id) == entering_vehicles.end()){
    // 如果车辆不在进入列表中，则添加车辆
    entering_vehicles.push_back(ego_actor_id);
    if (vehicle_last_junction.find(ego_actor_id) != vehicle_last_junction.end()) {
      // 如果车辆正在进入另一个交叉口，移除所有存储的数据
      RemoveActor(ego_actor_id);
    }
    vehicle_last_junction.insert({ego_actor_id, junction_id}); // 更新车辆的最后交叉口信息
  }
}

bool TrafficLightStage::HandleNonSignalisedJunction(const ActorId ego_actor_id, const JunctionID junction_id,
                                                    cc::Timestamp timestamp) {

  bool traffic_light_hazard = false; // 初始化交通信号危险标志为假

  auto& entering_vehicles = entering_vehicles_map.at(junction_id); // 获取进入该交叉口的车辆列表

  if (vehicle_stop_time.find(ego_actor_id) == vehicle_stop_time.end()) { // 检查该车辆是否已记录停车时间
    // 确保车辆在执行其他操作之前已经停止
    if (simulation_state.GetVelocity(ego_actor_id).Length() < EPSILON_RELATIVE_SPEED) { // 如果车辆速度接近零
      vehicle_stop_time.insert({ego_actor_id, timestamp}); // 记录停车时间
    }
    traffic_light_hazard = true; // 标记为交通信号危险
  }

  else if (entering_vehicles.front() == ego_actor_id) { // 如果该车辆是进入交叉口的第一辆车
    auto entry_elapsed_seconds = vehicle_stop_time.at(ego_actor_id).elapsed_seconds; // 获取停车已过的时间
    if (timestamp.elapsed_seconds - entry_elapsed_seconds < MINIMUM_STOP_TIME) { // 如果停车时间不足
      // 等待至少最低停车时间再进入交叉口
      traffic_light_hazard = true; // 标记为交通信号危险
    }
  } else {
    // 只有一辆车可以进入交叉口，其他车辆需停下。
    traffic_light_hazard = true; // 标记为交通信号危险
  }
  return traffic_light_hazard; // 返回交通信号危险标志
}

JunctionID TrafficLightStage::GetAffectedJunctionId(const ActorId ego_actor_id) {
    const Buffer &waypoint_buffer = buffer_map.at(ego_actor_id); // 获取车辆的路径缓冲区
    const SimpleWaypointPtr look_ahead_point = GetTargetWaypoint(waypoint_buffer, JUNCTION_LOOK_AHEAD).first; // 获取前方目标路点
    const auto front_point = waypoint_buffer.front(); // 获取路径中的第一个点

    auto look_ahead_junction_id = look_ahead_point->GetJunctionId(); // 获取前方目标路点的交叉口ID
    auto front_junction_id = front_point->GetJunctionId(); // 获取第一个点的交叉口ID

    // 检查车辆是否当前在一个非信号化的交叉口
    JunctionID current_junction_id = -1; // 初始化当前交叉口ID为-1
    if (vehicle_last_junction.find(ego_actor_id) != vehicle_last_junction.end()) { // 如果已记录该车辆的最后交叉口
      current_junction_id = vehicle_last_junction.at(ego_actor_id); // 获取当前交叉口ID
    }

    if (current_junction_id != -1) { // 如果正在处理一个交叉口
      if (current_junction_id == look_ahead_junction_id) { // 如果当前交叉口与前方交叉口相同
        return look_ahead_junction_id; // 返回前方交叉口ID
      } else {
        if (look_ahead_junction_id != -1) { // 如果前方交叉口不为-1
          // 检测到不同的交叉口
          return look_ahead_junction_id; // 返回前方交叉口ID
        } else {
          if (current_junction_id == front_junction_id) { // 如果当前交叉口与第一个点的交叉口相同
            // 仍在同一交叉口
            return front_junction_id; // 返回第一个点的交叉口ID
          } else {
            return -1; // 返回-1，表示没有有效交叉口
          }
        }
      }
    } else {
      // 如果不在处理任何交叉口，返回前方检测到的交叉口
      return look_ahead_junction_id; // 返回前方交叉口ID
    }
}

void TrafficLightStage::RemoveActor(const ActorId actor_id) {
  if (vehicle_last_junction.find(actor_id) != vehicle_last_junction.end()) { // 检查车辆是否有记录的最后交叉口
    auto junction_id = vehicle_last_junction.at(actor_id); // 获取该车辆的最后交叉口ID

    auto& entering_vehicles = entering_vehicles_map.at(junction_id); // 获取进入该交叉口的车辆列表
    auto ent_index = std::find(entering_vehicles.begin(), entering_vehicles.end(), actor_id); // 查找该车辆在列表中的位置
    if (ent_index != entering_vehicles.end()) { // 如果找到了该车辆
      entering_vehicles.erase(ent_index); // 从列表中移除该车辆
    }

    if (vehicle_stop_time.find(actor_id) != vehicle_stop_time.end()) { // 检查车辆是否有停车时间记录
      vehicle_stop_time.erase(actor_id); // 移除该车辆的停车时间记录
    }

    vehicle_last_junction.erase(actor_id); // 移除该车辆的最后交叉口记录
  }
}

void TrafficLightStage::Reset() {
  entering_vehicles_map.clear(); // 清空进入车辆的映射
  vehicle_last_junction.clear(); // 清空最后交叉口的映射
  vehicle_stop_time.clear(); // 清空停车时间记录
}

} // namespace traffic_manager
} // namespace carla