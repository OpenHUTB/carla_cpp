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
#include "publishers/CarlaClockPublisher.h" // 引入Carla时钟发布者模块
#include "publishers/CarlaRGBCameraPublisher.h" // 引入Carla RGB相机发布者模块
#include "publishers/CarlaDepthCameraPublisher.h" // 引入Carla 深度相机发布者模块
#include "publishers/CarlaNormalsCameraPublisher.h" // 引入Carla 法线相机发布者模块
#include "publishers/CarlaOpticalFlowCameraPublisher.h" // 引入Carla 光流相机发布者模块
#include "publishers/CarlaSSCameraPublisher.h" // 引入Carla 单目相机发布者模块
#include "publishers/CarlaISCameraPublisher.h" // 引入Carla 立体相机发布者模块
#include "publishers/CarlaDVSCameraPublisher.h" // 引入Carla DVS相机发布者模块
#include "publishers/CarlaLidarPublisher.h" // 引入Carla 激光雷达发布者模块
#include "publishers/CarlaSemanticLidarPublisher.h" // 引入Carla 语义激光雷达发布者模块
#include "publishers/CarlaRadarPublisher.h" // 引入Carla 雷达发布者模块
#include "publishers/CarlaIMUPublisher.h" // 引入Carla IMU发布者模块
#include "publishers/CarlaGNSSPublisher.h" // 引入Carla GNSS发布者模块
#include "publishers/CarlaMapSensorPublisher.h" // 引入Carla 地图传感器发布者模块
#include "publishers/CarlaSpeedometerSensor.h" // 引入Carla 速度计传感器模块
#include "publishers/CarlaTransformPublisher.h" // 引入Carla 变换发布者模块
#include "publishers/CarlaCollisionPublisher.h" // 引入Carla 碰撞发布者模块
#include "publishers/CarlaLineInvasionPublisher.h" // 引入Carla 线路侵入发布者模块

#include "subscribers/CarlaSubscriber.h" // 引入Carla 订阅者模块
#include "subscribers/CarlaEgoVehicleControlSubscriber.h" // 引入Carla 自主车辆控制订阅者模块

#include <vector> // 引入向量容器

