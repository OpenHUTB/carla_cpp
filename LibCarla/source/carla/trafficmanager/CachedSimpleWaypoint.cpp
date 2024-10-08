// Copyright (c) 2021 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/trafficmanager/CachedSimpleWaypoint.h"

namespace carla {
namespace traffic_manager {

  using SimpleWaypointPtr = std::shared_ptr<SimpleWaypoint>; // 定义一个智能指针类型，用于简单路径点

  CachedSimpleWaypoint::CachedSimpleWaypoint(const SimpleWaypointPtr& simple_waypoint) {
    this->waypoint_id = simple_waypoint->GetId(); // 获取并设置路径点的ID

    this->road_id = simple_waypoint->GetWaypoint()->GetRoadId(); // 获取并设置道路ID
    this->section_id = simple_waypoint->GetWaypoint()->GetSectionId(); // 获取并设置路段ID
    this->lane_id = simple_waypoint->GetWaypoint()->GetLaneId(); // 获取并设置车道ID
    this->s = static_cast<float>(simple_waypoint->GetWaypoint()->GetDistance()); // 获取并设置距离

    // 获取下一个路径点的ID并保存到列表中
    for (auto &wp : simple_waypoint->GetNextWaypoint()) {
      this->next_waypoints.push_back(wp->GetId());
    }
    
    // 获取前一个路径点的ID并保存到列表中
    for (auto &wp : simple_waypoint->GetPreviousWaypoint()) {
      this->previous_waypoints.push_back(wp->GetId());
    }

    // 如果有左侧路径点，则获取其ID
    if (simple_waypoint->GetLeftWaypoint() != nullptr) {
      this->next_left_waypoint = simple_waypoint->GetLeftWaypoint()->GetId();
    }
    // 如果有右侧路径点，则获取其ID
    if (simple_waypoint->GetRightWaypoint() != nullptr) {
      this->next_right_waypoint = simple_waypoint->GetRightWaypoint()->GetId();
    }

    this->geodesic_grid_id = simple_waypoint->GetGeodesicGridId(); // 获取并设置测地网格ID
    this->is_junction = simple_waypoint->CheckJunction(); // 检查是否为交叉口
    this->road_option = static_cast<uint8_t>(simple_waypoint->GetRoadOption()); // 获取道路选项并转换为uint8_t类型
  }

  void CachedSimpleWaypoint::Write(std::ofstream &out_file) {
    // 写入路径点ID
    WriteValue<uint64_t>(out_file, this->waypoint_id);

    // 写入道路ID、路段ID、车道ID和距离
    WriteValue<uint32_t>(out_file, this->road_id);
    WriteValue<uint32_t>(out_file, this->section_id);
    WriteValue<int32_t>(out_file, this->lane_id);
    WriteValue<float>(out_file, this->s);

    // 写入下一个路径点的数量和ID
    uint16_t total_next = static_cast<uint16_t>(this->next_waypoints.size());
    WriteValue<uint16_t>(out_file, total_next);
    for (auto &id : this->next_waypoints) {
      WriteValue<uint64_t>(out_file, id);
    }

    // 写入前一个路径点的数量和ID
    uint16_t total_previous = static_cast<uint16_t>(this->previous_waypoints.size());
    WriteValue<uint16_t>(out_file, total_previous);
    for (auto &id : this->previous_waypoints) {
      WriteValue<uint64_t>(out_file, id);
    }

    // 写入左侧和右侧路径点的ID
    WriteValue<uint64_t>(out_file, this->next_left_waypoint);
    WriteValue<uint64_t>(out_file, this->next_right_waypoint);

    // 写入测地网格ID
    WriteValue<int32_t>(out_file, this->geodesic_grid_id);

    // 写入是否为交叉口的布尔值
    WriteValue<bool>(out_file, this->is_junction);

    // 写入道路选项
    WriteValue<uint8_t>(out_file, this->road_option);
  }

