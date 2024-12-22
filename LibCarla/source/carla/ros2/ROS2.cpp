// Copyright (c) 2023 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/Logging.h" // 引入日志模块
#include "carla/ros2/ROS2.h" // 引入ROS2模块
#include "carla/geom/GeoLocation.h" // 引入地理位置模块
#include "carla/geom/Vector3D.h" // 引入三维向量模块
#include "carla/sensor/data/DVSEvent.h" // 引入DVS事件数据模块
#include "carla/sensor/data/LidarData.h" // 引入激光雷达数据模块
#include "carla/sensor/data/SemanticLidarData.h" // 引入语义激光雷达数据模块
#include "carla/sensor/data/RadarData.h" // 引入雷达数据模块
#include "carla/sensor/data/Image.h" // 引入图像数据模块
#include "carla/sensor/s11n/ImageSerializer.h" // 引入图像序列化模块
#include "carla/sensor/s11n/SensorHeaderSerializer.h" // 引入传感器头序列化模块

#include "publishers/CarlaPublisher.h" // 引入Carla发布者模块
#include "publishers/CarlaClockPublisher.h" // 引入时钟发布者模块
#include "publishers/CarlaRGBCameraPublisher.h" // 引入RGB相机发布者模块
#include "publishers/CarlaDepthCameraPublisher.h" // 引入深度相机发布者模块
#include "publishers/CarlaNormalsCameraPublisher.h" // 引入法线相机发布者模块
#include "publishers/CarlaOpticalFlowCameraPublisher.h" // 引入光流相机发布者模块
#include "publishers/CarlaSSCameraPublisher.h" // 引入超分辨率相机发布者模块
#include "publishers/CarlaISCameraPublisher.h" // 引入实例分割相机发布者模块
#include "publishers/CarlaDVSCameraPublisher.h" // 引入DVS相机发布者模块
#include "publishers/CarlaLidarPublisher.h" // 引入激光雷达发布者模块
#include "publishers/CarlaSemanticLidarPublisher.h" // 引入语义激光雷达发布者模块
#include "publishers/CarlaRadarPublisher.h" // 引入雷达发布者模块
#include "publishers/CarlaIMUPublisher.h" // 引入IMU发布者模块
#include "publishers/CarlaGNSSPublisher.h" // 引入GNSS发布者模块
#include "publishers/CarlaMapSensorPublisher.h" // 引入地图传感器发布者模块
#include "publishers/CarlaSpeedometerSensor.h" // 引入速度计传感器模块
#include "publishers/CarlaTransformPublisher.h" // 引入变换发布者模块
#include "publishers/CarlaCollisionPublisher.h" // 引入碰撞发布者模块
#include "publishers/CarlaLineInvasionPublisher.h" // 引入线路侵入发布者模块

#include "subscribers/CarlaSubscriber.h" // 引入Carla订阅者模块
#include "subscribers/CarlaEgoVehicleControlSubscriber.h" // 引入自我车辆控制订阅者模块

#include <vector> // 引入向量库

