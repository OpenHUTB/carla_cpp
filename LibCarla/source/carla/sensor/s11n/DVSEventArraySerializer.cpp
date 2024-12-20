// Copyright (c) 2020 Robotics and Perception Group (GPR)
// University of Zurich and ETH Zurich
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/sensor/s11n/DVSEventArraySerializer.h"
#include "carla/sensor/data/DVSEventArray.h"

// 命名空间 carla
namespace carla {
  // 命名空间 sensor
  namespace sensor {
    // 命名空间 s11n
    namespace s11n {

      // 类 DVSEventArraySerializer 的成员函数 Deserialize，接收一个右值引用的 RawData 作为参数
      SharedPtr<SensorData> DVSEventArraySerializer::Deserialize(RawData &&data) {
        // 创建一个 SharedPtr<data::DVSEventArray> 智能指针，
        // 并使用 new 创建一个新的 data::DVSEventArray 对象，同时将 data 数据通过 std::move 转移给新对象
        auto events_array = SharedPtr<data::DVSEventArray>(new data::DVSEventArray{std::move(data)});

        // 将创建的 data::DVSEventArray 的智能指针作为 SensorData 类型的智能指针返回
        return events_array;
      }

    } // namespace s11n
  } // namespace sensor
} // namespace carla
