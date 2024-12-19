// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/sensor/s11n/GBufferFloatSerializer.h"

#include "carla/sensor/data/Image.h"

namespace carla {
namespace sensor {
namespace s11n {

  SharedPtr<SensorData> GBufferFloatSerializer::Deserialize(RawData &&data) {
    auto image = SharedPtr<data::FloatImage>(new data::FloatImage{std::move(data)});
    return image;
  }

} // namespace s11n
} // namespace sensor
} // namespace carla
