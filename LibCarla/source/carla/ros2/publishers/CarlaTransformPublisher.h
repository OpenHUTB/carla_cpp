// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
#define _GLIBCXX_USE_CXX11_ABI 0// 定义宏以使用旧的 C++ ABI，通常用于与某些库的兼容性。  

#include <memory>// 引入内存管理相关的头文件，主要用于智能指针。  

#include <vector>

#include "CarlaPublisher.h"

namespace carla {
namespace ros2 {// 定义命名空间 carla::ros2，组织相关的类和功能。  

  struct CarlaTransformPublisherImpl;// 前向声明 CarlaTransformPublisherImpl 结构体，具体实现将隐藏在 Pimpl 模式中。

  class CarlaTransformPublisher : public CarlaPublisher {  // 定义 CarlaTransformPublisher 类，继承自 CarlaPublisher 类。
    public:
      CarlaTransformPublisher(const char* ros_name = "", const char* parent = "");// 构造函数，接受 ROS 名称和父级名称，默认值为空字符串。
      ~CarlaTransformPublisher();  // 析构函数，负责清理资源。
      CarlaTransformPublisher(const CarlaTransformPublisher&);// 拷贝构造函数，定义如何复制该类的实例。 
      CarlaTransformPublisher& operator=(const CarlaTransformPublisher&);  // 拷贝赋值运算符，定义如何赋值一个实例给另一个实例。
      CarlaTransformPublisher(CarlaTransformPublisher&&);// 移动构造函数，定义如何移动一个实例。
      CarlaTransformPublisher& operator=(CarlaTransformPublisher&&);// 移动赋值运算符，定义如何将一个实例的资源转移到另一个实例。  

      bool Init();// 初始化函数，返回布尔值表示初始化是否成功。  
      bool Publish();// 发布函数，返回布尔值表示发布是否成功。  
      void SetData(int32_t seconds, uint32_t nanoseconds, const float* translation, const float* rotation);// 设置数据函数，接受时间（秒和纳秒）以及平移和旋转的浮点数组。  
      const char* type() const override { return "transform"; }// 重写基类的 type() 方法，返回字符串 "transform"，表示该发布者的类型。  


    private:
      std::shared_ptr<CarlaTransformPublisherImpl> _impl; // 使用智能指针管理 CarlaTransformPublisherImpl 的实例，隐藏实现细节。  
  };
  };
}
}
