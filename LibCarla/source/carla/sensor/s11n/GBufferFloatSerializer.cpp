// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/sensor/s11n/GBufferFloatSerializer.h"  // 包含 GBufferFloatSerializer 的类声明

#include "carla/sensor/data/Image.h"  // 包含图像数据的相关定义。

namespace carla {
namespace sensor {
namespace s11n {

  SharedPtr<SensorData> GBufferFloatSerializer::Deserialize(RawData &&data) {
    auto image = SharedPtr<data::FloatImage>(new data::FloatImage{std::move(data)}); // 创建一个新的 FloatImage 对象，将输入的 RawData 移动构造为 FloatImage 数据
    return image;// 返回 FloatImage 对象的智能指针，作为反序列化的结果
  }

} // namespace s11n
} // namespace sensor
} // namespace carla
