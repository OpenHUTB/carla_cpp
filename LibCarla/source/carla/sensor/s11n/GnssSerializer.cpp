// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/sensor/s11n/GnssSerializer.h" // 包含 GnssSerializer 的声明

#include "carla/sensor/data/GnssMeasurement.h" // 包含 GnssMeasurement 类的定义

namespace carla { // Carla 项目顶级命名空间
namespace sensor { // 包含与传感器相关的所有功能
namespace s11n { // 提供序列化和反序列化功能的命名空间

  /// @brief 从原始 GNSS 数据反序列化为 GnssMeasurement 对象
  /// @param data GNSS 原始数据，表示为 RawData 对象
  /// @return 返回一个智能指针，指向反序列化后的 GnssMeasurement 对象
  SharedPtr<SensorData> GnssSerializer::Deserialize(RawData &&data) {
    // 将传入的 RawData 数据封装为 GnssMeasurement 对象，并使用 SharedPtr 进行管理
    return SharedPtr<SensorData>(new data::GnssMeasurement(std::move(data)));
  }

} // namespace s11n
} // namespace sensor
} // namespace carla
