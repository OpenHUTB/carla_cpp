// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/sensor/data/CollisionEvent.h"
#include "carla/sensor/s11n/CollisionEventSerializer.h"

// 命名空间 carla
namespace carla {
  // 命名空间 sensor
  namespace sensor {
    // 命名空间 s11n
    namespace s11n {

      // 函数 CollisionEventSerializer::Deserialize，接收一个右值引用的 RawData 作为参数
      SharedPtr<SensorData> CollisionEventSerializer::Deserialize(RawData &&data) {
        // 创建一个 SensorData 的共享指针，其指向一个新创建的 data::CollisionEvent 对象，使用 std::move 移动数据
        return SharedPtr<SensorData>(new data::CollisionEvent(std::move(data)));
      }

    } // namespace s11n
  } // namespace sensor
} // namespace carla
