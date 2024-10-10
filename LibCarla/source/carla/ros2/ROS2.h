// Copyright (c) 2023 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Buffer.h" // 引入 Carla 缓冲区头文件
#include "carla/BufferView.h" // 引入 Carla 缓冲区视图头文件
#include "carla/geom/Transform.h" // 引入 Carla 变换几何头文件
#include "carla/ros2/ROS2CallbackData.h" // 引入 ROS2 回调数据头文件
#include "carla/streaming/detail/Types.h" // 引入 Carla 流媒体类型头文件

#include <unordered_set> // 引入无序集合头文件
#include <unordered_map> // 引入无序映射头文件
#include <memory> // 引入智能指针头文件
#include <vector> // 引入向量头文件

// 前置声明
class AActor; // 声明 AActor 类
namespace carla {
  namespace geom {
    class GeoLocation; // 声明 GeoLocation 类
    class Vector3D; // 声明 Vector3D 类
  }
  namespace sensor {
    namespace data {
      struct DVSEvent; // 声明 DVSEvent 结构
      class LidarData; // 声明 LidarData 类
      class SemanticLidarData; // 声明 SemanticLidarData 类
      class RadarData; // 声明 RadarData 类
    }
  }
}

namespace carla {
namespace ros2 {

  class CarlaPublisher; // 声明 CarlaPublisher 类
  class CarlaTransformPublisher; // 声明 CarlaTransformPublisher 类
  class CarlaClockPublisher; // 声明 CarlaClockPublisher 类
  class CarlaEgoVehicleControlSubscriber; // 声明 CarlaEgoVehicleControlSubscriber 类

class ROS2
{
  public:

  // 删除单例的复制构造函数
  ROS2(const ROS2& obj) = delete; // 禁止拷贝构造函数
  static std::shared_ptr<ROS2> GetInstance() { // 获取单例实例
    if (!_instance) // 如果实例不存在
      _instance = std::shared_ptr<ROS2>(new ROS2); // 创建新的实例
    return _instance; // 返回实例
  }

  // 通用函数
  void Enable(bool enable); // 启用或禁用
  void Shutdown(); // 关闭功能
  bool IsEnabled() { return _enabled; } // 检查是否启用
  void SetFrame(uint64_t frame); // 设置帧数
  void SetTimestamp(double timestamp); // 设置时间戳

  // ros_name 管理函数
  void AddActorRosName(void *actor, std::string ros_name); // 添加 Actor 的 ROS 名称
  void AddActorParentRosName(void *actor, void* parent); // 添加 Actor 的父级 ROS 名称
  void RemoveActorRosName(void *actor); // 移除 Actor 的 ROS 名称
  void UpdateActorRosName(void *actor, std::string ros_name); // 更新 Actor 的 ROS 名称
  std::string GetActorRosName(void *actor); // 获取 Actor 的 ROS 名称
  std::string GetActorParentRosName(void *actor); // 获取 Actor 的父级 ROS 名称

  // 回调函数
  void AddActorCallback(void* actor, std::string ros_name, ActorCallback callback); // 添加 Actor 回调函数
  void RemoveActorCallback(void* actor); // 移除 Actor 回调函数

  // 允许流发布
  void EnableStream(carla::streaming::detail::stream_id_type id) { _publish_stream.insert(id); } // 启用流发布
  bool IsStreamEnabled(carla::streaming::detail::stream_id_type id) { return _publish_stream.count(id) > 0; } // 检查流是否启用
  void ResetStreams() { _publish_stream.clear(); } // 重置流

