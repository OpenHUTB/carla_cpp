// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/client/Actor.h"

#include <functional>
/**
 * @namespace carla::sensor
 * @brief 包含传感器数据相关类和定义的命名空间。
 */
namespace carla {
namespace sensor { class SensorData; }
/**
 * @namespace carla::client
 * @brief 包含客户端相关类和定义的命名空间。
 */
namespace client {
    /**
  * @class Sensor
  * @brief 传感器基类，继承自Actor类。
  *
  * Sensor类是一个抽象基类，代表CARLA模拟器中的一个传感器。它提供了注册回调、停止监听和检查是否正在监听新数据的功能。
  */
  class Sensor : public Actor {
  public:
      /**
     * @typedef CallbackFunctionType
     * @brief 回调函数的类型别名，用于接收传感器数据。
     *
     * 这是一个std::function类型的别名，它接受一个指向sensor::SensorData的智能指针作为参数，并返回void。
     */
    using CallbackFunctionType = std::function<void(SharedPtr<sensor::SensorData>)>;
    /**
     * @brief 使用Actor的构造函数初始化Sensor对象。
     *
     * 这是通过使用using声明来继承Actor的构造函数。
     */
    using Actor::Actor;
    /**
     * @brief 注册一个回调，以便在每次收到新的测量值时执行。
     *
     * @param callback 回调函数，当传感器接收到新的数据时会被调用。
     *
     * 这个函数是虚函数，需要在子类中实现。
     */
    virtual void Listen(CallbackFunctionType callback) = 0;
    /**
     * @brief 停止监听新的测量结果。
     *
     * 这个函数是虚函数，需要在子类中实现。
     */
    virtual void Stop() = 0;
    /**
     * @brief 返回此传感器实例当前是否正在监听新数据。
     *
     * @return 如果传感器正在监听新数据，则返回true；否则返回false。
     *
     * 这个函数是虚函数，需要在子类中实现。
     */
    virtual bool IsListening() const = 0;
  };

} // namespace client
} // namespace carla
