// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once// 确保该头文件只被包含一次
#define _GLIBCXX_USE_CXX11_ABI 0// 指定使用的C++ ABI版本

#include <memory>// 引入智能指针支持
#include <vector>// 引入向量容器支持

#include "CarlaPublisher.h"// 引入CarlaPublisher的定义

namespace carla {
namespace ros2 {

  struct CarlaRadarPublisherImpl;// 前向声明，用于实现细节

  class CarlaRadarPublisher : public CarlaPublisher {// CarlaRadarPublisher类，负责雷达数据的发布
    public:
      CarlaRadarPublisher(const char* ros_name = "", const char* parent = "");// 构造函数，接受ROS主题名称和父级名称，默认为空字符串
      ~CarlaRadarPublisher();// 析构函数
      CarlaRadarPublisher(const CarlaRadarPublisher&);// 拷贝构造函数
      CarlaRadarPublisher& operator=(const CarlaRadarPublisher&);// 拷贝赋值运算符
      CarlaRadarPublisher(CarlaRadarPublisher&&);// 移动构造函数
      CarlaRadarPublisher& operator=(CarlaRadarPublisher&&);// 移动赋值运算符

      bool Init();// 初始化雷达发布器
      bool Publish();// 发布雷达数据
      void SetData(int32_t seconds, uint32_t nanoseconds, size_t height, size_t width, size_t elements, const uint8_t* data);// 设置雷达数据，包含时间戳、高度、宽度、元素数量和数据指针
      const char* type() const override { return "radar"; }// 返回雷达类型

    private:
      void SetData(int32_t seconds, uint32_t nanoseconds, size_t height, size_t width, size_t elements, std::vector<uint8_t>&& data);// 另一个重载版本的设置数据，使用右值引用来移动数据

    private:
      std::shared_ptr<CarlaRadarPublisherImpl> _impl;// 使用智能指针管理实现细节，避免内存泄漏
  };
}
}
