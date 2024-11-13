// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Debug.h"
#include "carla/sensor/data/Array.h"
#include "carla/sensor/s11n/RadarSerializer.h"
#include "carla/sensor/data/RadarData.h"

namespace carla {
namespace sensor {
namespace data {

  //由雷达探测出来的雷达测量结果类。由一个RadarDetection数组的数据组成
  ///一个RadarDetection包含四个浮点变量: 速度, 朝向方位角, 海拔以及深度
  class RadarMeasurement : public Array<data::RadarDetection> {
    using Super = Array<data::RadarDetection>;
  protected:

    using Serializer = s11n::RadarSerializer;

    friend Serializer;

    explicit RadarMeasurement(RawData &&data)
      : Super(0u, std::move(data)) {}

  public:

    Super::size_type GetDetectionAmount() const {
      return Super::size();
    }
  };

} // namespace data
} // namespace sensor
} // namespace carla
