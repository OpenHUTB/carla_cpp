// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/sensor/s11n/GBufferUint8Serializer.h" // 包含 GBufferUint8Serializer 的定义

#include "carla/sensor/data/Image.h" // 包含 Image 数据类型的定义

namespace carla {
namespace sensor {
namespace s11n {

  // 从原始传感器数据反序列化出图像数据
  SharedPtr<SensorData> GBufferUint8Serializer::Deserialize(RawData &&data) {
    // 使用输入的原始数据构造一个新的 Image 对象
    // 通过智能指针 SharedPtr 管理对象的生命周期
    auto image = SharedPtr<data::Image>(new data::Image{std::move(data)});
    return image; // 返回封装了图像数据的智能指针
  }

} // namespace s11n
} // namespace sensor
} // namespace carla
