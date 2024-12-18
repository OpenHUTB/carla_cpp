// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
#define _GLIBCXX_USE_CXX11_ABI 0

#include <memory>
#include <vector>

#include "CarlaSubscriber.h"
#include "carla/ros2/ROS2CallbackData.h"

namespace carla {
namespace ros2 {

    /// CarlaEgoVehicleControlSubscriber的内部实现结构，用于隐藏实现细节。
  struct CarlaEgoVehicleControlSubscriberImpl;
  /// CarlaEgoVehicleControlSubscriber类，用于订阅并处理来自ROS2的自动驾驶车辆控制消息
  class CarlaEgoVehicleControlSubscriber : public CarlaSubscriber {
    public:
        /// 构造函数，初始化用户并关联到指定的车辆。
      /// @param vehicle 指向车辆实例的指针。
      /// @param ros_name ROS2节点的名称，默认为空字符串。
      /// @param parent 父节点的名称，默认为空字符串。
      CarlaEgoVehicleControlSubscriber(void* vehicle, const char* ros_name = "", const char* parent = "");
      /// 析构函数，清理资源并销毁用户。
      ~CarlaEgoVehicleControlSubscriber();
      /// 拷贝构造函数，创建当前对象的深拷贝。
      CarlaEgoVehicleControlSubscriber(const CarlaEgoVehicleControlSubscriber&);
      /// 拷贝赋值运算符，用于深拷贝赋值。
      CarlaEgoVehicleControlSubscriber& operator=(const CarlaEgoVehicleControlSubscriber&);
      /// 移动构造函数，创建当前对象的移动拷贝。
      CarlaEgoVehicleControlSubscriber(CarlaEgoVehicleControlSubscriber&&);
      /// 移动赋值运算符，用于移动赋值。
      CarlaEgoVehicleControlSubscriber& operator=(CarlaEgoVehicleControlSubscriber&&);
      /// 检查是否有新的控制消息到达。
      /// @return 如果有新消息，则返回true；否则返回false。
      bool HasNewMessage();
      /// 检查用户是否仍然活跃。
      /// @return 如果用户活跃，则返回true；否则返回false
      bool IsAlive();
      /// 获取最新的车辆控制消息。
      /// @return 包含控制指令的VehicleControl对象。
      VehicleControl GetMessage();
      /// 获取与用户关联的车辆实例的指针。
     /// @return 指向车辆实例的指针。
      void* GetVehicle();
      /// 初始化用户。
      /// @return 如果初始化成功，则返回true；否则返回false。
      bool Init();
      /// 读取并处理新的控制消息。
      /// @return 如果读取成功，则返回true；否则返回false。
      bool Read();
      /// 返回用户的类型名称。
      /// @return 类型为"Ego vehicle control"的字符串。
      const char* type() const override { return "Ego vehicle control"; }

      /// （内部使用）将控制消息转发给内部处理函数。
      /// @param control 包含控制指令的VehicleControl对象。
      /// @warning 此方法仅供内部使用，外部调用可能会导致未定义行为。
      void ForwardMessage(VehicleControl control);
      /// 销毁用户并释放相关资源。
      void DestroySubscriber();
    private:
        /// 设置内部数据，包括时间戳、车辆ID和控制数据。
      /// @param seconds 时间戳的秒部分。
      /// @param nanoseconds 时间戳的纳秒部分。
      /// @param actor_id 车辆ID。
      /// @param data 控制指令的浮点数数组。
      void SetData(int32_t seconds, uint32_t nanoseconds, uint32_t actor_id, std::vector<float>&& data);

    private:
        /// 指向内部实现结构的智能指针，用于隐藏实现细节。
      std::shared_ptr<CarlaEgoVehicleControlSubscriberImpl> _impl;
  };
}
}