  // 接收要发布的数据
  void ProcessDataFromCamera(
      uint64_t sensor_type,
      carla::streaming::detail::stream_id_type stream_id,
      const carla::geom::Transform sensor_transform,
      int W, int H, float Fov,
      const carla::SharedBufferView buffer,
      void *actor = nullptr); // 处理来自相机的数据
  void ProcessDataFromGNSS(
      uint64_t sensor_type,
      carla::streaming::detail::stream_id_type stream_id,
      const carla::geom::Transform sensor_transform,
      const carla::geom::GeoLocation &data,
      void *actor = nullptr); // 处理来自 GNSS 的数据
  void ProcessDataFromIMU(
      uint64_t sensor_type,
      carla::streaming::detail::stream_id_type stream_id,
      const carla::geom::Transform sensor_transform,
      carla::geom::Vector3D accelerometer,
      carla::geom::Vector3D gyroscope,
      float compass,
      void *actor = nullptr); // 处理来自 IMU 的数据
  void ProcessDataFromDVS(
      uint64_t sensor_type,
      carla::streaming::detail::stream_id_type stream_id,
      const carla::geom::Transform sensor_transform,
      const carla::SharedBufferView buffer,
      int W, int H, float Fov,
      void *actor = nullptr); // 处理来自 DVS 的数据
  void ProcessDataFromLidar(
      uint64_t sensor_type,
      carla::streaming::detail::stream_id_type stream_id,
      const carla::geom::Transform sensor_transform,
      carla::sensor::data::LidarData &data,
      void *actor = nullptr); // 处理来自激光雷达的数据
  void ProcessDataFromSemanticLidar(
      uint64_t sensor_type,
      carla::streaming::detail::stream_id_type stream_id,
      const carla::geom::Transform sensor_transform,
      carla::sensor::data::SemanticLidarData &data,
      void *actor = nullptr); // 处理来自语义激光雷达的数据
  void ProcessDataFromRadar(
      uint64_t sensor_type,
      carla::streaming::detail::stream_id_type stream_id,
      const carla::geom::Transform sensor_transform,
      const carla::sensor::data::RadarData &data,
      void *actor = nullptr); // 处理来自雷达的数据
  void ProcessDataFromObstacleDetection(
      uint64_t sensor_type,
      carla::streaming::detail::stream_id_type stream_id,
      const carla::geom::Transform sensor_transform,
      AActor *first_actor,
      AActor *second_actor,
      float distance,
      void *actor = nullptr); // 处理来自障碍物检测的数据
  void ProcessDataFromCollisionSensor( // 处理来自碰撞传感器的数据
      uint64_t sensor_type, // 传感器类型
      carla::streaming::detail::stream_id_type stream_id, // 流 ID
      const carla::geom::Transform sensor_transform, // 传感器变换
      uint32_t other_actor, // 其他 Actor 的 ID
      carla::geom::Vector3D impulse, // 冲击力
      void* actor); // 当前 Actor

 private: // 私有成员
 std::pair<std::shared_ptr<CarlaPublisher>, std::shared_ptr<CarlaTransformPublisher>> GetOrCreateSensor(int type, carla::streaming::detail::stream_id_type id, void* actor); // 获取或创建传感器

// 单例
ROS2() {}; // 构造函数

static std::shared_ptr<ROS2> _instance; // 单例实例

bool _enabled { false }; // 启用状态
uint64_t _frame { 0 }; // 帧数
int32_t _seconds { 0 }; // 秒数
uint32_t _nanoseconds { 0 }; // 纳秒数
std::unordered_map<void *, std::string> _actor_ros_name; // Actor 的 ROS 名称映射
std::unordered_map<void *, std::vector<void*> > _actor_parent_ros_name; // Actor 的父级 ROS 名称映射
std::shared_ptr<CarlaEgoVehicleControlSubscriber> _controller; // 控制器实例
std::shared_ptr<CarlaClockPublisher> _clock_publisher; // 时钟发布者实例
std::unordered_map<void *, std::shared_ptr<CarlaPublisher>> _publishers; // 发布者映射
std::unordered_map<void *, std::shared_ptr<CarlaTransformPublisher>> _transforms; // 变换发布者映射
std::unordered_set<carla::streaming::detail::stream_id_type> _publish_stream; // 发布流集合
std::unordered_map<void *, ActorCallback> _actor_callbacks; // Actor 回调映射
};

} // namespace ros2
} // namespace carla
