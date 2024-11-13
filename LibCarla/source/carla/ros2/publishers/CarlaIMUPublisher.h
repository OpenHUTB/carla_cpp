// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

// 指示头文件只被包含一次  
#pragma once
// 定义 GLIBCXX 的使用 C++11 ABI 的宏  
#define _GLIBCXX_USE_CXX11_ABI 0

// 引入内存管理库  
#include <memory>

// 引入 CarlaPublisher 类的定义  
#include "CarlaPublisher.h"

namespace carla {
namespace ros2 {

    // 前向声明结构体 CarlaIMUPublisherImpl  
  struct CarlaIMUPublisherImpl;

  // 定义 CarlaIMUPublisher 类，继承自 CarlaPublisher  
  class CarlaIMUPublisher : public CarlaPublisher {
    public:
        // 构造函数，接受 ROS 名称和父级名称作为参数  
      CarlaIMUPublisher(const char* ros_name = "", const char* parent = "");
      // 析构函数  
      ~CarlaIMUPublisher();
      // 拷贝构造函数
      CarlaIMUPublisher(const CarlaIMUPublisher&);
      // 拷贝赋值运算符 
      CarlaIMUPublisher& operator=(const CarlaIMUPublisher&);
      // 移动构造函数
      CarlaIMUPublisher(CarlaIMUPublisher&&);
      // 移动赋值运算符
      CarlaIMUPublisher& operator=(CarlaIMUPublisher&&);

      // 初始化函数，返回布尔值指示是否成功
      bool Init();
      // 发布数据的函数，返回布尔值指示是否成功
      bool Publish();
      // 设置 IMU 数据的函数，接收时间戳和传感器数据
      void SetData(int32_t seconds, uint32_t nanoseconds, float* accelerometer, float* gyroscope, float compass);
      // 覆盖基类的 type() 方法，返回传感器类型
      const char* type() const override { return "inertial measurement unit"; }

    private:
        // 使用智能指针管理 CarlaIMUPublisherImpl 的实例
      std::shared_ptr<CarlaIMUPublisherImpl> _impl;
  };
}// namespace ros2
}// namespace carla
