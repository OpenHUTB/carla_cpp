// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
//这个指令确保头文件只会被编译一次，避免重复包含（多重定义）的错误。

#include <cstdint>
//包含 C++ 标准库中的 头文件，提供精确宽度的整数类型

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
    //这个构造函数接受四个参数，初始化四个成员变量。
    //in_frame：当前帧的序号。
    //in_elapsed_seconds：模拟自当前情境开始以来的秒数。
    // in_delta_seconds：模拟自上一帧以来经过的秒数。
    // in_platform_timestamp：操作系统给出的秒数（例如，Unix 时间戳）。

    /// 自模拟器启动以来经过的帧数。
    std::size_t frame = 0u;

    /// 模拟自当前情境开始以来经过的秒数。
    double elapsed_seconds = 0.0;

    /// 模拟自上一帧以来经过的秒数。
    double delta_seconds = 0.0;

    /// 进行此测量的帧的时间戳，以操作系统给出的秒数为单位。
    double platform_timestamp = 0.0;

    // 判断两个时间戳是否相等（基于帧数）
    bool operator==(const Timestamp &rhs) const {
      return frame == rhs.frame;
    }

    // 判断两个时间戳是否不相等
    bool operator!=(const Timestamp &rhs) const {
      return !(*this == rhs);
      //这个==运算符重载用于比较两个Timestamp对象是否相等，判断依据是它们的frame数值是否相同。
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
 //时间戳的输出流操作符重载,用于打印时间戳对象
inline std::ostream &operator<<(std::ostream &out, const ::carla::client::Timestamp &timestamp) {
  out << "Timestamp(frame=" << std::to_string(timestamp.frame)
      << ",elapsed_seconds=" << std::to_string(timestamp.elapsed_seconds)
      << ",delta_seconds=" << std::to_string(timestamp.delta_seconds)
      << ",platform_timestamp=" << std::to_string(timestamp.platform_timestamp) << ')';
  return out;
}
} // namespace std
