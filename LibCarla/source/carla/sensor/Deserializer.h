// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Buffer.h"
#include "carla/Memory.h"
// 定义在carla命名空间下的sensor子命名空间
namespace carla {
namespace sensor {
// 前置声明SensorData类
  class SensorData;

  /// @brief 反序列化器（Deserializer）类，用于将包含传感器生成数据的缓冲区（Buffer）进行反序列化操作，
  ///        并创建合适的SensorData类实例来承载传感器的测量数据。
  /// @details 该类对SensorRegistry进行了封装，目的是避免引入所有的序列化器以及SensorData相关的各个类，
  ///          以一种更简洁、模块化的方式来处理传感器数据的反序列化过程。
  class Deserializer {
  public:
 /// @brief 静态函数，用于执行反序列化操作。
 /// @param buffer 以右值引用方式传入的Buffer对象，其中包含了需要反序列化的传感器数据。
/// @return 返回一个指向SensorData类的智能指针（SharedPtr），如果反序列化成功，该指针指向包含反序列化后传感器测量数据的对象；
///如果反序列化出现问题，根据具体实现可能返回空指针等表示失败的情况。
    static SharedPtr<SensorData> Deserialize(Buffer &&buffer);
  };

} // namespace sensor
} // namespace carla
