// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/road/element/Waypoint.h" // 引入Waypoint类的定义  
  
#include <boost/container_hash/hash.hpp> // 引入Boost库中的hash工具，用于辅助生成哈希值  
  
// 在std命名空间中定义一个新的类型别名和使用自定义的哈希函数  
namespace std {  
  
  // 使用std::hash模板特化，为carla::road::element::Waypoint类定义一个哈希函数类型  
  using WaypointHash = hash<carla::road::element::Waypoint>;  
  
  // 定义WaypointHash的operator()函数，这是哈希函数的核心，用于计算Waypoint对象的哈希值  
  WaypointHash::result_type WaypointHash::operator()(const argument_type &waypoint) const {  
    // 初始化哈希种子为0  
    WaypointHash::result_type seed = 0u;  
      
    // 使用boost::hash_combine函数结合waypoint的各个属性（road_id, section_id, lane_id, s的近似值）来生成哈希值  
    // road_id, section_id, lane_id是Waypoint对象的标识符，s是Waypoint在道路上的位置（可能是以米为单位）  
    // 注意：s的值被乘以200并向下取整到最近的浮点数，这可能是为了增加哈希值的分布范围，减少哈希冲突  
    boost::hash_combine(seed, waypoint.road_id);  
    boost::hash_combine(seed, waypoint.section_id);  
    boost::hash_combine(seed, waypoint.lane_id);  
    boost::hash_combine(seed, static_cast<float>(std::floor(waypoint.s * 200.0)));  
      
    // 返回最终生成的哈希值  
    return seed;  
  }  
  
} // namespace std
