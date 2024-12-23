// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

//指示头文件只被包含一次
#pragma once
//定义 GLIBCXX 的使用 C++11 ABI 的宏
#define _GLIBCXX_USE_CXX11_ABI 0

//引入内存管理和向量库
#include <memory>
#include <vector>

//引入 CarlaPublisher 类的定义
#include "CarlaPublisher.h"

namespace carla {
namespace ros2 {

    //前向声明结构体 CarlaSpeedometerSensorImpl
  struct CarlaSpeedometerSensorImpl;

  //定义 CarlaSpeedometerSensor 类，继承自 CarlaPublisher
  class CarlaSpeedometerSensor : public CarlaPublisher {
    public:
        //构造函数，接受 ROS 名称和父级名称作为参数，默认为空字符串
      CarlaSpeedometerSensor(const char* ros_name = "", const char* parent = "");
      //析构函数
      ~CarlaSpeedometerSensor();
      //拷贝构造函数
      CarlaSpeedometerSensor(const CarlaSpeedometerSensor&);
      //拷贝赋值运算符
      CarlaSpeedometerSensor& operator=(const CarlaSpeedometerSensor&);
      //移动构造函数
      CarlaSpeedometerSensor(CarlaSpeedometerSensor&&);
      //移动赋值运算符
      CarlaSpeedometerSensor& operator=(CarlaSpeedometerSensor&&);
      
      //初始化函数，返回布尔值指示是否成功
      bool Init();
      //发布数据的函数，返回布尔值指示是否成功
      bool Publish();
      //设置速度数据的函数，接受一个浮点值作为参数
      void SetData(float data);
      //覆盖基类的 type() 方法，返回传感器类型
      const char* type() const override { return "speedometer"; }

    private:
        //使用智能指针管理 CarlaSpeedometerSensorImpl 的实例
      std::shared_ptr<CarlaSpeedometerSensorImpl> _impl;
  };
}//namespace ros2
}//namespace carla
