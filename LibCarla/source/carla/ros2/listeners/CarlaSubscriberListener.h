// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
#define _GLIBCXX_USE_CXX11_ABI 0

#include <memory>
/**
 * @namespace carla::ros2
 * @brief 命名空间carla::ros2，包含了与CARLA和ROS 2集成相关的类和函数。
 */
namespace carla {
namespace ros2 {
    /**
       * @class CarlaSubscriberListenerImpl
       * @brief CarlaSubscriberListener的内部实现类，用于隐藏实现细节。
       */
  class CarlaSubscriberListenerImpl;
  /**
   * @class CarlaEgoVehicleControlSubscriber
   * @brief 用于订阅和控制CARLA中自车（ego vehicle）的类
   * 此类可能包含与ROS 2节点相关的逻辑，用于接收来自CARLA的自车控制命令。
   */
  class CarlaEgoVehicleControlSubscriber;
  /**
   * @class CarlaSubscriberListener
   * @brief Carla订阅者监听器类，用于处理与ROS 2订阅者相关的事件。
   *
   * 此类使用Pimpl（Pointer to IMPLementation）惯用法来隐藏实现细节，
   * 通过一个指向CarlaSubscriberListenerImpl的unique_ptr来实现。
   */
  class CarlaSubscriberListener {
    public:
        /**
       * @brief 构造函数，初始化CarlaSubscriberListener并设置其所有者。
       *
       * @param owner 指向CarlaEgoVehicleControlSubscriber的指针，作为此监听器的所有者。
       */
      CarlaSubscriberListener(CarlaEgoVehicleControlSubscriber* owner);
      /**
      * @brief 析构函数，清理资源并释放内部实现。
      */
      ~CarlaSubscriberListener();
      /**
       * @brief 禁用拷贝构造函数。
       *
       * 防止类对象被拷贝，确保资源的唯一性和安全性。
       */
      CarlaSubscriberListener(const CarlaSubscriberListener&) = delete;
      /**
       * @brief 禁用赋值运算符。
       *
       * 防止类对象被赋值，确保资源的唯一性和安全性。
       */
      CarlaSubscriberListener& operator=(const CarlaSubscriberListener&) = delete;
      /**
       * @brief 禁用移动构造函数。
       *
       * 防止类对象被移动，确保资源的唯一性和安全性。
       */
      CarlaSubscriberListener(CarlaSubscriberListener&&) = delete;
      /**
       * @brief 禁用移动赋值运算符。
       *
       * 防止类对象被移动赋值，确保资源的唯一性和安全性。
       */
      CarlaSubscriberListener& operator=(CarlaSubscriberListener&&) = delete;
      /**
      * @brief 设置此监听器的所有者。
      *
      * @param owner 指向新的CarlaEgoVehicleControlSubscriber所有者的指针。
      */

      void SetOwner(CarlaEgoVehicleControlSubscriber* owner);
      /**
       * @brief 指向内部实现类的指针。
       *
       * 使用unique_ptr来管理CarlaSubscriberListenerImpl的生命周期，确保资源的正确释放。
       */
      std::unique_ptr<CarlaSubscriberListenerImpl> _impl;
  };
}
}
