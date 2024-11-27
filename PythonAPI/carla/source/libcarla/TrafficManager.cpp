// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <chrono> // 包含处理时间的标准库
#include <memory> // 包含智能指针等内存管理功能的标准库
#include <stdio.h> // 包含标准输入输出功能的库
#include "carla/PythonUtil.h" // 可能是CARLA的Python工具头文件，用于Python与C++交互
#include "boost/python/suite/indexing/vector_indexing_suite.hpp" // Boost.Python库，用于使C++ std::vector可在Python中索引
 
#include "carla/trafficmanager/TrafficManager.h" // CARLA交通管理器的头文件
#include "carla/trafficmanager/SimpleWaypoint.h" // CARLA简单路点的头文件
 
// 使用typedef定义别名，方便后续代码使用
using ActorPtr = carla::SharedPtr<carla::client::Actor>; // Actor的智能指针
using ActorId = carla::ActorId; // Actor的唯一标识符
 
// 将carla::client::RoadOption枚举值转换为字符串
const char* RoadOptionToString(carla::client::RoadOption value) {
  switch (value)
  {
      // 根据枚举值返回对应的字符串
      case carla::client::RoadOption::Void:   return "Void";
      case carla::client::RoadOption::Left:   return "Left";
      case carla::client::RoadOption::Right:   return "Right";
      case carla::client::RoadOption::Straight: return "Straight";
      case carla::client::RoadOption::LaneFollow:   return "LaneFollow";
      case carla::client::RoadOption::ChangeLaneLeft:   return "ChangeLaneLeft";
      case carla::client::RoadOption::ChangeLaneRight: return "ChangeLaneRight";
      case carla::client::RoadOption::RoadEnd:   return "RoadEnd";
      default:      return "[Unknown RoadOption]";
  }
}
 
// 将Python列表（包含字符串）转换为uint8_t的vector，每个字符串对应一个道路选项
std::vector<uint8_t> RoadOptionToUint(boost::python::list input) {
  std::vector<uint8_t> route;                                       // 用于存储转换结果的vector
  for (int i = 0; i < len(input); ++i) {                             // 遍历输入列表
    uint8_t val;                                                            // 用于存储转换后的uint8_t值
    char* str = boost::python::extract<char*>(input[i]); // 提取列表中的字符串元素
    // 根据字符串值设置对应的uint8_t值
    if (strcmp(str,"Void") == 0) val = 0u;
    else if (strcmp(str,"Left") == 0) val = 1u;
    else if (strcmp(str,"Right") == 0) val = 2u;
    else if (strcmp(str,"Straight") == 0) val = 3u;
    else if (strcmp(str,"LaneFollow") == 0) val = 4u;
    else if (strcmp(str,"ChangeLaneLeft") == 0) val = 5u;
    else if (strcmp(str,"ChangeLaneRight") == 0) val = 6u;
    else if (strcmp(str,"RoadEnd") == 0) val = 7u;
    else val = 10u; // 未知选项设置为10
    route.push_back(val); // 将转换后的值添加到vector中
  }
  return route;
}
 
// 设置自定义路径
void InterSetCustomPath(carla::traffic_manager::TrafficManager& self, const ActorPtr &actor, boost::python::list input, bool empty_buffer) {
  self.SetCustomPath(actor, PythonLitstToVector<carla::geom::Location>(input), empty_buffer); // 调用TrafficManager的SetCustomPath方法，注意PythonLitstToVector函数未在代码中定义，可能是自定义的转换函数
}
 
// 设置导入的路线
void InterSetImportedRoute(carla::traffic_manager::TrafficManager& self, const ActorPtr &actor, boost::python::list input, bool empty_buffer) {
  self.SetImportedRoute(actor, RoadOptionToUint(input), empty_buffer); // 调用TrafficManager的SetImportedRoute方法，将Python列表转换为uint8_t的vector作为输入
}
 
// 获取下一个动作
boost::python::list InterGetNextAction(carla::traffic_manager::TrafficManager& self, const ActorPtr &actor_ptr) {
  boost::python::list l; // 用于存储返回结果的Python列表
  auto next_action = self.GetNextAction(actor_ptr->GetId()); // 调用TrafficManager的GetNextAction方法获取下一个动作
  l.append(RoadOptionToString(next_action.first)); // 将动作类型转换为字符串并添加到列表中
  l.append(next_action.second); // 将动作的第二个元素（可能是距离或时间）添加到列表中
  return l;
}
 
// 获取动作缓冲区
boost::python::list InterGetActionBuffer(carla::traffic_manager::TrafficManager& self, const ActorPtr &actor_ptr) {
  boost::python::list l; // 用于存储返回结果的Python列表
  auto action_buffer = self.GetActionBuffer(actor_ptr->GetId()); // 调用TrafficManager的GetActionBuffer方法获取动作缓冲区
  for (auto &next_action : action_buffer) { // 遍历动作缓冲区
    boost::python::list temp; // 用于存储单个动作的临时列表
    temp.append(RoadOptionToString(next_action.first)); // 将动作类型转换为字符串并添加到临时列表中
    temp.append(next_action.second); // 将动作的第二个元素添加到临时列表中
    l.append(temp); // 将临时列表添加到结果列表中
  }
  return l;
}


