// Copyright (c) 2020 Robotics and Perception Group (GPR)
// University of Zurich and ETH Zurich
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Memory.h"
#include "carla/sensor/RawData.h"
#include "carla/sensor/data/DVSEvent.h"

#include <cstdint>
#include <cstring>

namespace carla {
namespace sensor {

  class SensorData;

namespace s11n {

  /// 序列化 DVS 摄像机传感器生成的事件数组。
  class DVSEventArraySerializer {
  public:

#pragma pack(push, 1)
    struct DVSHeader {
      uint32_t width;
      uint32_t height;
      float fov_angle;
    };
#pragma pack(pop)

    constexpr static auto header_offset = sizeof(DVSHeader);
    using DVSEventArray = std::vector<data::DVSEvent>;

    static const DVSHeader &DeserializeHeader(const RawData &data) {
      return *reinterpret_cast<const DVSHeader *>(data.begin());
    }

    template <typename Sensor>
    static Buffer Serialize(const Sensor &sensor, const DVSEventArray &events, Buffer &&output);

    static SharedPtr<SensorData> Deserialize(RawData &&data);
  };

  template <typename Sensor>
  inline Buffer DVSEventArraySerializer::Serialize(const Sensor &sensor, const DVSEventArray &events, Buffer &&output) {
    DEBUG_ASSERT(events.size() > sizeof(DVSHeader));
    DVSHeader header = {
      sensor.GetImageWidth(),
      sensor.GetImageHeight(),
      sensor.GetFOVAngle(),
    };

    /// 重置输出缓冲区。
    output.reset(sizeof(DVSHeader) + (events.size() * sizeof(data::DVSEvent)));

    /// 指向缓冲区数据的指针。
    unsigned char *it = output.data();

    /// 将表头复制到输出缓冲区中。
    std::memcpy(it, reinterpret_cast<const void *>(&header), sizeof(header));
    it += sizeof(DVSHeader);

    /// 将结果复制到输出缓冲区中。
    for (auto e : events) {
      std::memcpy(it, reinterpret_cast<const void *>(&e), sizeof(data::DVSEvent));
      it += sizeof(data::DVSEvent);
    }
    return std::move(output);
  }

} // namespace s11n
} // namespace sensor
} // namespace carla
