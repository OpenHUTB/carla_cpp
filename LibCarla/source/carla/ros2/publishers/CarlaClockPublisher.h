// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
#define _GLIBCXX_USE_CXX11_ABI 0

#include <memory>
#include <vector>

#include "CarlaPublisher.h"
/**
 * @namespace carla::ros2
 * @brief 命名空间carla::ros2，包含了与CARLA和ROS 2集成相关的类和函数。
 */
namespace carla {
namespace ros2 {
    /**
   * @struct CarlaClockPublisherImpl
   * @brief CarlaClockPublisher的内部实现结构体，用于隐藏实现细节。
   */
  struct CarlaClockPublisherImpl;
  /**
   * @class CarlaClockPublisher
   * @brief 用于发布CARLA时钟信息的ROS 2发布者类。
   *
   * 此类继承自CarlaPublisher，并专门用于发布时钟信息（如秒和纳秒）。
   */
  class CarlaClockPublisher : public CarlaPublisher {
    public:
        /**
       * @brief 构造函数，初始化CarlaClockPublisher。
       *
       * @param ros_name ROS 2节点的名称，默认为空字符串。
       * @param parent 父节点的名称，默认为空字符串。
       */
      CarlaClockPublisher(const char* ros_name = "", const char* parent = "");
      /**
       * @brief 析构函数，清理资源并释放内部实现。
       */
      ~CarlaClockPublisher();
      /**
       * @brief 拷贝构造函数。
       *
       * @param other 要拷贝的CarlaClockPublisher对象。
       */
      CarlaClockPublisher(const CarlaClockPublisher&);
      /**
       * @brief 赋值运算符。
       *
       * @param other 要赋值的CarlaClockPublisher对象。
       * @return 引用到当前对象。
       */
      CarlaClockPublisher& operator=(const CarlaClockPublisher&);
      /**
       * @brief 移动构造函数。
       *
       * @param other 要移动的CarlaClockPublisher对象。
       */
      CarlaClockPublisher(CarlaClockPublisher&&);
      /**
       * @brief 移动赋值运算符。
       *
       * @param other 要移动赋值的CarlaClockPublisher对象。
       * @return 引用到当前对象。
       */
      CarlaClockPublisher& operator=(CarlaClockPublisher&&);
      /**
       * @brief 初始化发布者。
       *
       * @return 如果初始化成功，则返回true；否则返回false。
       */
      bool Init();
      /**
       * @brief 发布时钟信息。
       *
       * @return 如果发布成功，则返回true；否则返回false。
       */
      bool Publish();
      /**
      * @brief 设置要发布的时钟数据。
      *
      * @param sec 秒数。
      * @param nanosec 纳秒数。
      */
      void SetData(int32_t sec, uint32_t nanosec);
      /**
       * @brief 重写基类中的type函数，返回此发布者的类型。
       *
       * @return 指向类型名称的常量字符指针，此处为"clock"。
       */
      const char* type() const override { return "clock"; }

    private:
        /**
       * @brief 指向内部实现结构体的智能指针。
       *
       * 使用shared_ptr来管理CarlaClockPublisherImpl的生命周期，允许多个对象共享同一个实现。
       */
      std::shared_ptr<CarlaClockPublisherImpl> _impl;
  };
}
}