// 导出TrafficManager相关功能的函数
void export_trafficmanager() {
  namespace cc = carla::client; // 定义别名简化命名空间引用
  namespace ctm = carla::traffic_manager; // 定义别名简化命名空间引用
  using namespace boost::python; // 使用Boost.Python命名空间，方便后续代码调用Boost.Python的功能

  class_<ctm::TrafficManager>("TrafficManager", no_init)
    .def("get_port", &ctm::TrafficManager::Port)
    .def("vehicle_percentage_speed_difference", &ctm::TrafficManager::SetPercentageSpeedDifference, (arg("actor"), arg("percentage")))
    .def("vehicle_lane_offset", &ctm::TrafficManager::SetLaneOffset, (arg("actor"), arg("offset")))
    .def("set_desired_speed", &ctm::TrafficManager::SetDesiredSpeed, (arg("actor"), arg("speed")))
    .def("global_percentage_speed_difference", &ctm::TrafficManager::SetGlobalPercentageSpeedDifference, (arg("percentage")))
    .def("global_lane_offset", &ctm::TrafficManager::SetGlobalLaneOffset, (arg("offset")))
    .def("update_vehicle_lights", &ctm::TrafficManager::SetUpdateVehicleLights, (arg("actor"), arg("do_update")))
    .def("collision_detection", &ctm::TrafficManager::SetCollisionDetection, (arg("reference_actor"), arg("other_actor"), arg("detect_collision")))
    .def("force_lane_change", &ctm::TrafficManager::SetForceLaneChange, (arg("actor"), arg("direction")))
    .def("auto_lane_change", &ctm::TrafficManager::SetAutoLaneChange, (arg("actor"), arg("enable")))
    .def("distance_to_leading_vehicle", &ctm::TrafficManager::SetDistanceToLeadingVehicle, (arg("actor"), arg("distance")))
    .def("ignore_walkers_percentage", &ctm::TrafficManager::SetPercentageIgnoreWalkers, (arg("actor"), arg("perc")))
    .def("ignore_vehicles_percentage", &ctm::TrafficManager::SetPercentageIgnoreVehicles, (arg("actor"), arg("perc")))
    .def("ignore_lights_percentage", &ctm::TrafficManager::SetPercentageRunningLight, (arg("actor"), arg("perc")))
    .def("ignore_signs_percentage", &ctm::TrafficManager::SetPercentageRunningSign, (arg("actor"), arg("perc")))
    .def("set_global_distance_to_leading_vehicle", &ctm::TrafficManager::SetGlobalDistanceToLeadingVehicle, (arg("distance")))
    .def("keep_right_rule_percentage", &ctm::TrafficManager::SetKeepRightPercentage, (arg("actor"), arg("perc")))
    .def("random_left_lanechange_percentage", &ctm::TrafficManager::SetRandomLeftLaneChangePercentage, (arg("actor"), arg("percentage")))
    .def("random_right_lanechange_percentage", &ctm::TrafficManager::SetRandomRightLaneChangePercentage, (arg("actor"), arg("percentage")))
    .def("set_synchronous_mode", &ctm::TrafficManager::SetSynchronousMode, (arg("mode_switch")))
    .def("set_hybrid_physics_mode", &ctm::TrafficManager::SetHybridPhysicsMode, (arg("enabled")))
    .def("set_hybrid_physics_radius", &ctm::TrafficManager::SetHybridPhysicsRadius, (arg("r")))
    .def("set_random_device_seed", &ctm::TrafficManager::SetRandomDeviceSeed, (arg("value")))
    .def("set_osm_mode", &carla::traffic_manager::TrafficManager::SetOSMMode, (arg("mode_switch")))
    .def("set_path", &InterSetCustomPath, (arg("actor"), arg("path"), arg("empty_buffer")=true))
    .def("set_route", &InterSetImportedRoute, (arg("actor"), arg("path"), arg("empty_buffer")=true))
    .def("set_respawn_dormant_vehicles", &carla::traffic_manager::TrafficManager::SetRespawnDormantVehicles, (arg("mode_switch")))
    .def("set_boundaries_respawn_dormant_vehicles", &carla::traffic_manager::TrafficManager::SetBoundariesRespawnDormantVehicles, (arg("lower_bound"), arg("upper_bound")))
    .def("get_next_action", &InterGetNextAction, (arg("actor")))
    .def("get_all_actions", &InterGetActionBuffer, (arg("actor")))
    .def("shut_down", &ctm::TrafficManager::ShutDown);
}
