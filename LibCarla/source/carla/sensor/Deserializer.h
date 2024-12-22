// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
// 这是一个编译预处理指令，作用是保证该头文件在一个编译单元中只会被包含（引入）一次，避免重复定义等问题。

#include "carla/Buffer.h"  // 包含 "carla/Buffer.h" 头文件，推测这里的 Buffer 类可能用于处理数据缓冲区相关操作，比如存储传感器等产生的数据。

#include "carla/Memory.h"
// 包含 "carla/Memory.h" 头文件，通常这个头文件会涉及内存管理相关的功能，比如智能指针等类型的定义，可能会在后续代码中用于管理对象的生命周期等情况。

namespace carla {  // 开始定义名为 carla 的命名空间，用于对相关的类型、函数等进行逻辑分组，避免命名冲突，将代码组织在一个独立的作用域内。
namespace sensor {
// 前置声明SensorData类
  class SensorData;
 // 前置声明一个名为 SensorData 的类，告诉编译器存在这样一个类，但具体的定义在后续或者其他地方，这样可以在当前代码中使用该类的指针或者引用等，解决编译顺序导致的找不到类型定义的问题。

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
