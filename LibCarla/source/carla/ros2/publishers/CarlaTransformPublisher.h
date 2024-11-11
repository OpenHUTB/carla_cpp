// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
#define _GLIBCXX_USE_CXX11_ABI 0

#include <memory>
#include <vector>//引入所需的头文件 

#include "CarlaPublisher.h"//引入CarlaPublisher.h类的声明 

namespace carla {
namespace ros2 {
//定义命名空间carla::ros2,用于组织代码 
  struct CarlaTransformPublisherImpl;//声明一个私有的结构体CarlaTransformPublisherImpl 
//定义CarlaTransformPublisher类，继承自CarlaPublisher类 
  class CarlaTransformPublisher : public CarlaPublisher {
    public:
      CarlaTransformPublisher(const char* ros_name = "", const char* parent = "");//构造函数，接受ROS节点名称和父节点名称作为参数 
      ~CarlaTransformPublisher();//析构函数，用于清理资源 
      CarlaTransformPublisher(const CarlaTransformPublisher&);//构造拷贝函数 
      CarlaTransformPublisher& operator=(const CarlaTransformPublisher&);//拷贝运算赋值运算符 
      CarlaTransformPublisher(CarlaTransformPublisher&&);//移动构造函数 
      CarlaTransformPublisher& operator=(CarlaTransformPublisher&&);//移动赋值运算符 

      bool Init();//初始化方法，用于设置发布者 
      bool Publish();//发布方法，用于发布变换信息 
      void SetData(int32_t seconds, uint32_t nanoseconds, const float* translation, const float* rotation);//参数包括时间戳（秒和纳秒），平移向量和旋转向量 
      const char* type() const override { return "transform"; }//返回类型名称，重写基类方法 

    private://私有成员变量，指向CarlaTransformPublisherImpl的智能指针 
      std::shared_ptr<CarlaTransformPublisherImpl> _impl;//用于实现类的内部功能 
  };
}
}
