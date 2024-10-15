// Copyright (c) 2021 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <fstream>

#include "carla/trafficmanager/SimpleWaypoint.h"

namespace carla {  // 命名空间 carla 开始
namespace traffic_manager {   // 命名空间 traffic_manager 开始

  using SimpleWaypointPtr = std::shared_ptr<SimpleWaypoint>;   // 定义类型别名 SimpleWaypointPtr 为 std::shared_ptr<SimpleWaypoint>

  class CachedSimpleWaypoint {   // 定义类 CachedSimpleWaypoint
  public:
    uint64_t waypoint_id;    // 存储路点的唯一标识 ID
    uint32_t road_id;   // 存储路点所在道路的 ID
    uint32_t section_id;    // 存储路点所在路段的 ID
    int32_t lane_id;    // 存储路点在车道上的位置参数 s
    float s;    // 位置参数 s 的类型为浮点数
    std::vector<uint64_t> next_waypoints;    // 存储当前路点的下一个路点的 ID 列表
    std::vector<uint64_t> previous_waypoints;    // 存储当前路点的前一个路点的 ID 列表
    uint64_t next_left_waypoint = 0;   // 存储当前路点左侧下一个路点的 ID，初始值为 0
    uint64_t next_right_waypoint = 0;    // 存储当前路点右侧下一个路点的 ID，初始值为 0
    int32_t geodesic_grid_id;    // 存储与测地线网格相关的 ID
    bool is_junction;   // 表示路点是否在路口的布尔值
    uint8_t road_option;   // 道路选项，可能用于表示路点的特定属性

    CachedSimpleWaypoint() = default;    // 默认构造函数

    CachedSimpleWaypoint(const SimpleWaypointPtr& simple_waypoint);    // 带参数的构造函数，参数为 SimpleWaypoint 的智能指针

    void Read(const std::vector<uint8_t>& content, unsigned long& start);    // 从字节向量中读取路点信息的函数，传入字节向量和起始位置引用

    void Read(std::ifstream &in_file);  // 从输入文件流中读取路点信息的函数

    void Write(std::ofstream &out_file);   // 将路点信息写入输出文件流的函数

  private:
    template <typename T>
    void WriteValue(std::ofstream &out_file, const T &in_obj) {
      out_file.write(reinterpret_cast<const char *>(&in_obj), sizeof(T));
    }
    // 模板函数，将输入对象写入输出文件流，用于将不同类型的数据写入文件

    template <typename T>
    void ReadValue(std::ifstream &in_file, T &out_obj) {
      in_file.read(reinterpret_cast<char *>(&out_obj), sizeof(T));
    }
    // 模板函数，从输入文件流中读取数据并存储到输出对象中，用于从文件中读取不同类型的数据

    template <typename T>
    void ReadValue(const std::vector<uint8_t>& content, unsigned long& start, T &out_obj) {
      memcpy(&out_obj, &content[start], sizeof(T));
      start += sizeof(T);
    }
    // 模板函数，从字节向量中读取数据并存储到输出对象中，同时更新起始位置，用于从字节向量中读取不同类型的数据

  };

} // namespace traffic_manager  // 命名空间 traffic_manager 结束
} // namespace carla     // 命名空间 carla 结束