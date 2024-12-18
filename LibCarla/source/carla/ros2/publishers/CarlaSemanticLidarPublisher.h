// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once// 指示编译器这个头文件已经被包含过一次，防止重复包含。
#define _GLIBCXX_USE_CXX11_ABI 0// 定义宏_GLIBCXX_USE_CXX11_ABI，设置为0，这通常用于控制C++库ABI的版本。

#include <memory>// 包含C++标准库中的智能指针和向量容器。
#include <vector>

#include "CarlaPublisher.h"// 包含自定义的CarlaPublisher头文件。
// 定义carla命名空间下的ros2子命名空间。
namespace carla {
namespace ros2 {

  struct CarlaSemanticLidarPublisherImpl;// 声明一个内部使用的类CarlaSemanticLidarPublisherImpl，具体实现细节未给出。
  // 定义CarlaSemanticLidarPublisher类，继承自CarlaPublisher。
  class CarlaSemanticLidarPublisher : public CarlaPublisher {
    public:
      CarlaSemanticLidarPublisher(const char* ros_name = "", const char* parent = "");// 构造函数，接受ROS名称和父节点名称作为参数，并初始化类成员。
      ~CarlaSemanticLidarPublisher();// 析构函数，用于清理资源。
      CarlaSemanticLidarPublisher(const CarlaSemanticLidarPublisher&);// 拷贝构造函数。
      CarlaSemanticLidarPublisher& operator=(const CarlaSemanticLidarPublisher&); // 拷贝赋值运算符。
      CarlaSemanticLidarPublisher(CarlaSemanticLidarPublisher&&);// 移动构造函数。
      CarlaSemanticLidarPublisher& operator=(CarlaSemanticLidarPublisher&&);// 移动赋值运算符。

      bool Init();// 初始化方法，准备发布数据。
      bool Publish();// 发布数据的方法。
      void SetData(int32_t seconds, uint32_t nanoseconds, size_t elements, size_t height, size_t width, float* data); // 设置数据，包括时间戳和点云数据。
      const char* type() const override { return "semantic lidar"; } // 返回发布数据的类型。

    private:
      void SetData(int32_t seconds, uint32_t nanoseconds, size_t height, size_t width, std::vector<uint8_t>&& data);// 私有方法，设置数据，包括时间戳和点云数据的另一种形式。

    private:
      std::shared_ptr<CarlaSemanticLidarPublisherImpl> _impl;// 一个指向内部实现类的智能指针。
  };
}
}