namespace carla {
namespace ros2 {

// 静态字段
std::shared_ptr<ROS2> ROS2::_instance; // ROS2实例的共享指针

// 传感器列表（应该等同于SensorsRegistry列表）
enum ESensors { // 定义传感器枚举
  CollisionSensor, // 碰撞传感器
  DepthCamera, // 深度相机
  NormalsCamera, // 法线相机
  DVSCamera, // DVS相机
  GnssSensor, // GNSS传感器
  InertialMeasurementUnit, // 惯性测量单元
  LaneInvasionSensor, // 车道入侵传感器
  ObstacleDetectionSensor, // 障碍物检测传感器
  OpticalFlowCamera, // 光流相机
  Radar, // 雷达
  RayCastSemanticLidar, // 光线投射语义激光雷达
  RayCastLidar, // 光线投射激光雷达
  RssSensor, // RSS传感器
  SceneCaptureCamera, // 场景捕获相机
  SemanticSegmentationCamera, // 语义分割相机
  InstanceSegmentationCamera, // 实例分割相机
  WorldObserver, // 世界观察者
  CameraGBufferUint8, // 相机G缓冲区（8位无符号整数）
  CameraGBufferFloat // 相机G缓冲区（浮点数）
};
// 启动或禁用ROS2系统
// enabled是一个布尔值，表示是否启用ROS2
// 设置_enabled 成员变量为传入的enable值
// 记录启用状态到日志中
// 创建一个名为clock的时钟发布者，并初始化它
void ROS2::Enable(bool enable) { // 启用或禁用ROS2
  _enabled = enable; // 设置启用状态
  log_info("ROS2 enabled: ", _enabled); // 记录启用状态
  _clock_publisher = std::make_shared<CarlaClockPublisher>("clock", ""); // 创建时钟发布者
  _clock_publisher->Init(); // 初始化时钟发布者
}
// 设置当前帧，调用相应的回调函数
// Frame是一个无符号64位整数，表示新的帧号
// 更新_frame成员变量为传入的frame值
// 如果控制器存在且仍然存活，并且有新消息
// 获取当前操作者
// 查找对应的回调函数
// 获取控制信息并调用回调函数
void ROS2::SetFrame(uint64_t frame) { // 设置帧
  _frame = frame; // 更新帧
   //log_info("ROS2 new frame: ", _frame); // 记录新帧信息
   if (_controller) { // 如果控制器存在
    void* actor = _controller->GetVehicle(); // 获取操作者
    if (_controller->IsAlive()) { // 如果控制器仍然存活
      if (_controller->HasNewMessage()) { // 如果有新消息
        auto it = _actor_callbacks.find(actor); // 查找操作者回调
        if (it != _actor_callbacks.end()) { // 如果找到操作者回调
          VehicleControl control = _controller->GetMessage(); // 获取控制消息
          it->second(actor, control); // 调用回调函数
        }
      }
    } else { // 如果控制器不再存活
      RemoveActorCallback(actor); // 移除操作者回调
    }
   }
}

void ROS2::SetTimestamp(double timestamp) { // 设置时间戳
  double integral; // 整数部分
  const double fractional = modf(timestamp, &integral); // 分数部分
  const double multiplier = 1000000000.0; // 毫微秒乘数
  _seconds = static_cast<int32_t>(integral); // 更新秒数
  _nanoseconds = static_cast<uint32_t>(fractional * multiplier); // 更新纳秒数
  _clock_publisher->SetData(_seconds, _nanoseconds); // 设置时钟数据
  _clock_publisher->Publish(); // 发布时钟数据
   //log_info("ROS2 new timestamp: ", _timestamp); // 记录新时间戳
}

void ROS2::AddActorRosName(void *actor, std::string ros_name) { // 添加操作者的ROS名称
  _actor_ros_name.insert({actor, ros_name}); // 插入操作者和ROS名称
}

void ROS2::AddActorParentRosName(void *actor, void* parent) { // 添加操作者的父ROS名称
  auto it = _actor_parent_ros_name.find(actor); // 查找操作者
  if (it != _actor_parent_ros_name.end()) { // 如果找到
    it->second.push_back(parent); // 添加父名称
  } else { // 如果没找到
    _actor_parent_ros_name.insert({actor, {parent}}); // 插入操作者和父名称
  }
}

void ROS2::RemoveActorRosName(void *actor) { // 移除操作者的ROS名称
  _actor_ros_name.erase(actor); // 移除ROS名称
  _actor_parent_ros_name.erase(actor); // 移除父ROS名称

  _publishers.erase(actor); // 移除发布者
  _transforms.erase(actor); // 移除变换数据
}

void ROS2::UpdateActorRosName(void *actor, std::string ros_name) { // 更新操作者的ROS名称
  auto it = _actor_ros_name.find(actor); // 查找操作者
  if (it != _actor_ros_name.end()) { // 如果找到
    it->second = ros_name; // 更新ROS名称
  }
}

std::string ROS2::GetActorRosName(void *actor) { // 获取操作者的ROS名称
  auto it = _actor_ros_name.find(actor); // 查找操作者
  if (it != _actor_ros_name.end()) { // 如果找到
    return it->second; // 返回ROS名称
  } else { // 如果没找到
    return std::string(""); // 返回空字符串
  }
}
// 获取操作者父节点名称
std::string ROS2::GetActorParentRosName(void *actor) {
  auto it = _actor_parent_ros_name.find(actor);// 查找操作者的父节点ROS名称
  if (it != _actor_parent_ros_name.end())
  {
    const std::string current_actor_name = GetActorRosName(actor);// 获取当前操作者的ROS名称
    std::string parent_name;// 存储父节点名称
    for (auto parent_it = it->second.cbegin(); parent_it != it->second.cend(); ++parent_it)
    {
      const std::string name = GetActorRosName(*parent_it);// 获取父节点的ROS名称
      if (name == current_actor_name) // 如果是当前操作者，跳过
      {
        continue;
      }
      if (name.empty())// 如果名称为空，跳过
      {
        continue;
      }
      parent_name = name + '/' + parent_name;// 构建父节点名称
    }
    if (parent_name.back() == '/')// 如果最后一个字符是斜杠，去掉
      parent_name.pop_back();
    return parent_name;// 返回父节点名称
  }
  else
    return std::string("");// 如果没有找到，返回空字符串
}

void ROS2::AddActorCallback(void* actor, std::string ros_name, ActorCallback callback) {
  _actor_callbacks.insert({actor, std::move(callback)});// 添加操作者的回调函数

  _controller.reset();// 重置控制器
  _controller = std::make_shared<CarlaEgoVehicleControlSubscriber>(actor, ros_name.c_str());// 创建新的控制器
  _controller->Init(); // 初始化控制器
}

void ROS2::RemoveActorCallback(void* actor) {
  _controller.reset();// 重置控制器
  _actor_callbacks.erase(actor);// 移除操作者的回调
}

std::pair<std::shared_ptr<CarlaPublisher>, std::shared_ptr<CarlaTransformPublisher>> ROS2::GetOrCreateSensor(int type, carla::streaming::detail::stream_id_type id, void* actor) {
  auto it_publishers = _publishers.find(actor);// 查找操作者的发布者
  auto it_transforms = _transforms.find(actor);// 查找操作者的变换发布者
  std::shared_ptr<CarlaPublisher> publisher {};// 声明发布者
  std::shared_ptr<CarlaTransformPublisher> transform {};// 声明变换发布者
  if (it_publishers != _publishers.end()) {
    publisher = it_publishers->second;// 如果找到，获取发布者
    if (it_transforms != _transforms.end()) {
      transform = it_transforms->second;// 如果找到，获取变换发布者
    }
  } else {
    // 没找到传感器，创建一个给定类型
    const std::string string_id = std::to_string(id);// 将ID转换为字符串
    std::string ros_name = GetActorRosName(actor);// 获取操作者的ROS名称
    std::string parent_ros_name = GetActorParentRosName(actor);// 获取操作者的父节点ROS名称
    switch(type) {
      case ESensors::CollisionSensor: {// 碰撞传感器
        if (ros_name == "collision__") {
          ros_name.pop_back();// 去掉最后一个字符
          ros_name.pop_back();// 去掉倒数第二个字符
          ros_name += string_id;// 添加ID
          UpdateActorRosName(actor, ros_name);// 更新操作者的ROS名称
        }
        std::shared_ptr<CarlaCollisionPublisher> new_publisher = std::make_shared<CarlaCollisionPublisher>(ros_name.c_str(), parent_ros_name.c_str());// 创建新的碰撞发布者
        if (new_publisher->Init()) {// 初始化发布者
          _publishers.insert({actor, new_publisher});// 插入到发布者列表
          publisher = new_publisher; // 设置当前发布者
        }
        std::shared_ptr<CarlaTransformPublisher> new_transform = std::make_shared<CarlaTransformPublisher>(ros_name.c_str(), parent_ros_name.c_str());// 创建新的变换发布者
        if (new_transform->Init()) {// 初始化变换发布者
          _transforms.insert({actor, new_transform});// 插入到变换发布者列表
          transform = new_transform;// 设置当前变换发布者
        }
      } break;
      case ESensors::DepthCamera: {// 深度相机
        if (ros_name == "depth__") {
          ros_name.pop_back(); // 去掉最后一个字符
          ros_name.pop_back();// 去掉倒数第二个字符
          ros_name += string_id;// 添加ID
          UpdateActorRosName(actor, ros_name); // 更新操作者的ROS名称
        }
        std::shared_ptr<CarlaDepthCameraPublisher> new_publisher = std::make_shared<CarlaDepthCameraPublisher>(ros_name.c_str(),
parent_ros_name.c_str());// 创建新的深度相机发布者
        if (new_publisher->Init()) {// 初始化发布者
          _publishers.insert({actor, new_publisher});// 插入到发布者列表
          publisher = new_publisher;// 设置当前发布者
        }
        std::shared_ptr<CarlaTransformPublisher> new_transform = std::make_shared<CarlaTransformPublisher>(ros_name.c_str(), parent_ros_name.c_str());// 创建新的变换发布者
        if (new_transform->Init()) {// 初始化变换发布者
          _transforms.insert({actor, new_transform});// 插入到变换发布者列表
          transform = new_transform; // 设置当前变换发布者
        }
      } break;
      case ESensors::NormalsCamera: { // 法线相机
        if (ros_name == "normals__") {
          ros_name.pop_back();// 去掉最后一个字符
          ros_name.pop_back();// 去掉倒数第二个字符
          ros_name += string_id;// 添加ID
          UpdateActorRosName(actor, ros_name);// 更新操作者的ROS名称
        }
        std::shared_ptr<CarlaNormalsCameraPublisher> new_publisher = std::make_shared<CarlaNormalsCameraPublisher>(ros_name.c_str(), parent_ros_name.c_str());// 创建一个新的法线相机发布者
        if (new_publisher->Init()) {// 初始化发布者
          _publishers.insert({actor, new_publisher});// 将发布者插入到发布者集合中
          publisher = new_publisher;// 更新当前发布者
        }
        std::shared_ptr<CarlaTransformPublisher> new_transform = std::make_shared<CarlaTransformPublisher>(ros_name.c_str(), parent_ros_name.c_str()); // 创建一个新的变换发布者
        if (new_transform->Init()) {// 初始化变换发布者
          _transforms.insert({actor, new_transform});// 将变换发布者插入到变换集合中
          transform = new_transform;// 更新当前变换
        }
      } break;
      case ESensors::DVSCamera: {// DVS相机的处理
        if (ros_name == "dvs__") {// 检查ROS名称是否为"dvs__"
          ros_name.pop_back();// 去掉最后一个字符
          ros_name.pop_back();// 再去掉一个字符
          ros_name += string_id;// 添加字符串ID
          UpdateActorRosName(actor, ros_name);// 更新操作者的ROS名称
        }
        std::shared_ptr<CarlaDVSCameraPublisher> new_publisher = std::make_shared<CarlaDVSCameraPublisher>(ros_name.c_str(), parent_ros_name.c_str());// 创建新的DVS相机发布者
        if (new_publisher->Init()) {// 初始化DVS发布者
          _publishers.insert({actor, new_publisher});// 将DVS发布者插入到发布者集合中
          publisher = new_publisher;// 更新当前发布者
        }
        std::shared_ptr<CarlaTransformPublisher> new_transform = std::make_shared<CarlaTransformPublisher>(ros_name.c_str(), parent_ros_name.c_str());// 创建新的变换发布者
        if (new_transform->Init()) {// 初始化变换发布者
          _transforms.insert({actor, new_transform});// 将变换发布者插入到变换集合中
          transform = new_transform;// 更新当前变换
        }
      } break;
      case ESensors::GnssSensor: {// GNSS传感器的处理
        if (ros_name == "gnss__") {// 检查ROS名称是否为"gnss__"
          ros_name.pop_back();// 去掉最后一个字符
          ros_name.pop_back();// 再去掉一个字符
          ros_name += string_id;// 添加字符串ID
          UpdateActorRosName(actor, ros_name);// 更新操作者的ROS名称
        }
        std::shared_ptr<CarlaGNSSPublisher> new_publisher = std::make_shared<CarlaGNSSPublisher>(ros_name.c_str(), parent_ros_name.c_str()); // 创建新的GNSS发布者      
        if (new_publisher->Init()) {// 初始化GNSS发布者
          _publishers.insert({actor, new_publisher});// 将GNSS发布者插入到发布者集合中
          publisher = new_publisher;// 更新当前发布者
        }
        std::shared_ptr<CarlaTransformPublisher> new_transform = std::make_shared<CarlaTransformPublisher>(ros_name.c_str(), parent_ros_name.c_str());// 创建新的变换发布者
        if (new_transform->Init()) {// 初始化变换发布者
          _transforms.insert({actor, new_transform});// 将变换发布者插入到变换集合中
          transform = new_transform;// 更新当前变换
        }
      } break;
      case ESensors::InertialMeasurementUnit: {// 惯性测量单元的处理
        if (ros_name == "imu__") {// 检查ROS名称是否为"imu__"
          ros_name.pop_back();// 去掉最后一个字符
          ros_name.pop_back();// 再去掉一个字符
          ros_name += string_id;// 添加字符串ID
          UpdateActorRosName(actor, ros_name); // 更新操作者的ROS名称
        }
        std::shared_ptr<CarlaIMUPublisher> new_publisher = std::make_shared<CarlaIMUPublisher>(ros_name.c_str(), parent_ros_name.c_str());// 创建新的IMU发布者
        if (new_publisher->Init()) {// 初始化IMU发布者
          _publishers.insert({actor, new_publisher});// 将IMU发布者插入到发布者集合中
          publisher = new_publisher;// 更新当前发布者
        }
        std::shared_ptr<CarlaTransformPublisher> new_transform = std::make_shared<CarlaTransformPublisher>(ros_name.c_str(), parent_ros_name.c_str());// 创建新的变换发布者
        if (new_transform->Init()) {// 初始化变换发布者
          _transforms.insert({actor, new_transform});// 将变换发布者插入到变换集合中
          transform = new_transform;// 更新当前变换
        }
      } break;
      case ESensors::LaneInvasionSensor: {// 车道侵入传感器的处理
        if (ros_name == "lane_invasion__") {// 检查ROS名称是否为"lane_invasion__"
          ros_name.pop_back();// 去掉最后一个字符
          ros_name.pop_back();// 再去掉一个字符
          ros_name += string_id;// 添加字符串ID
          UpdateActorRosName(actor, ros_name);// 更新操作者的ROS名称
        }
        std::shared_ptr<CarlaLineInvasionPublisher> new_publisher = std::make_shared<CarlaLineInvasionPublisher>(ros_name.c_str(), parent_ros_name.c_str());// 创建新的压线发布者
        if (new_publisher->Init()) {// 初始化压线发布者
          _publishers.insert({actor, new_publisher});// 将压线发布者插入到发布者集合中
          publisher = new_publisher;// 更新当前发布者
        }
        std::shared_ptr<CarlaTransformPublisher> new_transform = std::make_shared<CarlaTransformPublisher>(ros_name.c_str(), parent_ros_name.c_str());// 创建新的变换发布者
        if (new_transform->Init()) {// 初始化变换发布者
          _transforms.insert({actor, new_transform});// 将变换发布者插入到变换集合中
          transform = new_transform;// 更新当前变换
        }
      } break;
      case ESensors::ObstacleDetectionSensor: {// 遇到障碍物检测传感器
        std::cerr << "Obstacle detection sensor does not have an available publisher" << std::endl;// 遇到障碍物检测传感器没有可用的发布者
      } break;
      case ESensors::OpticalFlowCamera: {// 光流相机
        if (ros_name == "optical_flow__") {// 如果ros_name是光流相机
          ros_name.pop_back(); // 移除最后一个字符
          ros_name.pop_back();// 再次移除最后一个字符
          ros_name += string_id;// 添加字符串标识
          UpdateActorRosName(actor, ros_name);// 更新操作者的ros名称
        }
        std::shared_ptr<CarlaOpticalFlowCameraPublisher> new_publisher = std::make_shared<CarlaOpticalFlowCameraPublisher>(ros_name.c_str(), parent_ros_name.c_str());// 创建新的光流相机发布者
        if (new_publisher->Init()) {// 初始化发布者
          _publishers.insert({actor, new_publisher});// 将新发布者插入到发布者集合中
          publisher = new_publisher;// 更新当前发布者
        }
        std::shared_ptr<CarlaTransformPublisher> new_transform = std::make_shared<CarlaTransformPublisher>(ros_name.c_str(), parent_ros_name.c_str());// 创建新的变换发布者
        if (new_transform->Init()) {// 初始化变换发布者
          _transforms.insert({actor, new_transform});// 将新变换发布者插入到变换集合中
          transform = new_transform;// 更新当前变换发布者
        }
      } break;
      case ESensors::Radar: {// 雷达传感器
        if (ros_name == "radar__") {// 如果ros_name是雷达
          ros_name.pop_back(); // 移除最后一个字符
          ros_name.pop_back();// 再次移除最后一个字符
          ros_name += string_id;// 添加字符串标识
          UpdateActorRosName(actor, ros_name);// 更新操作者的ros名称
        }
        std::shared_ptr<CarlaRadarPublisher> new_publisher = std::make_shared<CarlaRadarPublisher>(ros_name.c_str(), parent_ros_name.c_str());// 创建新的雷达发布者
        if (new_publisher->Init()) {// 初始化发布者
          _publishers.insert({actor, new_publisher});// 将新发布者插入到发布者集合中
          publisher = new_publisher;// 更新当前发布者
        }
        std::shared_ptr<CarlaTransformPublisher> new_transform = std::make_shared<CarlaTransformPublisher>(ros_name.c_str(), parent_ros_name.c_str());// 创建新的变换发布者
        if (new_transform->Init()) {// 初始化变换发布者
          _transforms.insert({actor, new_transform});// 将新变换发布者插入到变换集合中
          transform = new_transform;// 更新当前变换发布者
        }
      } break;
      case ESensors::RayCastSemanticLidar: {// 射线投射语义激光雷达
        if (ros_name == "ray_cast_semantic__") { // 如果ros_name是射线投射语义
          ros_name.pop_back();// 移除最后一个字符
          ros_name.pop_back();// 再次移除最后一个字符
          ros_name += string_id;// 添加字符串标识
          UpdateActorRosName(actor, ros_name);// 更新操作者的ros名称
        }
        std::shared_ptr<CarlaSemanticLidarPublisher> new_publisher = std::make_shared<CarlaSemanticLidarPublisher>(ros_name.c_str(), parent_ros_name.c_str());// 创建新的语义激光雷达发布者
        if (new_publisher->Init()) {// 初始化发布者
          _publishers.insert({actor, new_publisher});// 将新发布者插入到发布者集合中
          publisher = new_publisher;// 更新当前发布者
        }
        std::shared_ptr<CarlaTransformPublisher> new_transform = std::make_shared<CarlaTransformPublisher>(ros_name.c_str(), parent_ros_name.c_str());// 创建新的变换发布者
        if (new_transform->Init()) {// 初始化变换发布者
          _transforms.insert({actor, new_transform});// 将新变换发布者插入到变换集合中
          transform = new_transform;// 更新当前变换发布者
        }
      } break;
      case ESensors::RayCastLidar: {// 射线投射激光雷达
        if (ros_name == "ray_cast__") {// 如果ros_name是射线投射
          ros_name.pop_back();// 移除最后一个字符
          ros_name.pop_back();// 再次移除最后一个字符
          ros_name += string_id;// 添加字符串标识
          UpdateActorRosName(actor, ros_name);// 更新操作者的ros名称
        }
        std::shared_ptr<CarlaLidarPublisher> new_publisher = std::make_shared<CarlaLidarPublisher>(ros_name.c_str(), parent_ros_name.c_str());// 创建新的激光雷达发布者
        if (new_publisher->Init()) {// 初始化发布者
          _publishers.insert({actor, new_publisher});// 将新发布者插入到发布者集合中
          publisher = new_publisher;// 更新当前发布者
        }
        std::shared_ptr<CarlaTransformPublisher> new_transform = std::make_shared<CarlaTransformPublisher>(ros_name.c_str(), parent_ros_name.c_str());// 创建新的变换发布者
        if (new_transform->Init()) {// 初始化变换发布者
          _transforms.insert({actor, new_transform});// 将新变换发布者插入到变换集合中
          transform = new_transform;// 更新当前变换发布者
        }
      } break;
      case ESensors::RssSensor: {// RSS传感器
        std::cerr << "RSS sensor does not have an available publisher" << std::endl;// RSS传感器没有可用的发布者
      } break;
      case ESensors::SceneCaptureCamera: {// 场景捕捉相机
        if (ros_name == "rgb__") {// 如果ros_name是RGB
          ros_name.pop_back();// 移除最后一个字符
          ros_name.pop_back();// 再次移除最后一个字符
          ros_name += string_id;// 添加字符串标识
          UpdateActorRosName(actor, ros_name);// 更新操作者的ros名称
        }
        std::shared_ptr<CarlaRGBCameraPublisher> new_publisher = std::make_shared<CarlaRGBCameraPublisher>(ros_name.c_str(), parent_ros_name.c_str());// 创建新的RGB相机发布者
        if (new_publisher->Init()) { // 初始化发布者
          _publishers.insert({actor, new_publisher});// 将新发布者插入到发布者集合中
          publisher = new_publisher; // 更新当前发布者
        }
        std::shared_ptr<CarlaTransformPublisher> new_transform = std::make_shared<CarlaTransformPublisher>(ros_name.c_str(), parent_ros_name.c_str());// 创建新的变换发布者
        if (new_transform->Init()) { // 初始化变换发布者
          _transforms.insert({actor, new_transform});// 将新变换发布者插入到变换集合中
          transform = new_transform; // 更新当前变换发布者
        }
      } break;
      case ESensors::SemanticSegmentationCamera: {// 如果传感器是语义分割相机
        if (ros_name == "semantic_segmentation__") {// 检查ROS名称是否为语义分割相机
          ros_name.pop_back();// 移除最后一个字符
          ros_name.pop_back();// 移除倒数第二个字符
          ros_name += string_id;// 添加字符串ID
          UpdateActorRosName(actor, ros_name);// 更新操作者的ROS名称
        }
        std::shared_ptr<CarlaSSCameraPublisher> new_publisher = std::make_shared<CarlaSSCameraPublisher>(ros_name.c_str(), parent_ros_name.c_str()); // 创建新的语义分割相机发布者
        if (new_publisher->Init()) {// 初始化发布者
          _publishers.insert({actor, new_publisher});// 将新发布者插入到发布者集合中
          publisher = new_publisher; // 更新当前发布者
        }
        std::shared_ptr<CarlaTransformPublisher> new_transform = std::make_shared<CarlaTransformPublisher>(ros_name.c_str(), parent_ros_name.c_str());// 创建新的变换发布者
        if (new_transform->Init()) {// 初始化变换发布者
          _transforms.insert({actor, new_transform});// 将新变换发布者插入到变换集合中
          transform = new_transform;// 更新当前变换发布者
        }
      } break;
      case ESensors::InstanceSegmentationCamera: {// 如果传感器是实例分割相机
        if (ros_name == "instance_segmentation__") {// 检查ROS名称是否为实例分割相机
          ros_name.pop_back(); // 移除最后一个字符
          ros_name.pop_back();// 移除倒数第二个字符
          ros_name += string_id;// 添加字符串ID
          UpdateActorRosName(actor, ros_name); // 更新操作者的ROS名称
        }
        std::shared_ptr<CarlaISCameraPublisher> new_publisher = std::make_shared<CarlaISCameraPublisher>(ros_name.c_str(), parent_ros_name.c_str());// 创建新的实例分割相机发布者
        if (new_publisher->Init()) {// 初始化发布者
          _publishers.insert({actor, new_publisher});// 将新发布者插入到发布者集合中
          publisher = new_publisher;// 更新当前发布者
        }
        std::shared_ptr<CarlaTransformPublisher> new_transform = std::make_shared<CarlaTransformPublisher>(ros_name.c_str(), parent_ros_name.c_str());// 创建新的变换发布者
        if (new_transform->Init()) {// 初始化变换发布者
          _transforms.insert({actor, new_transform});// 将新变换发布者插入到变换集合中
          transform = new_transform; // 更新当前变换发布者
        }
      } break;
      case ESensors::WorldObserver: {// 如果传感器是世界观察者
        std::cerr << "World obserser does not have an available publisher" << std::endl;// 输出错误信息：世界观察者没有可用的发布者
      } break;
      case ESensors::CameraGBufferUint8: {// 如果传感器是GBuffer uint8相机
        std::cerr << "Camera GBuffer uint8 does not have an available publisher" << std::endl;// 输出错误信息：GBuffer uint8相机没有可用的发布者
      } break;
      case ESensors::CameraGBufferFloat: {// 如果传感器是GBuffer float相机
        std::cerr << "Camera GBuffer float does not have an available publisher" << std::endl;// 输出错误信息：GBuffer float相机没有可用的发布者
      } break;
      default: {// 默认情况
        std::cerr << "Unknown sensor type" << std::endl;// 输出错误信息：未知的传感器类型
      }
    }
  }
  return { publisher, transform };// 返回当前发布者和变换发布者
}

void ROS2::ProcessDataFromCamera(
    uint64_t sensor_type,// 传感器类型
    carla::streaming::detail::stream_id_type stream_id,// 流ID
    const carla::geom::Transform sensor_transform,// 传感器变换
    int W, int H, float Fov, // 宽度、高度、视场角
    const carla::SharedBufferView buffer,// 数据缓冲区
    void *actor) { // 操作者

  switch (sensor_type) { // 根据传感器类型进行处理
    case ESensors::CollisionSensor:// 碰撞传感器
      log_info("Sensor Collision to ROS data: frame.", _frame, "sensor.", sensor_type, "stream.", stream_id, "buffer.", buffer->size());// 记录碰撞传感器数据
      break;
    case ESensors::DepthCamera:// 深度相机
      {
        log_info("Sensor DepthCamera to ROS data: frame.", _frame, "sensor.", sensor_type, "stream.", stream_id, "buffer.", buffer->size());// 记录深度相机数据
        auto sensors = GetOrCreateSensor(ESensors::DepthCamera, stream_id, actor);
        if (sensors.first) {// 如果存在第一个传感器
          std::shared_ptr<CarlaDepthCameraPublisher> publisher = std::dynamic_pointer_cast<CarlaDepthCameraPublisher>(sensors.first); // 转换为深度相机发布者
          const carla::sensor::s11n::ImageSerializer::ImageHeader *header =// 获取图像头信息
            reinterpret_cast<const carla::sensor::s11n::ImageSerializer::ImageHeader *>(buffer->data());
          if (!header) // 如果头信息为空
            return;// 返回
          if (!publisher->HasBeenInitialized())// 如果发布者未初始化
            publisher->InitInfoData(0, 0, H, W, Fov, true); // 初始化信息数据
          publisher->SetImageData(_seconds, _nanoseconds, header->height, header->width, (const uint8_t*) (buffer->data() + carla::sensor::s11n::ImageSerializer::header_offset));// 设置图像数据
          publisher->SetCameraInfoData(_seconds, _nanoseconds);// 设置相机信息数据
          publisher->Publish();// 发布数据
        }
        if (sensors.second) {// 如果存在第二个传感器
          std::shared_ptr<CarlaTransformPublisher> publisher = std::dynamic_pointer_cast<CarlaTransformPublisher>(sensors.second); // 转换为变换发布者
          publisher->SetData(_seconds, _nanoseconds, (const float*)&sensor_transform.location, (const float*)&sensor_transform.rotation);// 设置位置信息和旋转信息
          publisher->Publish();// 发布数据
        }
      }
      break;
    case ESensors::NormalsCamera: // 法线相机
      log_info("Sensor NormalsCamera to ROS data: frame.", _frame, "sensor.", sensor_type, "stream.", stream_id, "buffer.", buffer->size());// 记录法线相机数据
      {
        auto sensors = GetOrCreateSensor(ESensors::NormalsCamera, stream_id, actor); // 获取或创建传感器
        if (sensors.first) { // 如果存在第一个传感器
          std::shared_ptr<CarlaNormalsCameraPublisher> publisher = std::dynamic_pointer_cast<CarlaNormalsCameraPublisher>(sensors.first);// 转换为法线相机发布者
          const carla::sensor::s11n::ImageSerializer::ImageHeader *header =// 获取图像头信息
            reinterpret_cast<const carla::sensor::s11n::ImageSerializer::ImageHeader *>(buffer->data());
          if (!header)// 如果头信息为空
            return;// 返回
          if (!publisher->HasBeenInitialized())// 如果发布者未初始化
            publisher->InitInfoData(0, 0, H, W, Fov, true);// 初始化信息数据
          publisher->SetImageData(_seconds, _nanoseconds, header->height, header->width, (const uint8_t*) (buffer->data() + carla::sensor::s11n::ImageSerializer::header_offset));// 设置图像数据
          publisher->SetCameraInfoData(_seconds, _nanoseconds);// 设置相机信息数据
          publisher->Publish();// 发布数据
        }
        if (sensors.second) {// 如果存在第二个传感器
          std::shared_ptr<CarlaTransformPublisher> publisher = std::dynamic_pointer_cast<CarlaTransformPublisher>(sensors.second); // 转换为变换发布者
          publisher->SetData(_seconds, _nanoseconds, (const float*)&sensor_transform.location, (const float*)&sensor_transform.rotation);// 设置位置信息和旋转信息
          publisher->Publish(); // 发布数据
        }
      }
      break;
    case ESensors::LaneInvasionSensor:// 压线传感器
      log_info("Sensor LaneInvasionSensor to ROS data: frame.", _frame, "sensor.", sensor_type, "stream.", stream_id, "buffer.", buffer->size());// 记录压线传感器的数据到ROS，输出帧、传感器类型、流ID和缓冲区大小
      {
        auto sensors = GetOrCreateSensor(ESensors::LaneInvasionSensor, stream_id, actor);// 获取或创建压线传感器
        if (sensors.first) {// 如果第一个传感器存在
          std::shared_ptr<CarlaLineInvasionPublisher> publisher = std::dynamic_pointer_cast<CarlaLineInvasionPublisher>(sensors.first); // 转换为压线发布者
          publisher->SetData(_seconds, _nanoseconds, (const int32_t*) buffer->data());// 设置数据
          publisher->Publish();// 发布数据
        }
        if (sensors.second) {// 如果第二个传感器存在
          std::shared_ptr<CarlaTransformPublisher> publisher = std::dynamic_pointer_cast<CarlaTransformPublisher>(sensors.second);// 转换为变换发布者
          publisher->SetData(_seconds, _nanoseconds, (const float*)&sensor_transform.location, (const float*)&sensor_transform.rotation);// 设置位置信息和旋转信息
          publisher->Publish();// 发布数据
        }
      }
      break;
    case ESensors::OpticalFlowCamera:// 光流相机传感器
      log_info("Sensor OpticalFlowCamera to ROS data: frame.", _frame, "sensor.", sensor_type, "stream.", stream_id, "buffer.", buffer->size());// 记录光流相机的数据到ROS，输出帧、传感器类型、流ID和缓冲区大小
      {
        auto sensors = GetOrCreateSensor(ESensors::OpticalFlowCamera, stream_id, actor);// 获取或创建光流相机传感器
        if (sensors.first) { // 如果第一个传感器存在
          std::shared_ptr<CarlaOpticalFlowCameraPublisher> publisher = std::dynamic_pointer_cast<CarlaOpticalFlowCameraPublisher>(sensors.first);// 转换为光流相机发布者
          const carla::sensor::s11n::OpticalFlowImageSerializer::ImageHeader *header =// 获取图像头信息
            reinterpret_cast<const carla::sensor::s11n::OpticalFlowImageSerializer::ImageHeader *>(buffer->data());
          if (!header) // 如果没有图像头，返回
            return;
          if (!publisher->HasBeenInitialized())// 如果发布者尚未初始化
            publisher->InitInfoData(0, 0, H, W, Fov, true);// 初始化信息数据
          publisher->SetImageData(_seconds, _nanoseconds, header->height, header->width, (const float*) (buffer->data() + carla::sensor::s11n::OpticalFlowImageSerializer::header_offset));// 设置图像数据
          publisher->SetCameraInfoData(_seconds, _nanoseconds);
          publisher->Publish();// 发布数据
        }
        if (sensors.second) {// 如果第二个传感器存在
          std::shared_ptr<CarlaTransformPublisher> publisher = std::dynamic_pointer_cast<CarlaTransformPublisher>(sensors.second);// 转换为变换发布者
          publisher->SetData(_seconds, _nanoseconds, (const float*)&sensor_transform.location, (const float*)&sensor_transform.rotation);// 设置位置信息和旋转信息
          publisher->Publish();// 发布数据
        }
      }
      break;
    case ESensors::RssSensor:// RSS传感器
      log_info("Sensor RssSensor to ROS data: frame.", _frame, "sensor.", sensor_type, "stream.", stream_id, "buffer.", buffer->size()); // 记录RSS传感器的数据到ROS，输出帧、传感器类型、流ID和缓冲区大小
      break;
    case ESensors::SceneCaptureCamera:// 场景捕捉相机传感器
    {
      log_info("Sensor SceneCaptureCamera to ROS data: frame.", _frame, "sensor.", sensor_type, "stream.", stream_id, "buffer.", buffer->size());// 记录场景捕捉相机的数据到ROS，输出帧、传感器类型、流ID和缓冲区大小
      {
        auto sensors = GetOrCreateSensor(ESensors::SceneCaptureCamera, stream_id, actor);// 获取或创建场景捕捉相机传感器
        if (sensors.first) {// 如果第一个传感器存在
          std::shared_ptr<CarlaRGBCameraPublisher> publisher = std::dynamic_pointer_cast<CarlaRGBCameraPublisher>(sensors.first);// 设置图像数据
          const carla::sensor::s11n::ImageSerializer::ImageHeader *header =// 获取图像头信息
            reinterpret_cast<const carla::sensor::s11n::ImageSerializer::ImageHeader *>(buffer->data());
          if (!header)// 如果没有图像头，返回
            return;
          if (!publisher->HasBeenInitialized())// 如果发布者尚未初始化
            publisher->InitInfoData(0, 0, H, W, Fov, true);// 初始化信息数据
          publisher->SetImageData(_seconds, _nanoseconds, header->height, header->width, (const uint8_t*) (buffer->data() + carla::sensor::s11n::ImageSerializer::header_offset));// 设置图像数据
          publisher->SetCameraInfoData(_seconds, _nanoseconds);
          publisher->Publish();// 发布数据
        }
        if (sensors.second) {// 如果第二个传感器存在
          std::shared_ptr<CarlaTransformPublisher> publisher = std::dynamic_pointer_cast<CarlaTransformPublisher>(sensors.second);// 转换为变换发布者
          publisher->SetData(_seconds, _nanoseconds, (const float*)&sensor_transform.location, (const float*)&sensor_transform.rotation); // 设置位置信息和旋转信息
          publisher->Publish();// 发布数据
        }
      }
      break;
    }
    case ESensors::SemanticSegmentationCamera:// 语义分割相机
      log_info("Sensor SemanticSegmentationCamera to ROS data: frame.", _frame, "sensor.", sensor_type, "stream.", stream_id, "buffer.", buffer->size());// 记录信息：语义分割相机到ROS数据
      {
        auto sensors = GetOrCreateSensor(ESensors::SemanticSegmentationCamera, stream_id, actor);// 获取或创建传感器
        if (sensors.first) {// 如果第一个传感器存在
          std::shared_ptr<CarlaSSCameraPublisher> publisher = std::dynamic_pointer_cast<CarlaSSCameraPublisher>(sensors.first);// 转换为语义分割相机发布者
          const carla::sensor::s11n::ImageSerializer::ImageHeader *header =// 获取图像头信息
            reinterpret_cast<const carla::sensor::s11n::ImageSerializer::ImageHeader *>(buffer->data());// 从缓冲区中获取数据
          if (!header) // 如果图像头不存在
            return;// 返回
          if (!publisher->HasBeenInitialized())// 如果发布者尚未初始化
            publisher->InitInfoData(0, 0, H, W, Fov, true);// 初始化信息数据
          publisher->SetImageData(_seconds, _nanoseconds, header->height, header->width, (const uint8_t*) (buffer->data() + carla::sensor::s11n::ImageSerializer::header_offset));// 设置图像数据
          publisher->SetCameraInfoData(_seconds, _nanoseconds); // 设置相机信息数据
          publisher->Publish();// 发布数据
        }
        if (sensors.second) {// 如果第二个传感器存在
          std::shared_ptr<CarlaTransformPublisher> publisher = std::dynamic_pointer_cast<CarlaTransformPublisher>(sensors.second);// 转换为变换发布者
          publisher->SetData(_seconds, _nanoseconds, (const float*)&sensor_transform.location, (const float*)&sensor_transform.rotation);// 设置位置信息和旋转信息
          publisher->Publish();// 发布数据
        }
      }
      break;// 结束该case
    case ESensors::InstanceSegmentationCamera:// 实例分割相机
      log_info("Sensor InstanceSegmentationCamera to ROS data: frame.", _frame, "sensor.", sensor_type, "stream.", stream_id, "buffer.", buffer->size());// 记录信息：实例分割相机到ROS数据
      {
        auto sensors = GetOrCreateSensor(ESensors::InstanceSegmentationCamera, stream_id, actor);// 获取或创建传感器
        if (sensors.first) { // 如果第一个传感器存在
          std::shared_ptr<CarlaISCameraPublisher> publisher = std::dynamic_pointer_cast<CarlaISCameraPublisher>(sensors.first);// 转换为实例分割相机发布者
          const carla::sensor::s11n::ImageSerializer::ImageHeader *header =// 获取图像头信息
            reinterpret_cast<const carla::sensor::s11n::ImageSerializer::ImageHeader *>(buffer->data());// 从缓冲区中获取数据
          if (!header)// 如果图像头不存在
            return;// 返回
          if (!publisher->HasBeenInitialized())// 如果发布者尚未初始化
            publisher->InitInfoData(0, 0, H, W, Fov, true);// 初始化信息数据
          publisher->SetImageData(_seconds, _nanoseconds, header->height, header->width, (const uint8_t*) (buffer->data() + carla::sensor::s11n::ImageSerializer::header_offset));// 设置图像数据
          publisher->SetCameraInfoData(_seconds, _nanoseconds);// 设置相机信息数据
          publisher->Publish();// 发布数据
        }
        if (sensors.second) { // 如果第二个传感器存在
          std::shared_ptr<CarlaTransformPublisher> publisher = std::dynamic_pointer_cast<CarlaTransformPublisher>(sensors.second);// 转换为变换发布者
          publisher->SetData(_seconds, _nanoseconds, (const float*)&sensor_transform.location, (const float*)&sensor_transform.rotation);// 设置位置信息和旋转信息
          publisher->Publish();// 发布数据
        }
      }
      break;// 结束该case
    case ESensors::WorldObserver:// 世界观察者
      log_info("Sensor WorldObserver to ROS data: frame.", _frame, "sensor.", sensor_type, "stream.", stream_id, "buffer.", buffer->size());// 记录信息：世界观察者到ROS数据
      break;// 结束该case
    case ESensors::CameraGBufferUint8:// 相机G缓冲区（无符号8位）
      log_info("Sensor CameraGBufferUint8 to ROS data: frame.", _frame, "sensor.", sensor_type, "stream.", stream_id, "buffer.", buffer->size()); // 记录信息：相机G缓冲区（无符号8位）到ROS数据
      break;// 结束该case
    case ESensors::CameraGBufferFloat:// 相机G缓冲区（浮点型）
      log_info("Sensor CameraGBufferFloat to ROS data: frame.", _frame, "sensor.", sensor_type, "stream.", stream_id, "buffer.", buffer->size());// 记录信息：相机G缓冲区（浮点型）到ROS数据
      break;// 结束该case
    default:// 默认情况
      log_info("Sensor to ROS data: frame.", _frame, "sensor.", sensor_type, "stream.", stream_id, "buffer.", buffer->size());// 记录信息：传感器到ROS数据
  }
}

void ROS2::ProcessDataFromGNSS(
    uint64_t sensor_type,// 传感器类型
    carla::streaming::detail::stream_id_type stream_id,// 数据流ID
    const carla::geom::Transform sensor_transform,// 传感器变换
    const carla::geom::GeoLocation &data, // 地理位置数据
    void *actor) {// 操作者
  log_info("Sensor GnssSensor to ROS data: frame.", _frame, "sensor.", sensor_type, "stream.", stream_id, "geo.", data.latitude, data.longitude, data.altitude);// 记录GNSS传感器数据
  auto sensors = GetOrCreateSensor(ESensors::GnssSensor, stream_id, actor);// 获取或创建传感器
  if (sensors.first) { // 如果存在第一个传感器
    std::shared_ptr<CarlaGNSSPublisher> publisher = std::dynamic_pointer_cast<CarlaGNSSPublisher>(sensors.first); // 将传感器转换为GNSS发布者
    publisher->SetData(_seconds, _nanoseconds, reinterpret_cast<const double*>(&data)); // 设置数据
    publisher->Publish(); // 发布数据
  }
  if (sensors.second) { // 如果存在第二个传感器
    std::shared_ptr<CarlaTransformPublisher> publisher = std::dynamic_pointer_cast<CarlaTransformPublisher>(sensors.second);// 将传感器转换为变换发布者
    publisher->SetData(_seconds, _nanoseconds, (const float*)&sensor_transform.location, (const float*)&sensor_transform.rotation);// 设置变换数据
    publisher->Publish();// 发布变换数据
  }
}

void ROS2::ProcessDataFromIMU(
    uint64_t sensor_type,// 传感器类型
    carla::streaming::detail::stream_id_type stream_id,// 数据流ID
    const carla::geom::Transform sensor_transform,// 传感器变换
    carla::geom::Vector3D accelerometer, // 加速度计数据
    carla::geom::Vector3D gyroscope,// 陀螺仪数据
    float compass, // 指南针数据
    void *actor) { // 操作者
  log_info("Sensor InertialMeasurementUnit to ROS data: frame.", _frame, "sensor.", sensor_type, "stream.", stream_id, "imu.", accelerometer.x, gyroscope.x, compass);// 记录IMU传感器数据
  auto sensors = GetOrCreateSensor(ESensors::InertialMeasurementUnit, stream_id, actor);// 获取或创建传感器
  if (sensors.first) {// 如果存在第一个传感器
    std::shared_ptr<CarlaIMUPublisher> publisher = std::dynamic_pointer_cast<CarlaIMUPublisher>(sensors.first);// 将传感器转换为IMU发布者
    publisher->SetData(_seconds, _nanoseconds, reinterpret_cast<float*>(&accelerometer), reinterpret_cast<float*>(&gyroscope), compass);// 设置数据
    publisher->Publish(); // 发布数据
  }
  if (sensors.second) {// 如果存在第二个传感器
    std::shared_ptr<CarlaTransformPublisher> publisher = std::dynamic_pointer_cast<CarlaTransformPublisher>(sensors.second);// 将传感器转换为变换发布者
    publisher->SetData(_seconds, _nanoseconds, (const float*)&sensor_transform.location, (const float*)&sensor_transform.rotation);// 设置变换数据
    publisher->Publish();// 发布变换数据
  }
}

void ROS2::ProcessDataFromDVS(
    uint64_t sensor_type, // 传感器类型
    carla::streaming::detail::stream_id_type stream_id,// 数据流ID
    const carla::geom::Transform sensor_transform, // 传感器变换
    const carla::SharedBufferView buffer,// 缓冲区视图
    int W, int H, float Fov, // 宽度、高度、视场角
    void *actor) { // 操作者
  log_info("Sensor DVS to ROS data: frame.", _frame, "sensor.", sensor_type, "stream.", stream_id);// 记录DVS传感器数据
  auto sensors = GetOrCreateSensor(ESensors::DVSCamera, stream_id, actor);// 获取或创建传感器
  if (sensors.first) { // 如果存在第一个传感器
    std::shared_ptr<CarlaDVSCameraPublisher> publisher = std::dynamic_pointer_cast<CarlaDVSCameraPublisher>(sensors.first);// 将传感器转换为DVS相机发布者
    const carla::sensor::s11n::ImageSerializer::ImageHeader *header =// 图像头信息
      reinterpret_cast<const carla::sensor::s11n::ImageSerializer::ImageHeader *>(buffer->data());// 从缓冲区获取头部
    if (!header)// 如果头部为空
      return; // 退出
    if (!publisher->HasBeenInitialized())  // 如果发布者尚未初始化
      publisher->InitInfoData(0, 0, H, W, Fov, true);// 初始化信息数据
    size_t elements = (buffer->size() - carla::sensor::s11n::ImageSerializer::header_offset) / sizeof(carla::sensor::data::DVSEvent);// 计算元素数量
    publisher->SetImageData(_seconds, _nanoseconds, elements, header->height, header->width, (const uint8_t*) (buffer->data() + carla::sensor::s11n::ImageSerializer::header_offset));// 设置图像数据
    publisher->SetCameraInfoData(_seconds, _nanoseconds);// 设置相机信息数据
    publisher->SetPointCloudData(1, elements * sizeof(carla::sensor::data::DVSEvent), elements, (const uint8_t*) (buffer->data() + carla::sensor::s11n::ImageSerializer::header_offset));// 设置点云数据
    publisher->Publish();// 发布数据
  }
  if (sensors.second) { // 如果存在第二个传感器
    std::shared_ptr<CarlaTransformPublisher> publisher = std::dynamic_pointer_cast<CarlaTransformPublisher>(sensors.second);// 将传感器转换为变换发布者
    publisher->SetData(_seconds, _nanoseconds, (const float*)&sensor_transform.location, (const float*)&sensor_transform.rotation);// 设置变换数据
    publisher->Publish();// 发布变换数据
  }
}

void ROS2::ProcessDataFromLidar(
    uint64_t sensor_type,// 传感器类型
    carla::streaming::detail::stream_id_type stream_id,// 数据流ID
    const carla::geom::Transform sensor_transform, // 传感器变换
    carla::sensor::data::LidarData &data, // 激光雷达数据
    void *actor) {// 操作者
  log_info("Sensor Lidar to ROS data: frame.", _frame, "sensor.", sensor_type, "stream.", stream_id, "points.", data._points.size());// 记录激光雷达传感器数据
  auto sensors = GetOrCreateSensor(ESensors::RayCastLidar, stream_id, actor);// 获取或创建传感器
  if (sensors.first) {// 如果存在第一个传感器
    std::shared_ptr<CarlaLidarPublisher> publisher = std::dynamic_pointer_cast<CarlaLidarPublisher>(sensors.first);// 将传感器转换为激光雷达发布者
    size_t width = data._points.size();// 获取点云宽度
    size_t height = 1;// 设置高度为1
    publisher->SetData(_seconds, _nanoseconds, height, width, (float*)data._points.data());// 设置数据
    publisher->Publish();// 发布数据
  }
  if (sensors.second) {// 如果存在第二个传感器
    std::shared_ptr<CarlaTransformPublisher> publisher = std::dynamic_pointer_cast<CarlaTransformPublisher>(sensors.second);// 将传感器转换为变换发布者
    publisher->SetData(_seconds, _nanoseconds, (const float*)&sensor_transform.location, (const float*)&sensor_transform.rotation);// 设置变换数据
    publisher->Publish();// 发布变换数据
  }
}

void ROS2::ProcessDataFromSemanticLidar(
    uint64_t sensor_type,// 传感器类型
    carla::streaming::detail::stream_id_type stream_id,// 流ID
    const carla::geom::Transform sensor_transform,// 传感器变换
    carla::sensor::data::SemanticLidarData &data,// 语义激光雷达数据
    void *actor) {// 操作者
  static_assert(sizeof(float) == sizeof(uint32_t), "Invalid float size");// 确保float和uint32_t大小一致
  log_info("Sensor SemanticLidar to ROS data: frame.", _frame, "sensor.", sensor_type, "stream.", stream_id, "points.", data._ser_points.size());// 记录日志：传感器语义激光雷达到ROS数据
  auto sensors = GetOrCreateSensor(ESensors::RayCastSemanticLidar, stream_id, actor);// 获取或创建传感器
  if (sensors.first) {// 如果传感器存在
    std::shared_ptr<CarlaSemanticLidarPublisher> publisher = std::dynamic_pointer_cast<CarlaSemanticLidarPublisher>(sensors.first);// 动态转换到CarlaSemanticLidarPublisher
    size_t width = data._ser_points.size();// 点的数量
    size_t height = 1; // 高度设为1
    publisher->SetData(_seconds, _nanoseconds, 6, height, width, (float*)data._ser_points.data());// 设置数据
    publisher->Publish();// 发布数据
  }
  if (sensors.second) {// 如果第二个传感器存在
    std::shared_ptr<CarlaTransformPublisher> publisher = std::dynamic_pointer_cast<CarlaTransformPublisher>(sensors.second);// 动态转换到CarlaTransformPublisher
    publisher->SetData(_seconds, _nanoseconds, (const float*)&sensor_transform.location, (const float*)&sensor_transform.rotation); // 设置变换数据
    publisher->Publish();// 发布变换数据
  }
}

void ROS2::ProcessDataFromRadar(
    uint64_t sensor_type,// 传感器类型
    carla::streaming::detail::stream_id_type stream_id,// 流ID
    const carla::geom::Transform sensor_transform,// 传感器变换
    const carla::sensor::data::RadarData &data,// 雷达数据
    void *actor) {// 操作者
  log_info("Sensor Radar to ROS data: frame.", _frame, "sensor.", sensor_type, "stream.", stream_id, "points.", data._detections.size());// 记录日志：传感器雷达到ROS数据
  auto sensors = GetOrCreateSensor(ESensors::Radar, stream_id, actor); // 获取或创建传感器
  if (sensors.first) {// 如果传感器存在
    std::shared_ptr<CarlaRadarPublisher> publisher = std::dynamic_pointer_cast<CarlaRadarPublisher>(sensors.first);// 动态转换到CarlaRadarPublisher
    size_t elements = data.GetDetectionCount();// 获取检测数量
    size_t width = elements * sizeof(carla::sensor::data::RadarDetection); // 计算宽度
    size_t height = 1;// 高度设为1
    publisher->SetData(_seconds, _nanoseconds, height, width, elements, (const uint8_t*)data._detections.data()); // 设置数据
    publisher->Publish();// 发布数据
  }
  if (sensors.second) { // 如果第二个传感器存在
    std::shared_ptr<CarlaTransformPublisher> publisher = std::dynamic_pointer_cast<CarlaTransformPublisher>(sensors.second);// 动态转换到CarlaTransformPublisher
    publisher->SetData(_seconds, _nanoseconds, (const float*)&sensor_transform.location, (const float*)&sensor_transform.rotation);// 设置变换数据
    publisher->Publish(); // 发布变换数据
  }
}

void ROS2::ProcessDataFromObstacleDetection(
    uint64_t sensor_type,// 传感器类型
    carla::streaming::detail::stream_id_type stream_id,// 流ID
    const carla::geom::Transform sensor_transform,// 传感器变换
    AActor *first_ctor, // 第一个构造函数
    AActor *second_actor, // 第二个操作者
    float distance,// 距离
    void *actor) {  // 操作者
  log_info("Sensor ObstacleDetector to ROS data: frame.", _frame, "sensor.", sensor_type, "stream.", stream_id, "distance.", distance);// 记录日志：传感器障碍物检测到ROS数据
}
// 处理来自碰撞传感器的数据
// 参数
void ROS2::ProcessDataFromCollisionSensor(
    uint64_t sensor_type,// 传感器类型
    carla::streaming::detail::stream_id_type stream_id, // 流ID
    const carla::geom::Transform sensor_transform,// 传感器变换
    uint32_t other_actor,// 其他操作者
    carla::geom::Vector3D impulse, // 冲击力
    void* actor) { // 操作者
  // 获取或创建一个碰撞传感器
  auto sensors = GetOrCreateSensor(ESensors::CollisionSensor, stream_id, actor); // 获取或创建传感器
  if (sensors.first) {// 如果传感器存在
    // 将其转换为CarlaCollisionPublisher类型
    std::shared_ptr<CarlaCollisionPublisher> publisher = std::dynamic_pointer_cast<CarlaCollisionPublisher>(sensors.first);// 动态转换到CarlaCollisionPublisher
    publisher->SetData(_seconds, _nanoseconds, other_actor, impulse.x, impulse.y, impulse.z);// 设置碰撞数据
    publisher->Publish();
  }
  if (sensors.second) {// 如果第二个传感器存在
    // 将其转换为CarlaCollisionPublisher类型
    std::shared_ptr<CarlaTransformPublisher> publisher = std::dynamic_pointer_cast<CarlaTransformPublisher>(sensors.second);// 动态转换到CarlaTransformPublisher
    publisher->SetData(_seconds, _nanoseconds, (const float*)&sensor_transform.location, (const float*)&sensor_transform.rotation);// 设置变换数据
    publisher->Publish();// 发布变换数据
  }
}
// 该函数用于关闭系统
// 遍历所有发布者和变换对象
// 重置
// 重置时钟发布者和控制器
// 将_enabled设置为false
void ROS2::Shutdown() {// 关闭
  for (auto& element : _publishers) {// 遍历发布者
    element.second.reset();// 重置发布者
  }
  for (auto& element : _transforms) {// 遍历变换
    element.second.reset();// 重置变换
  }
  _clock_publisher.reset();// 重置时钟发布者
  _controller.reset();// 重置控制器
  _enabled = false;// 禁用
}

} // namespace ros2
} // namespace carla
