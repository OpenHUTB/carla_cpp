// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/client/Sensor.h"    // 引入CARLA客户端传感器的头文件



namespace carla {         // 定义carla命名空间
namespace client {    // 定义client命名空间



  class ClientSideSensor : public Sensor {   // 定义ClientSideSensor类，继承自Sensor类
  public:

    using Sensor::Sensor;    // 使用基类Sensor的构造函数

  };  
   // 结束ClientSideSensor类定义

} // namespace client  
} // namespace carla  
