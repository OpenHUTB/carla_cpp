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
   * @struct CarlaCollisionPublisherImpl
   * @brief CarlaCollisionPublisher的内部实现结构体，用于隐藏实现细节。
   *
   * 该结构体的完整定义在源文件中，这里仅声明了一个指向它的智能指针类型。
   */
  struct CarlaCollisionPublisherImpl;
  /**
   * @class CarlaCollisionPublisher
   * @brief 用于发布CARLA碰撞事件的ROS 2发布者类。
   *
   * 该类继承自CarlaPublisher，并专门用于发布碰撞事件信息，如碰撞时间、碰撞物体的ID和碰撞位置。
   */
  class CarlaCollisionPublisher : public CarlaPublisher {
    public:
        /**
       * @brief 构造函数，初始化CarlaCollisionPublisher。
       *
       * @param ros_name ROS 2节点的名称，默认为空字符串。
       * @param parent 父节点的名称，默认为空字符串。
       */
      CarlaCollisionPublisher(const char* ros_name = "", const char* parent = "");
      /**
       * @brief 析构函数，清理资源并释放内部实现。
       */
      ~CarlaCollisionPublisher();
      /**
       * @brief 拷贝构造函数。
       *
       * @param other 要拷贝的CarlaCollisionPublisher对象。
       */
      CarlaCollisionPublisher(const CarlaCollisionPublisher&);
      /**
       * @brief 赋值运算符。
       *
       * @param other 要赋值的CarlaCollisionPublisher对象。
       * @return 引用到当前对象。
       */
      CarlaCollisionPublisher& operator=(const CarlaCollisionPublisher&);
      /**
      * @brief 移动构造函数。
      *
      * @param other 要移动的CarlaCollisionPublisher对象。
      */
      CarlaCollisionPublisher(CarlaCollisionPublisher&&);
      /**
       * @brief 移动赋值运算符。
       *
       * @param other 要移动赋值的CarlaCollisionPublisher对象。
       * @return 引用到当前对象。
       */
      CarlaCollisionPublisher& operator=(CarlaCollisionPublisher&&);
      /**
       * @brief 初始化发布者。
       *
       * @return 如果初始化成功，则返回true；否则返回false。
       */
      bool Init();
      /**
       * @brief 发布碰撞事件信息。
       *
       * @return 如果发布成功，则返回true；否则返回false。
       */
      bool Publish();
      /**
       * @brief 设置要发布的碰撞事件数据。
       *
       * @param seconds 碰撞发生的时间（秒）。
       * @param nanoseconds 碰撞发生的时间（纳秒）。
       * @param actor_id 发生碰撞的物体的ID。
       * @param x 碰撞位置的X坐标。
       * @param y 碰撞位置的Y坐标。
       * @param z 碰撞位置的Z坐标。
       */
      void SetData(int32_t seconds, uint32_t nanoseconds, uint32_t actor_id, float x, float y, float z);
      /**
       * @brief 重写基类中的type函数，返回此发布者的类型。
       *
       * @return 指向类型名称的常量字符指针，此处为"collision"。
       */
      const char* type() const override { return "collision"; }

    private:
        /**
       * @brief 另一个版本的SetData函数，允许传递碰撞位置数据的向量。
       *
       * 这个函数是私有的，意味着它只能在CarlaCollisionPublisher类内部被调用。
       *
       * @param seconds 碰撞发生的时间（秒）。
       * @param nanoseconds 碰撞发生的时间（纳秒）。
       * @param actor_id 发生碰撞的物体的ID。
       * @param data 包含碰撞位置数据的向量（X, Y, Z）。
       */
      void SetData(int32_t seconds, uint32_t nanoseconds, uint32_t actor_id, std::vector<float>&& data);

    private:
        /**
       * @brief 指向内部实现结构体的智能指针。
       *
       * 使用shared_ptr来管理CarlaCollisionPublisherImpl的生命周期，允许多个对象共享同一个实现。
       */
      std::shared_ptr<CarlaCollisionPublisherImpl> _impl;
  };
}
}
