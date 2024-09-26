// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/trafficmanager/LocalizationUtils.h" // 引入定位工具的头文件
#include "carla/trafficmanager/Constants.h" // 引入常量的头文件

namespace carla {
namespace traffic_manager {

using constants::Collision::EPSILON; // 使用常量定义的误差值

// 计算偏差的叉积，判断目标位置相对于参考位置的偏差
float DeviationCrossProduct(const cg::Location &reference_location, // 参考位置
                            const cg::Vector3D &heading_vector, // 前进方向向量
                            const cg::Location &target_location) { // 目标位置
  cg::Vector3D next_vector = target_location - reference_location; // 计算目标向量
  next_vector = next_vector.MakeSafeUnitVector(EPSILON); // 将目标向量归一化
  const float cross_z = heading_vector.x * next_vector.y - heading_vector.y * next_vector.x; // 计算z轴上的叉积
  return cross_z; // 返回叉积结果
}

// 计算偏差的点积，判断目标位置与前进方向的对齐程度
float DeviationDotProduct(const cg::Location &reference_location, // 参考位置
                          const cg::Vector3D &heading_vector, // 前进方向向量
                          const cg::Location &target_location) { // 目标位置
  cg::Vector3D next_vector = target_location - reference_location; // 计算目标向量
  next_vector.z = 0.0f; // 忽略z轴分量
  next_vector = next_vector.MakeSafeUnitVector(EPSILON); // 将目标向量归一化
  cg::Vector3D heading_vector_flat(heading_vector.x, heading_vector.y, 0); // 创建平面的前进方向向量
  heading_vector_flat = heading_vector_flat.MakeSafeUnitVector(EPSILON); // 将平面向量归一化
  float dot_product = cg::Math::Dot(next_vector, heading_vector_flat); // 计算点积
  dot_product = std::max(0.0f, std::min(dot_product, 1.0f)); // 限制点积在0到1之间
  return dot_product; // 返回点积结果
}

// 将一个航点添加到缓冲区并更新经过的车辆信息
void PushWaypoint(ActorId actor_id, TrackTraffic &track_traffic, // 车辆ID和交通轨迹引用
                  Buffer &buffer, SimpleWaypointPtr &waypoint) { // 缓冲区和航点指针
  const uint64_t waypoint_id = waypoint->GetId(); // 获取航点ID
  buffer.push_back(waypoint); // 将航点添加到缓冲区
  track_traffic.UpdatePassingVehicle(waypoint_id, actor_id); // 更新经过该航点的车辆信息
}

// 从缓冲区中移除一个航点并更新经过的车辆信息
void PopWaypoint(ActorId actor_id, TrackTraffic &track_traffic, // 车辆ID和交通轨迹引用
                 Buffer &buffer, bool front_or_back) { // 缓冲区和方向标志（前或后）
  SimpleWaypointPtr removed_waypoint = front_or_back ? buffer.front() : buffer.back(); // 根据方向选择移除的航点
  const uint64_t removed_waypoint_id = removed_waypoint->GetId(); // 获取被移除航点的ID
  if (front_or_back) { // 如果是前方
    buffer.pop_front(); // 移除前方航点
  } else { // 如果是后方
    buffer.pop_back(); // 移除后方航点
  }
  track_traffic.RemovePassingVehicle(removed_waypoint_id, actor_id); // 更新经过的车辆信息
}

// 获取目标航点及其索引
TargetWPInfo GetTargetWaypoint(const Buffer &waypoint_buffer, const float &target_point_distance) { // 缓冲区和目标距离
  SimpleWaypointPtr target_waypoint = waypoint_buffer.front(); // 初始化目标航点为缓冲区的第一个航点
  const SimpleWaypointPtr &buffer_front = waypoint_buffer.front(); // 获取缓冲区的前端航点
  uint64_t startPosn = static_cast<uint64_t>(std::fabs(target_point_distance * INV_MAP_RESOLUTION)); // 计算起始位置
  uint64_t index = startPosn; // 初始化索引为起始位置
  
  // 确定是前向还是后向扫描航点缓冲区的条件
  if (startPosn < waypoint_buffer.size()) { // 如果起始位置小于缓冲区大小
    bool mScanForward = false; // 初始化扫描方向标志
    const float target_point_dist_power = target_point_distance * target_point_distance; // 计算目标距离的平方
    if (buffer_front->DistanceSquared(target_waypoint) < target_point_dist_power) { // 判断是否需要前向扫描
      mScanForward = true;
    }

    // 前向扫描航点
    if (mScanForward) {
      for (uint64_t i = startPosn; // 从起始位置开始扫描
           (i < waypoint_buffer.size()) && (buffer_front->DistanceSquared(target_waypoint) < target_point_dist_power);
           ++i) {
        target_waypoint = waypoint_buffer.at(i); // 更新目标航点
        index = i; // 更新索引
      }
    } else { // 后向扫描航点
      for (uint64_t i = startPosn; // 从起始位置开始扫描
           (buffer_front->DistanceSquared(target_waypoint) > target_point_dist_power);
           --i) {
        target_waypoint = waypoint_buffer.at(i); // 更新目标航点
        index = i; // 更新索引
      }
    }
  } else { // 如果起始位置超出缓冲区
    target_waypoint = waypoint_buffer.back(); // 目标航点为缓冲区最后一个航点
    index = waypoint_buffer.size() - 1; // 索引为缓冲区大小减一
  }
  return std::make_pair(target_waypoint, index); // 返回目标航点和索引
}

} // namespace traffic_manager
} // namespace carla
