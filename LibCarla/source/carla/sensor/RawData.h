// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once // 防止头文件重复包含

#include "carla/Buffer.h" // 引入Buffer类的头文件
#include "carla/sensor/s11n/SensorHeaderSerializer.h" // 引入SensorHeaderSerializer类的头文件
#include "carla/ros2/ROS2.h" // 引入ROS2类的头文件

#include <cstdint> // 引入cstdint库以使用固定宽度整数类型
#include <iterator> // 引入iterator库以使用迭代器

namespace carla { // carla命名空间
namespace sensor { // sensor命名空间

  /// 包装一个传感器生成的原始数据以及一些有用的元信息。
  class RawData {
   using HeaderSerializer = s11n::SensorHeaderSerializer; // 定义HeaderSerializer为SensorHeaderSerializer的别名
  private:

    // 获取头部信息
    const auto &GetHeader() const {
     return HeaderSerializer::Deserialize(_buffer); // 反序列化缓冲区以获取头部信息
    }

  public:

    /// 生成数据的传感器的类型ID。
    uint64_t GetSensorTypeId() const {
     return GetHeader().sensor_type; // 返回传感器类型ID
    }

    /// 生成数据时的帧计数。
    uint64_t GetFrame() const {
     return GetHeader().frame; // 返回帧计数
    }

    /// 生成数据时的时间戳。
    double GetTimestamp() const {
     return GetHeader().timestamp; // 返回时间戳
    }

    /// 生成数据时的传感器变换信息。
    const rpc::Transform &GetSensorTransform() const {
     return GetHeader().sensor_transform; // 返回传感器变换信息
    }

    /// 指向传感器生成的数据的开始迭代器。
    auto begin() noexcept {
     return _buffer.begin() + HeaderSerializer::header_offset; // 返回数据开始位置的迭代器
    }

    /// @copydoc begin() 的常量版本
    auto begin() const noexcept {
     return _buffer.begin() + HeaderSerializer::header_offset; // 返回数据开始位置的迭代器
    }

    /// 指向传感器生成的数据的结束迭代器。
    auto end() noexcept {
     return _buffer.end(); // 返回数据结束位置的迭代器
    }

    /// @copydoc end() 的常量版本
    auto end() const noexcept {
     return _buffer.end(); // 返回数据结束位置的迭代器
    }

    /// 检索指向包含传感器生成的数据的内存的指针。
    auto data() noexcept {
      return begin(); // 返回数据起始地址
    }

    /// @copydoc data() 的常量版本
    auto data() const noexcept {
      return begin(); // 返回数据起始地址
    }

    /// 传感器生成的数据的字节大小。
    size_t size() const {
      DEBUG_ASSERT(std::distance(begin(), end()) >= 0); // 确保数据大小非负
      return static_cast<size_t>(std::distance(begin(), end())); // 返回数据大小
    }

  private:

    template <typename... Items>
    friend class CompositeSerializer; // 允许CompositeSerializer类访问私有成员
    friend class carla::ros2::ROS2; // 允许carla::ros2::ROS2类访问私有成员

    // 构造函数，接受一个Buffer对象并移动它
    RawData(Buffer &&buffer) : _buffer(std::move(buffer)) {}

    Buffer _buffer; // 存储传感器数据的缓冲区
  };

} // namespace sensor
} // namespace carla
