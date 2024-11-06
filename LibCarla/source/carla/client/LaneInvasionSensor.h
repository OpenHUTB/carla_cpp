// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
/**
 * @file LaneInvasionSensor.h
 * @brief 包含LaneInvasionSensor类的声明，这是一个继承自ClientSideSensor的类，用于检测车道入侵。
 */
#include "carla/client/ClientSideSensor.h"

#include <atomic>
 /**
  * @namespace carla
  * @brief CARLA模拟器的主命名空间。
  */
namespace carla {
    /**
 * @namespace client
 * @brief 包含CARLA客户端相关类和函数的命名空间。
 */
namespace client {
    /**
   * @class Map
   * @brief 地图类的前向声明，用于在LaneInvasionSensor类中可能的引用。
   */
  class Map;
  /**
   * @class Vehicle
   * @brief 车辆类的前向声明，用于在LaneInvasionSensor类中可能的引用。
   */
  class Vehicle;
  /**
   * @class LaneInvasionSensor
   * @brief LaneInvasionSensor类是一个检测车道入侵的传感器，继承自ClientSideSensor。
   */
  class LaneInvasionSensor final : public ClientSideSensor {
  public:
      /**
           * @brief 使用基类ClientSideSensor的构造函数。
           */
    using ClientSideSensor::ClientSideSensor;
    /**
     * @brief 析构函数，用于清理LaneInvasionSensor对象。
     */
    ~LaneInvasionSensor();

    /**
     * @brief 注册一个回调，每次收到新的车道入侵测量值时执行。
     *
     * @param callback 回调函数，当接收到新的测量值时会被调用。
     * @warning 在已在监听的传感器上调用此函数会替换先前设置的回调，从而窃取其数据流。
     * 请注意，多个传感器实例（即使在不同的进程中）可能指向模拟器中的同一物理传感器。
     */
    void Listen(CallbackFunctionType callback) override;

    /**
      * @brief 停止监听新的车道入侵测量结果。
      */
    void Stop() override;

    /**
     * @brief 返回此传感器实例当前是否正在监听模拟器中的相关传感器。
     *
     * @return 如果正在监听，则返回true；否则返回false。
     */
    bool IsListening() const override {
      return _callback_id != 0u;
    }

  private:
      /**
           * @brief 原子性的回调ID，用于标识当前设置的回调。
           * 当_callback_id不为0时，表示正在监听。
           */
    std::atomic_size_t _callback_id{0u};
  };

} // namespace client
} // namespace carla
