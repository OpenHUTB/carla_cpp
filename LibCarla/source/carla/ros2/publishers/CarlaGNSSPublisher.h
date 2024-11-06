// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
#define _GLIBCXX_USE_CXX11_ABI 0

#include <memory>// 引入智能指针相关的头文件
/// @file  
/// @brief 包含CarlaGNSSPublisher类的声明，该类用于在ROS 2中发布CARLA的GNSS数据。
#include "CarlaPublisher.h"// 引入CarlaPublisher类的头文件  
/// @namespace carla  
/// @brief CARLA相关的命名空间。
namespace carla {
    /// @namespace ros2  
/// @brief ROS 2相关的命名空间，包含与ROS 2集成的类和方法。 
namespace ros2 {
    /// @struct CarlaGNSSPublisherImpl  
  /// @brief CarlaGNSSPublisher的内部实现结构体，使用Pimpl（Pointer to IMPLementation）技巧隐藏实现细节。
  struct CarlaGNSSPublisherImpl;
  /// @class CarlaGNSSPublisher  
  /// @brief 用于在ROS 2中发布CARLA的GNSS（全球导航卫星系统）数据的类。  
  /// 继承自CarlaPublisher类，专门用于初始化、设置和发布GNSS数据。  
  class CarlaGNSSPublisher : public CarlaPublisher {
    public:
        /// @brief 构造函数，用于创建CarlaGNSSPublisher对象。  
      /// @param ros_name ROS节点的名称（可选），默认为空字符串。  
      /// @param parent 父节点的名称（可选），默认为空字符串。  
      CarlaGNSSPublisher(const char* ros_name = "", const char* parent = "");
      /// @brief 析构函数，用于销毁CarlaGNSSPublisher对象。
      ~CarlaGNSSPublisher();
      /// @brief 拷贝构造函数，用于创建当前对象的深拷贝。
      CarlaGNSSPublisher(const CarlaGNSSPublisher&);
      /// @brief 拷贝赋值运算符，用于将当前对象的状态复制到另一个同类型的对象中。
      CarlaGNSSPublisher& operator=(const CarlaGNSSPublisher&);
      /// @brief 移动构造函数，用于创建当前对象的移动拷贝，避免不必要的深拷贝。
      CarlaGNSSPublisher(CarlaGNSSPublisher&&);
      /// @brief 移动赋值运算符，用于将当前对象的状态移动到另一个同类型的对象中。
      CarlaGNSSPublisher& operator=(CarlaGNSSPublisher&&);
      /// @brief 初始化函数，用于设置ROS节点和发布者。  
      /// @return 如果初始化成功，则返回true；否则返回false。  
      bool Init();
      /// @brief 发布GNSS数据。  
     /// @return 如果发布成功，则返回true；否则返回false。
      bool Publish();
      /// @brief 设置GNSS数据。  
    /// @param seconds 时间戳的秒部分。  
    /// @param nanoseconds 时间戳的纳秒部分。  
    /// @param data 指向GNSS数据的指针，数据以double类型数组的形式提供。
      void SetData(int32_t seconds, uint32_t nanoseconds, const double* data);
      /// @brief 返回发布者的类型名称。  
     /// @return 类型名称字符串，对于GNSS发布者，返回"gnss"。
      const char* type() const override { return "gnss"; }

    private:
        /// @brief 指向CarlaGNSSPublisherImpl的智能指针，用于隐藏实现细节。
      std::shared_ptr<CarlaGNSSPublisherImpl> _impl;
  };
}
}
