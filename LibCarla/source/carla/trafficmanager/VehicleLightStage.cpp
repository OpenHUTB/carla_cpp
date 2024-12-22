
#include "carla/trafficmanager/Constants.h"
#include "carla/trafficmanager/LocalizationUtils.h"

#include "carla/trafficmanager/VehicleLightStage.h"

namespace carla {
namespace traffic_manager {

using namespace constants::VehicleLight;

// VehicleLightStage构造函数
VehicleLightStage::VehicleLightStage(
  const std::vector<ActorId> &vehicle_id_list, // 车辆ID列表
  const BufferMap &buffer_map, // 缓冲区映射
  const Parameters &parameters, // 参数设置
  const cc::World &world, // 世界模型
  ControlFrame& control_frame) // 控制帧
  : vehicle_id_list(vehicle_id_list), // 初始化车辆ID列表
    buffer_map(buffer_map), // 初始化缓冲区映射
    parameters(parameters), // 初始化参数
    world(world), // 初始化世界模型
    control_frame(control_frame) {} // 初始化控制帧

// 更新世界信息
void VehicleLightStage::UpdateWorldInfo() {
  // 一次性获取全局天气和所有车辆的灯光状态
  all_light_states = world.GetVehiclesLightStates(); // 获取所有车辆灯光状态
  weather = world.GetWeather(); // 获取当前天气
}

// 更新车辆状态
void VehicleLightStage::Update(const unsigned long index) {
  ActorId actor_id = vehicle_id_list.at(index); // 根据索引获取车辆ID

  if (!parameters.GetUpdateVehicleLights(actor_id))
    return; // 如果该车辆未设置为自动更新灯光状态，则返回

  rpc::VehicleLightState::flag_type light_states = uint32_t(-1); // 初始化灯光状态
  bool brake_lights = false; // 刹车灯状态
  bool left_turn_indicator = false; // 左转指示灯状态
  bool right_turn_indicator = false; // 右转指示灯状态
  bool position = false; // 位置灯状态
  bool low_beam = false; // 近光灯状态
  bool high_beam = false; // 高光灯状态
  bool fog_lights = false; // 雾灯状态

  // 查找当前车辆的灯光状态
  for (auto&& vls : all_light_states) {
    if (vls.first == actor_id) { // 如果车辆ID匹配
      light_states = vls.second; // 获取灯光状态
      break; // 找到后退出循环
    }
  }

  // 通过检查临近的路点来判断车辆是否转向
  const Buffer& waypoint_buffer = buffer_map.at(actor_id); // 获取车辆的路点缓冲区
  cg::Location front_location = waypoint_buffer.front()->GetLocation(); // 获取车辆前方位置

  for (const SimpleWaypointPtr& waypoint : waypoint_buffer) { // 遍历路点
    if (waypoint->CheckJunction()) { // 检查是否在交叉口
      RoadOption target_ro = waypoint->GetRoadOption(); // 获取目标道路选项
      if (target_ro == RoadOption::Left) left_turn_indicator = true; // 如果是左转，设置左转指示灯
      else if (target_ro == RoadOption::Right) right_turn_indicator = true; // 如果是右转，设置右转指示灯
      break; // 找到后退出循环
    }
    // 如果前方位置与路点距离超过最大检查距离，退出循环
    if (cg::Math::DistanceSquared(front_location, waypoint->GetLocation()) > MAX_DISTANCE_LIGHT_CHECK) {
      break;
    }
  }

  // 确定刹车灯状态
  for (size_t cc = 0; cc < control_frame.size(); cc++) { // 遍历控制帧
    if (auto* maybe_ctrl = boost::variant2::get_if<carla::rpc::Command::ApplyVehicleControl>(&control_frame[cc].command)) {
      carla::rpc::Command::ApplyVehicleControl& ctrl = *maybe_ctrl; // 获取控制命令
      if (ctrl.actor == actor_id) { // 如果控制命令的车辆ID匹配
        brake_lights = (ctrl.control.brake > 0.5); // 如果刹车值大于0.5，表示硬刹车，设置刹车灯
        break; // 找到后退出循环
      }
    }
  }

    // 确定位置灯、雾灯和光束状态

    // 在日落到黎明之间开启光束和位置灯
     if (weather.sun_altitude_angle < SUN_ALTITUDE_DEGREES_BEFORE_DAWN || // 如果太阳高度角小于黎明前的阈值
        weather.sun_altitude_angle > SUN_ALTITUDE_DEGREES_AFTER_SUNSET) // 或者大于日落后的阈值
    {
        position = true; // 开启位置灯
        low_beam = true; // 开启近光灯
    }
    else if (weather.sun_altitude_angle < SUN_ALTITUDE_DEGREES_JUST_AFTER_DAWN || // 如果太阳高度角小于黎明后刚过的阈值
             weather.sun_altitude_angle > SUN_ALTITUDE_DEGREES_JUST_BEFORE_SUNSET) // 或者大于日落前刚过的阈值
    {
        position = true; // 开启位置灯
    }

    // 在大雨天气下开启灯光
    if (weather.precipitation > HEAVY_PRECIPITATION_THRESHOLD) { // 如果降水量超过大雨阈值
        position = true; // 开启位置灯
        low_beam = true; // 开启近光灯
    }

    // 开启雾灯
    if (weather.fog_density > FOG_DENSITY_THRESHOLD) { // 如果雾密度超过雾灯阈值
        position = true; // 开启位置灯
        low_beam = true; // 开启近光灯
        fog_lights = true; // 开启雾灯
    }

    // 确定新的车辆灯光状态
    rpc::VehicleLightState::flag_type new_light_states = light_states; // 初始化新的灯光状态为当前状态

    if (brake_lights) // 如果刹车灯开启
        new_light_states |= rpc::VehicleLightState::flag_type(rpc::VehicleLightState::LightState::Brake); // 设置刹车灯状态
    else
        new_light_states &= ~rpc::VehicleLightState::flag_type(rpc::VehicleLightState::LightState::Brake); // 关闭刹车灯状态

    if (left_turn_indicator) // 如果左转指示灯开启
        new_light_states |= rpc::VehicleLightState::flag_type(rpc::VehicleLightState::LightState::LeftBlinker); // 设置左转指示灯状态
    else
        new_light_states &= ~rpc::VehicleLightState::flag_type(rpc::VehicleLightState::LightState::LeftBlinker); // 关闭左转指示灯状态

    if (right_turn_indicator) // 如果右转指示灯开启
        new_light_states |= rpc::VehicleLightState::flag_type(rpc::VehicleLightState::LightState::RightBlinker); // 设置右转指示灯状态
    else
        new_light_states &= ~rpc::VehicleLightState::flag_type(rpc::VehicleLightState::LightState::RightBlinker); // 关闭右转指示灯状态

    if (position) // 如果位置灯开启
        new_light_states |= rpc::VehicleLightState::flag_type(rpc::VehicleLightState::LightState::Position); // 设置位置灯状态
    else
        new_light_states &= ~rpc::VehicleLightState::flag_type(rpc::VehicleLightState::LightState::Position); // 关闭位置灯状态

    if (low_beam) // 如果近光灯开启
        new_light_states |= rpc::VehicleLightState::flag_type(rpc::VehicleLightState::LightState::LowBeam); // 设置近光灯状态
    else
        new_light_states &= ~rpc::VehicleLightState::flag_type(rpc::VehicleLightState::LightState::LowBeam); // 关闭近光灯状态

    if (high_beam) // 如果远光灯开启
        new_light_states |= rpc::VehicleLightState::flag_type(rpc::VehicleLightState::LightState::HighBeam); // 设置远光灯状态
    else
        new_light_states &= ~rpc::VehicleLightState::flag_type(rpc::VehicleLightState::LightState::HighBeam); // 关闭远光灯状态

    if (fog_lights) // 如果雾灯开启
        new_light_states |= rpc::VehicleLightState::flag_type(rpc::VehicleLightState::LightState::Fog); // 设置雾灯状态
    else
        new_light_states &= ~rpc::VehicleLightState::flag_type(rpc::VehicleLightState::LightState::Fog); // 关闭雾灯状态

    // 如果灯光状态发生变化，更新车辆灯光状态
    if (new_light_states != light_states) // 检查新的灯光状态是否与当前状态不同
        control_frame.push_back(carla::rpc::Command::SetVehicleLightState(actor_id, new_light_states)); // 更新灯光状态命令

    void VehicleLightStage::RemoveActor(const ActorId) { // 移除车辆的函数（尚未实现）
    }


    void VehicleLightStage::Reset() { // 重置车辆灯光状态的函数（尚未实现）
    }

} // namespace traffic_manager
} // namespace carla
