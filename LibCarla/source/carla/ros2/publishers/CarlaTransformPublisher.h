// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
#define _GLIBCXX_USE_CXX11_ABI 0

#include <memory>// 引入智能指针std::shared_ptr
#include <vector>// 引入STL容器std::vector

#include "CarlaPublisher.h"// 引入CarlaPublisher类的定义
/**
 * @namespace carla
 * @brief CARLA项目的命名空间。
 */
namespace carla {
    /**
 * @namespace ros2
 * @brief ROS2相关的命名空间，用于区分CARLA与ROS2之间的交互部分。
 */
namespace ros2 {
    /**
     * @struct CarlaTransformPublisherImpl
     * @brief CarlaTransformPublisher类的内部实现结构体，采用Pimpl（Pointer to IMPLementation）惯用法隐藏实现细节。
     */
  struct CarlaTransformPublisherImpl;
  /**
     * @class CarlaTransformPublisher
     * @brief CarlaTransformPublisher类继承自CarlaPublisher，用于在CARLA中发布变换信息到ROS2。
     */
  class CarlaTransformPublisher : public CarlaPublisher {
    public:
        /**
            * @brief 构造函数，初始化CarlaTransformPublisher对象。
            * @param ros_name ROS2节点的名称，默认为空字符串。
            * @param parent 父节点的名称，默认为空字符串。
            */
      CarlaTransformPublisher(const char* ros_name = "", const char* parent = "");
      /**
             * @brief 析构函数，释放CarlaTransformPublisher对象占用的资源。
             */
      ~CarlaTransformPublisher();
      /**
             * @brief 拷贝构造函数，通过拷贝另一个CarlaTransformPublisher对象来初始化新对象。
             * @param other 要拷贝的CarlaTransformPublisher对象。
             */
      CarlaTransformPublisher(const CarlaTransformPublisher&);
      /**
             * @brief 拷贝赋值运算符，将另一个CarlaTransformPublisher对象的内容复制到当前对象。
             * @param other 要复制的CarlaTransformPublisher对象。
             * @return 返回当前对象的引用。
             */
      CarlaTransformPublisher& operator=(const CarlaTransformPublisher&);
      /**
            * @brief 移动构造函数，通过移动另一个CarlaTransformPublisher对象来初始化新对象。
            * @param other 要移动的CarlaTransformPublisher对象。
            */
      CarlaTransformPublisher(CarlaTransformPublisher&&);
      /**
            * @brief 移动赋值运算符，将另一个CarlaTransformPublisher对象的内容移动到当前对象。
            * @param other 要移动的CarlaTransformPublisher对象。
            * @return 返回当前对象的引用。
            */
      CarlaTransformPublisher& operator=(CarlaTransformPublisher&&);
      /**
             * @brief 初始化函数，用于初始化CarlaTransformPublisher对象。
             * @return 如果初始化成功，则返回true；否则返回false。
             */
      bool Init();
      /**
             * @brief 发布函数，用于将变换信息发布到ROS2。
             * @return 如果发布成功，则返回true；否则返回false。
             */
      bool Publish();
      /**
             * @brief 设置变换数据的函数。
             * @param seconds 变换的时间戳，秒部分。
             * @param nanoseconds 变换的时间戳，纳秒部分。
             * @param translation 变换的平移部分，数组包含x, y, z三个分量。
             * @param rotation 变换的旋转部分，数组包含roll, pitch, yaw三个分量。
             */
      void SetData(int32_t seconds, uint32_t nanoseconds, const float* translation, const float* rotation);
      /**
             * @brief 重写type函数，返回当前对象的类型字符串。
             * @return 返回"transform"字符串，表示当前对象发布的消息类型为变换信息。
             */
      const char* type() const override { return "transform"; }

    private:
        /**
             * @brief 指向CarlaTransformPublisherImpl的智能指针，用于隐藏实现细节。
             */
      std::shared_ptr<CarlaTransformPublisherImpl> _impl;
  };
}
}
