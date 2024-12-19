// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/sensor/s11n/EpisodeStateSerializer.h" // 包含 EpisodeStateSerializer 的定义

#include "carla/sensor/data/RawEpisodeState.h" // 包含 RawEpisodeState 数据类型的定义

namespace carla {
namespace sensor {
namespace s11n {
  //从原始传感器数据反序列化出场景状态数据
  SharedPtr<SensorData> EpisodeStateSerializer::Deserialize(RawData &&data) {
    // 将输入的原始数据封装到一个新的 RawEpisodeState 对象中
    // 使用智能指针 SharedPtr 管理对象的生命周期
    return SharedPtr<data::RawEpisodeState>(new data::RawEpisodeState{std::move(data)});
  }

} // namespace s11n
} // namespace sensor
} // namespace carla