namespace carla {
namespace ros2 {

// 静态字段
std::shared_ptr<ROS2> ROS2::_instance; // 创建ROS2的共享指针实例

// 传感器列表（应该等同于SensorsRegistry列表）
enum ESensors { // 定义传感器类型枚举
  CollisionSensor, // 碰撞传感器
  DepthCamera, // 深度相机
  NormalsCamera, // 法线相机
  DVSCamera, // DVS相机
  GnssSensor, // GNSS传感器
  InertialMeasurementUnit, // 惯性测量单元
  LaneInvasionSensor, // 车道侵入传感器
  ObstacleDetectionSensor, // 障碍物检测传感器
  OpticalFlowCamera, // 光流相机
  Radar, // 雷达
  RayCastSemanticLidar, // 射线投射语义激光雷达
  RayCastLidar, // 射线投射激光雷达
  RssSensor, // RSS传感器
  SceneCaptureCamera, // 场景捕获相机
  SemanticSegmentationCamera, // 语义分割相机
  InstanceSegmentationCamera, // 实例分割相机
  WorldObserver, // 世界观察者
  CameraGBufferUint8, // 相机G缓冲区（无符号8位整数）
  CameraGBufferFloat // 相机G缓冲区（浮点数）
};

void ROS2::Enable(bool enable) { // 启用或禁用ROS2功能
  _enabled = enable; // 设置启用状态
  log_info("ROS2 enabled: ", _enabled); // 记录启用信息
  _clock_publisher = std::make_shared<CarlaClockPublisher>("clock", ""); // 初始化时钟发布者
  _clock_publisher->Init(); // 调用初始化方法
}

void ROS2::SetFrame(uint64_t frame) { // 设置帧数
  _frame = frame; // 存储新帧数
   //log_info("ROS2 new frame: ", _frame); // 记录新帧数
   if (_controller) { // 如果控制器存在
    void* actor = _controller->GetVehicle(); // 获取车辆实例
    if (_controller->IsAlive()) { // 检查控制器是否存活
      if (_controller->HasNewMessage()) { // 检查是否有新消息
        auto it = _actor_callbacks.find(actor); // 查找对应的回调
        if (it != _actor_callbacks.end()) { // 如果找到回调
          VehicleControl control = _controller->GetMessage(); // 获取控制消息
          it->second(actor, control); // 调用回调函数
        }
      }
    } else { // 如果控制器不存活
      RemoveActorCallback(actor); // 移除该演员的回调
    }
   }
}

void ROS2::SetTimestamp(double timestamp) { // 设置时间戳
  double integral; // 整数部分
  const double fractional = modf(timestamp, &integral); // 分离小数部分
  const double multiplier = 1000000000.0; // 毫微秒乘数
  _seconds = static_cast<int32_t>(integral); // 设置秒数
  _nanoseconds = static_cast<uint32_t>(fractional * multiplier); // 设置纳秒数
  _clock_publisher->SetData(_seconds, _nanoseconds); // 更新时钟数据
  _clock_publisher->Publish(); // 发布时钟数据
   //log_info("ROS2 new timestamp: ", _timestamp); // 记录新时间戳
}

void ROS2::AddActorRosName(void *actor, std::string ros_name) { // 添加演员的ROS名称
  _actor_ros_name.insert({actor, ros_name}); // 插入演员和对应的ROS名称
}

void ROS2::AddActorParentRosName(void *actor, void* parent) { // 添加演员的父级ROS名称
  auto it = _actor_parent_ros_name.find(actor); // 查找演员的父级名称
  if (it != _actor_parent_ros_name.end()) { // 如果找到父级名称
    it->second.push_back(parent); // 将父级添加到列表中
  } else { // 如果未找到
    _actor_parent_ros_name.insert({actor, {parent}}); // 插入新的父级名称
  }
}

void ROS2::RemoveActorRosName(void *actor) { // 移除演员的ROS名称
  _actor_ros_name.erase(actor); // 移除演员名称
  _actor_parent_ros_name.erase(actor); // 移除演员父级名称

  _publishers.erase(actor); // 移除对应的发布者
  _transforms.erase(actor); // 移除对应的变换
}

void ROS2::UpdateActorRosName(void *actor, std::string ros_name) { // 更新演员的ROS名称
  auto it = _actor_ros_name.find(actor); // 查找演员名称
  if (it != _actor_ros_name.end()) { // 如果找到
    it->second = ros_name; // 更新为新名称
  }
}

std::string ROS2::GetActorRosName(void *actor) { // 获取演员的ROS名称
  auto it = _actor_ros_name.find(actor); // 查找演员名称
  if (it != _actor_ros_name.end()) { // 如果找到
    return it->second; // 返回对应的名称
  } else { // 如果未找到
    return std::string(""); // 返回空字符串
  }
}

std::string ROS2::GetActorParentRosName(void *actor) {
  auto it = _actor_parent_ros_name.find(actor);
  if (it != _actor_parent_ros_name.end())
  {
    const std::string current_actor_name = GetActorRosName(actor);
    std::string parent_name;
    for (auto parent_it = it->second.cbegin(); parent_it != it->second.cend(); ++parent_it)
    {
      const std::string name = GetActorRosName(*parent_it);
      if (name == current_actor_name)
      {
        continue;
      }
      if (name.empty())
      {
        continue;
      }
      parent_name = name + '/' + parent_name;
    }
    if (parent_name.back() == '/')
      parent_name.pop_back();
    return parent_name;
  }
  else
    return std::string("");
}

void ROS2::AddActorCallback(void* actor, std::string ros_name, ActorCallback callback) {
  _actor_callbacks.insert({actor, std::move(callback)});

  _controller.reset();
  _controller = std::make_shared<CarlaEgoVehicleControlSubscriber>(actor, ros_name.c_str());
  _controller->Init();
}

void ROS2::RemoveActorCallback(void* actor) {
  _controller.reset();
  _actor_callbacks.erase(actor);
}

std::pair<std::shared_ptr<CarlaPublisher>, std::shared_ptr<CarlaTransformPublisher>> ROS2::GetOrCreateSensor(int type, carla::streaming::detail::stream_id_type id, void* actor) {
  auto it_publishers = _publishers.find(actor);
  auto it_transforms = _transforms.find(actor);
  std::shared_ptr<CarlaPublisher> publisher {};
  std::shared_ptr<CarlaTransformPublisher> transform {};
  if (it_publishers != _publishers.end()) {
    publisher = it_publishers->second;
    if (it_transforms != _transforms.end()) {
      transform = it_transforms->second;
    }
  } else {
    // 没找到传感器，创建一个给定类型
    const std::string string_id = std::to_string(id);
    std::string ros_name = GetActorRosName(actor);
    std::string parent_ros_name = GetActorParentRosName(actor);
    switch(type) {
      case ESensors::CollisionSensor: {
        if (ros_name == "collision__") {
          ros_name.pop_back();
          ros_name.pop_back();
          ros_name += string_id;
          UpdateActorRosName(actor, ros_name);
        }
        std::shared_ptr<CarlaCollisionPublisher> new_publisher = std::make_shared<CarlaCollisionPublisher>(ros_name.c_str(), parent_ros_name.c_str());
        if (new_publisher->Init()) {
          _publishers.insert({actor, new_publisher});
          publisher = new_publisher;
        }
        std::shared_ptr<CarlaTransformPublisher> new_transform = std::make_shared<CarlaTransformPublisher>(ros_name.c_str(), parent_ros_name.c_str());
        if (new_transform->Init()) {
          _transforms.insert({actor, new_transform});
          transform = new_transform;
        }
      } break;
      case ESensors::DepthCamera: {
        if (ros_name == "depth__") {
          ros_name.pop_back();
          ros_name.pop_back();
          ros_name += string_id;
          UpdateActorRosName(actor, ros_name);
        }
        std::shared_ptr<CarlaDepthCameraPublisher> new_publisher = std::make_shared<CarlaDepthCameraPublisher>(ros_name.c_str(), parent_ros_name.c_str());
        if (new_publisher->Init()) {
          _publishers.insert({actor, new_publisher});
          publisher = new_publisher;
        }
        std::shared_ptr<CarlaTransformPublisher> new_transform = std::make_shared<CarlaTransformPublisher>(ros_name.c_str(), parent_ros_name.c_str());
        if (new_transform->Init()) {
          _transforms.insert({actor, new_transform});
          transform = new_transform;
        }
      } break;
      case ESensors::NormalsCamera: {
        if (ros_name == "normals__") {
          ros_name.pop_back();
          ros_name.pop_back();
          ros_name += string_id;
          UpdateActorRosName(actor, ros_name);
        }
        std::shared_ptr<CarlaNormalsCameraPublisher> new_publisher = std::make_shared<CarlaNormalsCameraPublisher>(ros_name.c_str(), parent_ros_name.c_str());
        if (new_publisher->Init()) {
          _publishers.insert({actor, new_publisher});
          publisher = new_publisher;
        }
        std::shared_ptr<CarlaTransformPublisher> new_transform = std::make_shared<CarlaTransformPublisher>(ros_name.c_str(), parent_ros_name.c_str());
        if (new_transform->Init()) {
          _transforms.insert({actor, new_transform});
          transform = new_transform;
        }
      } break;
      case ESensors::DVSCamera: {
        if (ros_name == "dvs__") {
          ros_name.pop_back();
          ros_name.pop_back();
          ros_name += string_id;
          UpdateActorRosName(actor, ros_name);
        }
        std::shared_ptr<CarlaDVSCameraPublisher> new_publisher = std::make_shared<CarlaDVSCameraPublisher>(ros_name.c_str(), parent_ros_name.c_str());
        if (new_publisher->Init()) {
          _publishers.insert({actor, new_publisher});
          publisher = new_publisher;
        }
        std::shared_ptr<CarlaTransformPublisher> new_transform = std::make_shared<CarlaTransformPublisher>(ros_name.c_str(), parent_ros_name.c_str());
        if (new_transform->Init()) {
          _transforms.insert({actor, new_transform});
          transform = new_transform;
        }
      } break;
      case ESensors::GnssSensor: {
        if (ros_name == "gnss__") {
          ros_name.pop_back();
          ros_name.pop_back();
          ros_name += string_id;
          UpdateActorRosName(actor, ros_name);
        }
        std::shared_ptr<CarlaGNSSPublisher> new_publisher = std::make_shared<CarlaGNSSPublisher>(ros_name.c_str(), parent_ros_name.c_str());
        if (new_publisher->Init()) {
          _publishers.insert({actor, new_publisher});
          publisher = new_publisher;
        }
        std::shared_ptr<CarlaTransformPublisher> new_transform = std::make_shared<CarlaTransformPublisher>(ros_name.c_str(), parent_ros_name.c_str());
        if (new_transform->Init()) {
          _transforms.insert({actor, new_transform});
          transform = new_transform;
        }
      } break;
      case ESensors::InertialMeasurementUnit: {
        if (ros_name == "imu__") {
          ros_name.pop_back();
          ros_name.pop_back();
          ros_name += string_id;
          UpdateActorRosName(actor, ros_name);
        }
        std::shared_ptr<CarlaIMUPublisher> new_publisher = std::make_shared<CarlaIMUPublisher>(ros_name.c_str(), parent_ros_name.c_str());
        if (new_publisher->Init()) {
          _publishers.insert({actor, new_publisher});
          publisher = new_publisher;
        }
        std::shared_ptr<CarlaTransformPublisher> new_transform = std::make_shared<CarlaTransformPublisher>(ros_name.c_str(), parent_ros_name.c_str());
        if (new_transform->Init()) {
          _transforms.insert({actor, new_transform});
          transform = new_transform;
        }
      } break;
      case ESensors::LaneInvasionSensor: {
        if (ros_name == "lane_invasion__") {
          ros_name.pop_back();
          ros_name.pop_back();
          ros_name += string_id;
          UpdateActorRosName(actor, ros_name);
        }
        std::shared_ptr<CarlaLineInvasionPublisher> new_publisher = std::make_shared<CarlaLineInvasionPublisher>(ros_name.c_str(), parent_ros_name.c_str());
        if (new_publisher->Init()) {
          _publishers.insert({actor, new_publisher});
          publisher = new_publisher;
        }
        std::shared_ptr<CarlaTransformPublisher> new_transform = std::make_shared<CarlaTransformPublisher>(ros_name.c_str(), parent_ros_name.c_str());
        if (new_transform->Init()) {
          _transforms.insert({actor, new_transform});
          transform = new_transform;
        }
      } break;
      case ESensors::ObstacleDetectionSensor: {
        std::cerr << "Obstacle detection sensor does not have an available publisher" << std::endl;
      } break;
      case ESensors::OpticalFlowCamera: {
        if (ros_name == "optical_flow__") {
          ros_name.pop_back();
          ros_name.pop_back();
          ros_name += string_id;
          UpdateActorRosName(actor, ros_name);
        }
        std::shared_ptr<CarlaOpticalFlowCameraPublisher> new_publisher = std::make_shared<CarlaOpticalFlowCameraPublisher>(ros_name.c_str(), parent_ros_name.c_str());
        if (new_publisher->Init()) {
          _publishers.insert({actor, new_publisher});
          publisher = new_publisher;
        }
        std::shared_ptr<CarlaTransformPublisher> new_transform = std::make_shared<CarlaTransformPublisher>(ros_name.c_str(), parent_ros_name.c_str());
        if (new_transform->Init()) {
          _transforms.insert({actor, new_transform});
          transform = new_transform;
        }
      } break;
      case ESensors::Radar: {
        if (ros_name == "radar__") {
          ros_name.pop_back();
          ros_name.pop_back();
          ros_name += string_id;
          UpdateActorRosName(actor, ros_name);
        }
        std::shared_ptr<CarlaRadarPublisher> new_publisher = std::make_shared<CarlaRadarPublisher>(ros_name.c_str(), parent_ros_name.c_str());
        if (new_publisher->Init()) {
          _publishers.insert({actor, new_publisher});
          publisher = new_publisher;
        }
        std::shared_ptr<CarlaTransformPublisher> new_transform = std::make_shared<CarlaTransformPublisher>(ros_name.c_str(), parent_ros_name.c_str());
        if (new_transform->Init()) {
          _transforms.insert({actor, new_transform});
          transform = new_transform;
        }
      } break;
      case ESensors::RayCastSemanticLidar: {
        if (ros_name == "ray_cast_semantic__") {
          ros_name.pop_back();
          ros_name.pop_back();
          ros_name += string_id;
          UpdateActorRosName(actor, ros_name);
        }
        std::shared_ptr<CarlaSemanticLidarPublisher> new_publisher = std::make_shared<CarlaSemanticLidarPublisher>(ros_name.c_str(), parent_ros_name.c_str());
        if (new_publisher->Init()) {
          _publishers.insert({actor, new_publisher});
          publisher = new_publisher;
        }
        std::shared_ptr<CarlaTransformPublisher> new_transform = std::make_shared<CarlaTransformPublisher>(ros_name.c_str(), parent_ros_name.c_str());
        if (new_transform->Init()) {
          _transforms.insert({actor, new_transform});
          transform = new_transform;
        }
      } break;
      case ESensors::RayCastLidar: {
        if (ros_name == "ray_cast__") {
          ros_name.pop_back();
          ros_name.pop_back();
          ros_name += string_id;
          UpdateActorRosName(actor, ros_name);
        }
        std::shared_ptr<CarlaLidarPublisher> new_publisher = std::make_shared<CarlaLidarPublisher>(ros_name.c_str(), parent_ros_name.c_str());
        if (new_publisher->Init()) {
          _publishers.insert({actor, new_publisher});
          publisher = new_publisher;
        }
        std::shared_ptr<CarlaTransformPublisher> new_transform = std::make_shared<CarlaTransformPublisher>(ros_name.c_str(), parent_ros_name.c_str());
        if (new_transform->Init()) {
          _transforms.insert({actor, new_transform});
          transform = new_transform;
        }
      } break;
      case ESensors::RssSensor: {
        std::cerr << "RSS sensor does not have an available publisher" << std::endl;
      } break;
      case ESensors::SceneCaptureCamera: {
        if (ros_name == "rgb__") {
          ros_name.pop_back();
          ros_name.pop_back();
          ros_name += string_id;
          UpdateActorRosName(actor, ros_name);
        }
        std::shared_ptr<CarlaRGBCameraPublisher> new_publisher = std::make_shared<CarlaRGBCameraPublisher>(ros_name.c_str(), parent_ros_name.c_str());
        if (new_publisher->Init()) {
          _publishers.insert({actor, new_publisher});
          publisher = new_publisher;
        }
        std::shared_ptr<CarlaTransformPublisher> new_transform = std::make_shared<CarlaTransformPublisher>(ros_name.c_str(), parent_ros_name.c_str());
        if (new_transform->Init()) {
          _transforms.insert({actor, new_transform});
          transform = new_transform;
        }
      } break;
      case ESensors::SemanticSegmentationCamera: {
        if (ros_name == "semantic_segmentation__") {
          ros_name.pop_back();
          ros_name.pop_back();
          ros_name += string_id;
          UpdateActorRosName(actor, ros_name);
        }
        std::shared_ptr<CarlaSSCameraPublisher> new_publisher = std::make_shared<CarlaSSCameraPublisher>(ros_name.c_str(), parent_ros_name.c_str());
        if (new_publisher->Init()) {
          _publishers.insert({actor, new_publisher});
          publisher = new_publisher;
        }
        std::shared_ptr<CarlaTransformPublisher> new_transform = std::make_shared<CarlaTransformPublisher>(ros_name.c_str(), parent_ros_name.c_str());
        if (new_transform->Init()) {
          _transforms.insert({actor, new_transform});
          transform = new_transform;
        }
      } break;
      case ESensors::InstanceSegmentationCamera: {
        if (ros_name == "instance_segmentation__") {
          ros_name.pop_back();
          ros_name.pop_back();
          ros_name += string_id;
          UpdateActorRosName(actor, ros_name);
        }
        std::shared_ptr<CarlaISCameraPublisher> new_publisher = std::make_shared<CarlaISCameraPublisher>(ros_name.c_str(), parent_ros_name.c_str());
        if (new_publisher->Init()) {
          _publishers.insert({actor, new_publisher});
          publisher = new_publisher;
        }
        std::shared_ptr<CarlaTransformPublisher> new_transform = std::make_shared<CarlaTransformPublisher>(ros_name.c_str(), parent_ros_name.c_str());
        if (new_transform->Init()) {
          _transforms.insert({actor, new_transform});
          transform = new_transform;
        }
      } break;
      case ESensors::WorldObserver: {
        std::cerr << "World obserser does not have an available publisher" << std::endl;
      } break;
      case ESensors::CameraGBufferUint8: {
        std::cerr << "Camera GBuffer uint8 does not have an available publisher" << std::endl;
      } break;
      case ESensors::CameraGBufferFloat: {
        std::cerr << "Camera GBuffer float does not have an available publisher" << std::endl;
      } break;
      default: {
        std::cerr << "Unknown sensor type" << std::endl;
      }
    }
  }
  return { publisher, transform };
}

void ROS2::ProcessDataFromCamera(
    uint64_t sensor_type,
    carla::streaming::detail::stream_id_type stream_id,
    const carla::geom::Transform sensor_transform,
    int W, int H, float Fov,
    const carla::SharedBufferView buffer,
    void *actor) {

  switch (sensor_type) {
    case ESensors::CollisionSensor:
      log_info("Sensor Collision to ROS data: frame.", _frame, "sensor.", sensor_type, "stream.", stream_id, "buffer.", buffer->size());
      break;
    case ESensors::DepthCamera:
      {
        log_info("Sensor DepthCamera to ROS data: frame.", _frame, "sensor.", sensor_type, "stream.", stream_id, "buffer.", buffer->size());
        auto sensors = GetOrCreateSensor(ESensors::DepthCamera, stream_id, actor);
        if (sensors.first) {
          std::shared_ptr<CarlaDepthCameraPublisher> publisher = std::dynamic_pointer_cast<CarlaDepthCameraPublisher>(sensors.first);
          const carla::sensor::s11n::ImageSerializer::ImageHeader *header =
            reinterpret_cast<const carla::sensor::s11n::ImageSerializer::ImageHeader *>(buffer->data());
          if (!header)
            return;
          if (!publisher->HasBeenInitialized())
            publisher->InitInfoData(0, 0, H, W, Fov, true);
          publisher->SetImageData(_seconds, _nanoseconds, header->height, header->width, (const uint8_t*) (buffer->data() + carla::sensor::s11n::ImageSerializer::header_offset));
          publisher->SetCameraInfoData(_seconds, _nanoseconds);
          publisher->Publish();
        }
        if (sensors.second) {
          std::shared_ptr<CarlaTransformPublisher> publisher = std::dynamic_pointer_cast<CarlaTransformPublisher>(sensors.second);
          publisher->SetData(_seconds, _nanoseconds, (const float*)&sensor_transform.location, (const float*)&sensor_transform.rotation);
          publisher->Publish();
        }
      }
      break;
    case ESensors::NormalsCamera:
      log_info("Sensor NormalsCamera to ROS data: frame.", _frame, "sensor.", sensor_type, "stream.", stream_id, "buffer.", buffer->size());
      {
        auto sensors = GetOrCreateSensor(ESensors::NormalsCamera, stream_id, actor);
        if (sensors.first) {
          std::shared_ptr<CarlaNormalsCameraPublisher> publisher = std::dynamic_pointer_cast<CarlaNormalsCameraPublisher>(sensors.first);
          const carla::sensor::s11n::ImageSerializer::ImageHeader *header =
            reinterpret_cast<const carla::sensor::s11n::ImageSerializer::ImageHeader *>(buffer->data());
          if (!header)
            return;
          if (!publisher->HasBeenInitialized())
            publisher->InitInfoData(0, 0, H, W, Fov, true);
          publisher->SetImageData(_seconds, _nanoseconds, header->height, header->width, (const uint8_t*) (buffer->data() + carla::sensor::s11n::ImageSerializer::header_offset));
          publisher->SetCameraInfoData(_seconds, _nanoseconds);
          publisher->Publish();
        }
        if (sensors.second) {
          std::shared_ptr<CarlaTransformPublisher> publisher = std::dynamic_pointer_cast<CarlaTransformPublisher>(sensors.second);
          publisher->SetData(_seconds, _nanoseconds, (const float*)&sensor_transform.location, (const float*)&sensor_transform.rotation);
          publisher->Publish();
        }
      }
      break;
    case ESensors::LaneInvasionSensor:
      log_info("Sensor LaneInvasionSensor to ROS data: frame.", _frame, "sensor.", sensor_type, "stream.", stream_id, "buffer.", buffer->size());
      {
        auto sensors = GetOrCreateSensor(ESensors::LaneInvasionSensor, stream_id, actor);
        if (sensors.first) {
          std::shared_ptr<CarlaLineInvasionPublisher> publisher = std::dynamic_pointer_cast<CarlaLineInvasionPublisher>(sensors.first);
          publisher->SetData(_seconds, _nanoseconds, (const int32_t*) buffer->data());
          publisher->Publish();
        }
        if (sensors.second) {
          std::shared_ptr<CarlaTransformPublisher> publisher = std::dynamic_pointer_cast<CarlaTransformPublisher>(sensors.second);
          publisher->SetData(_seconds, _nanoseconds, (const float*)&sensor_transform.location, (const float*)&sensor_transform.rotation);
          publisher->Publish();
        }
      }
      break;
    case ESensors::OpticalFlowCamera:
      log_info("Sensor OpticalFlowCamera to ROS data: frame.", _frame, "sensor.", sensor_type, "stream.", stream_id, "buffer.", buffer->size());
      {
        auto sensors = GetOrCreateSensor(ESensors::OpticalFlowCamera, stream_id, actor);
        if (sensors.first) {
          std::shared_ptr<CarlaOpticalFlowCameraPublisher> publisher = std::dynamic_pointer_cast<CarlaOpticalFlowCameraPublisher>(sensors.first);
          const carla::sensor::s11n::OpticalFlowImageSerializer::ImageHeader *header =
            reinterpret_cast<const carla::sensor::s11n::OpticalFlowImageSerializer::ImageHeader *>(buffer->data());
          if (!header)
            return;
          if (!publisher->HasBeenInitialized())
            publisher->InitInfoData(0, 0, H, W, Fov, true);
          publisher->SetImageData(_seconds, _nanoseconds, header->height, header->width, (const float*) (buffer->data() + carla::sensor::s11n::OpticalFlowImageSerializer::header_offset));
          publisher->SetCameraInfoData(_seconds, _nanoseconds);
          publisher->Publish();
        }
        if (sensors.second) {
          std::shared_ptr<CarlaTransformPublisher> publisher = std::dynamic_pointer_cast<CarlaTransformPublisher>(sensors.second);
          publisher->SetData(_seconds, _nanoseconds, (const float*)&sensor_transform.location, (const float*)&sensor_transform.rotation);
          publisher->Publish();
        }
      }
      break;
    case ESensors::RssSensor:
      log_info("Sensor RssSensor to ROS data: frame.", _frame, "sensor.", sensor_type, "stream.", stream_id, "buffer.", buffer->size());
      break;
    case ESensors::SceneCaptureCamera:
    {
      log_info("Sensor SceneCaptureCamera to ROS data: frame.", _frame, "sensor.", sensor_type, "stream.", stream_id, "buffer.", buffer->size());
      {
        auto sensors = GetOrCreateSensor(ESensors::SceneCaptureCamera, stream_id, actor);
        if (sensors.first) {
          std::shared_ptr<CarlaRGBCameraPublisher> publisher = std::dynamic_pointer_cast<CarlaRGBCameraPublisher>(sensors.first);
          const carla::sensor::s11n::ImageSerializer::ImageHeader *header =
            reinterpret_cast<const carla::sensor::s11n::ImageSerializer::ImageHeader *>(buffer->data());
          if (!header)
            return;
          if (!publisher->HasBeenInitialized())
            publisher->InitInfoData(0, 0, H, W, Fov, true);
          publisher->SetImageData(_seconds, _nanoseconds, header->height, header->width, (const uint8_t*) (buffer->data() + carla::sensor::s11n::ImageSerializer::header_offset));
          publisher->SetCameraInfoData(_seconds, _nanoseconds);
          publisher->Publish();
        }
        if (sensors.second) {
          std::shared_ptr<CarlaTransformPublisher> publisher = std::dynamic_pointer_cast<CarlaTransformPublisher>(sensors.second);
          publisher->SetData(_seconds, _nanoseconds, (const float*)&sensor_transform.location, (const float*)&sensor_transform.rotation);
          publisher->Publish();
        }
      }
      break;
    }
    case ESensors::SemanticSegmentationCamera:
      log_info("Sensor SemanticSegmentationCamera to ROS data: frame.", _frame, "sensor.", sensor_type, "stream.", stream_id, "buffer.", buffer->size());
      {
        auto sensors = GetOrCreateSensor(ESensors::SemanticSegmentationCamera, stream_id, actor);
        if (sensors.first) {
          std::shared_ptr<CarlaSSCameraPublisher> publisher = std::dynamic_pointer_cast<CarlaSSCameraPublisher>(sensors.first);
          const carla::sensor::s11n::ImageSerializer::ImageHeader *header =
            reinterpret_cast<const carla::sensor::s11n::ImageSerializer::ImageHeader *>(buffer->data());
          if (!header)
            return;
          if (!publisher->HasBeenInitialized())
            publisher->InitInfoData(0, 0, H, W, Fov, true);
          publisher->SetImageData(_seconds, _nanoseconds, header->height, header->width, (const uint8_t*) (buffer->data() + carla::sensor::s11n::ImageSerializer::header_offset));
          publisher->SetCameraInfoData(_seconds, _nanoseconds);
          publisher->Publish();
        }
        if (sensors.second) {
          std::shared_ptr<CarlaTransformPublisher> publisher = std::dynamic_pointer_cast<CarlaTransformPublisher>(sensors.second);
          publisher->SetData(_seconds, _nanoseconds, (const float*)&sensor_transform.location, (const float*)&sensor_transform.rotation);
          publisher->Publish();
        }
      }
      break;
    case ESensors::InstanceSegmentationCamera:
      log_info("Sensor InstanceSegmentationCamera to ROS data: frame.", _frame, "sensor.", sensor_type, "stream.", stream_id, "buffer.", buffer->size());
      {
        auto sensors = GetOrCreateSensor(ESensors::InstanceSegmentationCamera, stream_id, actor);
        if (sensors.first) {
          std::shared_ptr<CarlaISCameraPublisher> publisher = std::dynamic_pointer_cast<CarlaISCameraPublisher>(sensors.first);
          const carla::sensor::s11n::ImageSerializer::ImageHeader *header =
            reinterpret_cast<const carla::sensor::s11n::ImageSerializer::ImageHeader *>(buffer->data());
          if (!header)
            return;
          if (!publisher->HasBeenInitialized())
            publisher->InitInfoData(0, 0, H, W, Fov, true);
          publisher->SetImageData(_seconds, _nanoseconds, header->height, header->width, (const uint8_t*) (buffer->data() + carla::sensor::s11n::ImageSerializer::header_offset));
          publisher->SetCameraInfoData(_seconds, _nanoseconds);
          publisher->Publish();
        }
        if (sensors.second) {
          std::shared_ptr<CarlaTransformPublisher> publisher = std::dynamic_pointer_cast<CarlaTransformPublisher>(sensors.second);
          publisher->SetData(_seconds, _nanoseconds, (const float*)&sensor_transform.location, (const float*)&sensor_transform.rotation);
          publisher->Publish();
        }
      }
      break;
    case ESensors::WorldObserver:
      log_info("Sensor WorldObserver to ROS data: frame.", _frame, "sensor.", sensor_type, "stream.", stream_id, "buffer.", buffer->size());
      break;
    case ESensors::CameraGBufferUint8:
      log_info("Sensor CameraGBufferUint8 to ROS data: frame.", _frame, "sensor.", sensor_type, "stream.", stream_id, "buffer.", buffer->size());
      break;
    case ESensors::CameraGBufferFloat:
      log_info("Sensor CameraGBufferFloat to ROS data: frame.", _frame, "sensor.", sensor_type, "stream.", stream_id, "buffer.", buffer->size());
      break;
    default:
      log_info("Sensor to ROS data: frame.", _frame, "sensor.", sensor_type, "stream.", stream_id, "buffer.", buffer->size());
  }
}

void ROS2::ProcessDataFromGNSS(
    uint64_t sensor_type,
    carla::streaming::detail::stream_id_type stream_id,
    const carla::geom::Transform sensor_transform,
    const carla::geom::GeoLocation &data,
    void *actor) {
  log_info("Sensor GnssSensor to ROS data: frame.", _frame, "sensor.", sensor_type, "stream.", stream_id, "geo.", data.latitude, data.longitude, data.altitude);
  auto sensors = GetOrCreateSensor(ESensors::GnssSensor, stream_id, actor);
  if (sensors.first) {
    std::shared_ptr<CarlaGNSSPublisher> publisher = std::dynamic_pointer_cast<CarlaGNSSPublisher>(sensors.first);
    publisher->SetData(_seconds, _nanoseconds, reinterpret_cast<const double*>(&data));
    publisher->Publish();
  }
  if (sensors.second) {
    std::shared_ptr<CarlaTransformPublisher> publisher = std::dynamic_pointer_cast<CarlaTransformPublisher>(sensors.second);
    publisher->SetData(_seconds, _nanoseconds, (const float*)&sensor_transform.location, (const float*)&sensor_transform.rotation);
    publisher->Publish();
  }
}

void ROS2::ProcessDataFromIMU(
    uint64_t sensor_type,
    carla::streaming::detail::stream_id_type stream_id,
    const carla::geom::Transform sensor_transform,
    carla::geom::Vector3D accelerometer,
    carla::geom::Vector3D gyroscope,
    float compass,
    void *actor) {
  log_info("Sensor InertialMeasurementUnit to ROS data: frame.", _frame, "sensor.", sensor_type, "stream.", stream_id, "imu.", accelerometer.x, gyroscope.x, compass);
  auto sensors = GetOrCreateSensor(ESensors::InertialMeasurementUnit, stream_id, actor);
  if (sensors.first) {
    std::shared_ptr<CarlaIMUPublisher> publisher = std::dynamic_pointer_cast<CarlaIMUPublisher>(sensors.first);
    publisher->SetData(_seconds, _nanoseconds, reinterpret_cast<float*>(&accelerometer), reinterpret_cast<float*>(&gyroscope), compass);
    publisher->Publish();
  }
  if (sensors.second) {
    std::shared_ptr<CarlaTransformPublisher> publisher = std::dynamic_pointer_cast<CarlaTransformPublisher>(sensors.second);
    publisher->SetData(_seconds, _nanoseconds, (const float*)&sensor_transform.location, (const float*)&sensor_transform.rotation);
    publisher->Publish();
  }
}

void ROS2::ProcessDataFromDVS(
    uint64_t sensor_type,
    carla::streaming::detail::stream_id_type stream_id,
    const carla::geom::Transform sensor_transform,
    const carla::SharedBufferView buffer,
    int W, int H, float Fov,
    void *actor) {
  log_info("Sensor DVS to ROS data: frame.", _frame, "sensor.", sensor_type, "stream.", stream_id);
  auto sensors = GetOrCreateSensor(ESensors::DVSCamera, stream_id, actor);
  if (sensors.first) {
    std::shared_ptr<CarlaDVSCameraPublisher> publisher = std::dynamic_pointer_cast<CarlaDVSCameraPublisher>(sensors.first);
    const carla::sensor::s11n::ImageSerializer::ImageHeader *header =
      reinterpret_cast<const carla::sensor::s11n::ImageSerializer::ImageHeader *>(buffer->data());
    if (!header)
      return;
    if (!publisher->HasBeenInitialized())
      publisher->InitInfoData(0, 0, H, W, Fov, true);
    size_t elements = (buffer->size() - carla::sensor::s11n::ImageSerializer::header_offset) / sizeof(carla::sensor::data::DVSEvent);
    publisher->SetImageData(_seconds, _nanoseconds, elements, header->height, header->width, (const uint8_t*) (buffer->data() + carla::sensor::s11n::ImageSerializer::header_offset));
    publisher->SetCameraInfoData(_seconds, _nanoseconds);
    publisher->SetPointCloudData(1, elements * sizeof(carla::sensor::data::DVSEvent), elements, (const uint8_t*) (buffer->data() + carla::sensor::s11n::ImageSerializer::header_offset));
    publisher->Publish();
  }
  if (sensors.second) {
    std::shared_ptr<CarlaTransformPublisher> publisher = std::dynamic_pointer_cast<CarlaTransformPublisher>(sensors.second);
    publisher->SetData(_seconds, _nanoseconds, (const float*)&sensor_transform.location, (const float*)&sensor_transform.rotation);
    publisher->Publish();
  }
}

void ROS2::ProcessDataFromLidar(
    uint64_t sensor_type,
    carla::streaming::detail::stream_id_type stream_id,
    const carla::geom::Transform sensor_transform,
    carla::sensor::data::LidarData &data,
    void *actor) {
  log_info("Sensor Lidar to ROS data: frame.", _frame, "sensor.", sensor_type, "stream.", stream_id, "points.", data._points.size());
  auto sensors = GetOrCreateSensor(ESensors::RayCastLidar, stream_id, actor);
  if (sensors.first) {
    std::shared_ptr<CarlaLidarPublisher> publisher = std::dynamic_pointer_cast<CarlaLidarPublisher>(sensors.first);
    size_t width = data._points.size();
    size_t height = 1;
    publisher->SetData(_seconds, _nanoseconds, height, width, (float*)data._points.data());
    publisher->Publish();
  }
  if (sensors.second) {
    std::shared_ptr<CarlaTransformPublisher> publisher = std::dynamic_pointer_cast<CarlaTransformPublisher>(sensors.second);
    publisher->SetData(_seconds, _nanoseconds, (const float*)&sensor_transform.location, (const float*)&sensor_transform.rotation);
    publisher->Publish();
  }
}

void ROS2::ProcessDataFromSemanticLidar(
    uint64_t sensor_type,
    carla::streaming::detail::stream_id_type stream_id,
    const carla::geom::Transform sensor_transform,
    carla::sensor::data::SemanticLidarData &data,
    void *actor) {
  static_assert(sizeof(float) == sizeof(uint32_t), "Invalid float size");
  log_info("Sensor SemanticLidar to ROS data: frame.", _frame, "sensor.", sensor_type, "stream.", stream_id, "points.", data._ser_points.size());
  auto sensors = GetOrCreateSensor(ESensors::RayCastSemanticLidar, stream_id, actor);
  if (sensors.first) {
    std::shared_ptr<CarlaSemanticLidarPublisher> publisher = std::dynamic_pointer_cast<CarlaSemanticLidarPublisher>(sensors.first);
    size_t width = data._ser_points.size();
    size_t height = 1;
    publisher->SetData(_seconds, _nanoseconds, 6, height, width, (float*)data._ser_points.data());
    publisher->Publish();
  }
  if (sensors.second) {
    std::shared_ptr<CarlaTransformPublisher> publisher = std::dynamic_pointer_cast<CarlaTransformPublisher>(sensors.second);
    publisher->SetData(_seconds, _nanoseconds, (const float*)&sensor_transform.location, (const float*)&sensor_transform.rotation);
    publisher->Publish();
  }
}

void ROS2::ProcessDataFromRadar(
    uint64_t sensor_type,
    carla::streaming::detail::stream_id_type stream_id,
    const carla::geom::Transform sensor_transform,
    const carla::sensor::data::RadarData &data,
    void *actor) {
  log_info("Sensor Radar to ROS data: frame.", _frame, "sensor.", sensor_type, "stream.", stream_id, "points.", data._detections.size());
  auto sensors = GetOrCreateSensor(ESensors::Radar, stream_id, actor);
  if (sensors.first) {
    std::shared_ptr<CarlaRadarPublisher> publisher = std::dynamic_pointer_cast<CarlaRadarPublisher>(sensors.first);
    size_t elements = data.GetDetectionCount();
    size_t width = elements * sizeof(carla::sensor::data::RadarDetection);
    size_t height = 1;
    publisher->SetData(_seconds, _nanoseconds, height, width, elements, (const uint8_t*)data._detections.data());
    publisher->Publish();
  }
  if (sensors.second) {
    std::shared_ptr<CarlaTransformPublisher> publisher = std::dynamic_pointer_cast<CarlaTransformPublisher>(sensors.second);
    publisher->SetData(_seconds, _nanoseconds, (const float*)&sensor_transform.location, (const float*)&sensor_transform.rotation);
    publisher->Publish();
  }
}

void ROS2::ProcessDataFromObstacleDetection(
    uint64_t sensor_type,
    carla::streaming::detail::stream_id_type stream_id,
    const carla::geom::Transform sensor_transform,
    AActor *first_ctor,
    AActor *second_actor,
    float distance,
    void *actor) {
  log_info("Sensor ObstacleDetector to ROS data: frame.", _frame, "sensor.", sensor_type, "stream.", stream_id, "distance.", distance);
}

void ROS2::ProcessDataFromCollisionSensor(
    uint64_t sensor_type,
    carla::streaming::detail::stream_id_type stream_id,
    const carla::geom::Transform sensor_transform,
    uint32_t other_actor,
    carla::geom::Vector3D impulse,
    void* actor) {
  auto sensors = GetOrCreateSensor(ESensors::CollisionSensor, stream_id, actor);
  if (sensors.first) {
    std::shared_ptr<CarlaCollisionPublisher> publisher = std::dynamic_pointer_cast<CarlaCollisionPublisher>(sensors.first);
    publisher->SetData(_seconds, _nanoseconds, other_actor, impulse.x, impulse.y, impulse.z);
    publisher->Publish();
  }
  if (sensors.second) {
    std::shared_ptr<CarlaTransformPublisher> publisher = std::dynamic_pointer_cast<CarlaTransformPublisher>(sensors.second);
    publisher->SetData(_seconds, _nanoseconds, (const float*)&sensor_transform.location, (const float*)&sensor_transform.rotation);
    publisher->Publish();
  }
}

void ROS2::Shutdown() {
  for (auto& element : _publishers) {
    element.second.reset();
  }
  for (auto& element : _transforms) {
    element.second.reset();
  }
  _clock_publisher.reset();
  _controller.reset();
  _enabled = false;
}

} // namespace ros2
} // namespace carla
