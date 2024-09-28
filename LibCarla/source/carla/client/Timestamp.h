// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <cstdint>

namespace carla {
namespace client {

  class Timestamp {
  public:

    Timestamp() = default;

    Timestamp(
        std::size_t in_frame,
        double in_elapsed_seconds,
        double in_delta_seconds,
        double in_platform_timestamp)
      : frame(in_frame),
        elapsed_seconds(in_elapsed_seconds),
        delta_seconds(in_delta_seconds),
        platform_timestamp(in_platform_timestamp) {}

    /// 自模拟器启动以来经过的帧数。
    std::size_t frame = 0u;

    /// 模拟自当前情境开始以来经过的秒数。
    double elapsed_seconds = 0.0;

    /// 模拟自上一帧以来经过的秒数。
    double delta_seconds = 0.0;

    /// 进行此测量的帧的时间戳，以操作系统给出的秒数为单位。
    double platform_timestamp = 0.0;

    bool operator==(const Timestamp &rhs) const {
      return frame == rhs.frame;
    }

    bool operator!=(const Timestamp &rhs) const {
      return !(*this == rhs);
    }
  };

} // namespace client
} // namespace carla


namespace std {
/**
 * \brief 标准输出流操作
 *
 * \param[in/out] out 要写入的输出流
 * \param[in] timestamp 输出流的时间戳
 *
 * \returns The stream object.
 *
 */
inline std::ostream &operator<<(std::ostream &out, const ::carla::client::Timestamp &timestamp) {
  out << "Timestamp(frame=" << std::to_string(timestamp.frame)
      << ",elapsed_seconds=" << std::to_string(timestamp.elapsed_seconds)
      << ",delta_seconds=" << std::to_string(timestamp.delta_seconds)
      << ",platform_timestamp=" << std::to_string(timestamp.platform_timestamp) << ')';
  return out;
}
} // namespace std
