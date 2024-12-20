// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

// 指示头文件只被包含一次
#pragma once
// 定义 GLIBCXX 的使用 C++11 ABI 的宏
#define _GLIBCXX_USE_CXX11_ABI 0

// 引入内存管理和向量库
#include <memory>
#include <vector>

// 引入 CarlaPublisher 类的定义
#include "CarlaPublisher.h"

namespace carla {
namespace ros2 {

    // 前向声明结构体 CarlaLidarPublisherImpl
  struct CarlaLidarPublisherImpl;

  // 定义 CarlaLidarPublisher 类，继承自 CarlaPublisher 
  class CarlaLidarPublisher : public CarlaPublisher {
    public:
        // 构造函数，接受 ROS 名称和父级名称作为参数，默认为空字符串
      CarlaLidarPublisher(const char* ros_name = "", const char* parent = "");
      // 析构函数
      ~CarlaLidarPublisher();
      // 拷贝构造函数
      CarlaLidarPublisher(const CarlaLidarPublisher&);
      // 拷贝赋值运算符
      CarlaLidarPublisher& operator=(const CarlaLidarPublisher&);
      // 移动构造函数
      CarlaLidarPublisher(CarlaLidarPublisher&&);
      // 移动赋值运算符
      CarlaLidarPublisher& operator=(CarlaLidarPublisher&&);

      // 初始化函数，返回布尔值指示是否成功
      bool Init();
      // 发布数据的函数，返回布尔值指示是否成功
      bool Publish();
      // 设置激光雷达数据的函数，接收时间戳、高度、宽度和数据指针
      void SetData(int32_t seconds, uint32_t nanoseconds, size_t height, size_t width, float* data);
      // 覆盖基类的 type() 方法，返回传感器类型
      const char* type() const override { return "lidar"; }

    private:
        // 私有设置数据的函数，接收时间戳、高度、宽度和以 rvalue 引用方式传递的向量数据
      void SetData(int32_t seconds, uint32_t nanoseconds, size_t height, size_t width, std::vector<uint8_t>&& data);

    private:
        // 使用智能指针管理 CarlaLidarPublisherImpl 的实例
      std::shared_ptr<CarlaLidarPublisherImpl> _impl;
  };
}// namespace ros2
}// namespace carla