  void CachedSimpleWaypoint::Read(std::ifstream &in_file) {
    // 从文件中读取路径点ID
    ReadValue<uint64_t>(in_file, this->waypoint_id);

    // 从文件中读取道路ID、路段ID、车道ID和距离
    ReadValue<uint32_t>(in_file, this->road_id);
    ReadValue<uint32_t>(in_file, this->section_id);
    ReadValue<int32_t>(in_file, this->lane_id);
    ReadValue<float>(in_file, this->s);

    // 从文件中读取下一个路径点的数量和ID
    uint16_t total_next;
    ReadValue<uint16_t>(in_file, total_next);
    for (uint16_t i = 0; i < total_next; i++) {
      uint64_t id;
      ReadValue<uint64_t>(in_file, id);
      this->next_waypoints.push_back(id);
    }

    // 从文件中读取前一个路径点的数量和ID
    uint16_t total_previous;
    ReadValue<uint16_t>(in_file, total_previous);
    for (uint16_t i = 0; i < total_previous; i++) {
      uint64_t id;
      ReadValue<uint64_t>(in_file, id);
      this->previous_waypoints.push_back(id);
    }

    // 从文件中读取左侧和右侧路径点的ID
   ReadValue<uint64_t>(in_file, this->next_left_waypoint); // 从文件中读取左侧路径点的ID
ReadValue<uint64_t>(in_file, this->next_right_waypoint); // 从文件中读取右侧路径点的ID

// geo_grid_id
ReadValue<int32_t>(in_file, this->geodesic_grid_id); // 从文件中读取测地网格ID

// is_junction
ReadValue<bool>(in_file, this->is_junction); // 从文件中读取是否为交叉口的布尔值

// road_option
ReadValue<uint8_t>(in_file, this->road_option); // 从文件中读取道路选项

void CachedSimpleWaypoint::Read(const std::vector<uint8_t>& content, unsigned long& start) {
    ReadValue<uint64_t>(content, start, this->waypoint_id); // 从字节数组中读取路径点ID

    // road_id, section_id, lane_id, s
    ReadValue<uint32_t>(content, start, this->road_id); // 从字节数组中读取道路ID
    ReadValue<uint32_t>(content, start, this->section_id); // 从字节数组中读取路段ID
    ReadValue<int32_t>(content, start, this->lane_id); // 从字节数组中读取车道ID
    ReadValue<float>(content, start, this->s); // 从字节数组中读取距离

    // list_of_next
    uint16_t total_next; // 定义变量来存储下一个路径点的数量
    ReadValue<uint16_t>(content, start, total_next); // 从字节数组中读取下一个路径点的数量
    for (uint16_t i = 0; i < total_next; i++) { // 遍历每个下一个路径点
        uint64_t id; // 定义变量来存储路径点ID
        ReadValue<uint64_t>(content, start, id); // 从字节数组中读取路径点ID
        this->next_waypoints.push_back(id); // 将路径点ID添加到列表中
    }

    // list_of_previous
    uint16_t total_previous; // 定义变量来存储前一个路径点的数量
    ReadValue<uint16_t>(content, start, total_previous); // 从字节数组中读取前一个路径点的数量
    for (uint16_t i = 0; i < total_previous; i++) { // 遍历每个前一个路径点
        uint64_t id; // 定义变量来存储路径点ID
        ReadValue<uint64_t>(content, start, id); // 从字节数组中读取路径点ID
        this->previous_waypoints.push_back(id); // 将路径点ID添加到列表中
    }

    // left, right
    ReadValue<uint64_t>(content, start, this->next_left_waypoint); // 从字节数组中读取左侧路径点的ID
    ReadValue<uint64_t>(content, start, this->next_right_waypoint); // 从字节数组中读取右侧路径点的ID

    // geo_grid_id
    ReadValue<int32_t>(content, start, this->geodesic_grid_id); // 从字节数组中读取测地网格ID

    // is_junction
    ReadValue<bool>(content, start, this->is_junction); // 从字节数组中读取是否为交叉口的布尔值

    // road_option
    ReadValue<uint8_t>(content, start, this->road_option); // 从字节数组中读取道路选项
}

} // namespace traffic_manager
} // namespace carla
