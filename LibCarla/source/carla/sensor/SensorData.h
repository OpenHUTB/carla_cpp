// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once  // 防止头文件被重复包含

#include "carla/Memory.h"  // 包含内存管理相关的头文件
#include "carla/NonCopyable.h"  // 包含不可复制类的头文件
#include "carla/sensor/RawData.h"  // 包含原始数据相关的头文件

/// @todo 这个不应该暴露在这个命名空间中。
#include "carla/client/detail/EpisodeProxy.h"  // 包含剧集代理的头文件

namespace carla {  // carla 命名空间
namespace sensor {  // sensor 命名空间

  /// 所有传感器生成数据的对象的基类
  class SensorData
    : public EnableSharedFromThis<SensorData>,  // 允许共享指针
      private NonCopyable {  // 不可复制的类
  protected:

    // 构造函数，初始化帧数、时间戳和传感器变换
    SensorData(size_t frame, double timestamp, const rpc::Transform &sensor_transform)
      : _frame(frame),  // 初始化帧数
        _timestamp(timestamp),  // 初始化时间戳
        _sensor_transform(sensor_transform) {}  // 初始化传感器变换

    // 通过原始数据构造 SensorData 对象
    explicit SensorData(const RawData &data)
      : SensorData(data.GetFrame(), data.GetTimestamp(), data.GetSensorTransform()) {}  // 调用上面的构造函数

  public:

    virtual ~SensorData() = default;  // 虚析构函数，使用默认实现

    /// 获取数据生成时的帧计数
    size_t GetFrame() const {
      return _frame;  // 返回帧数
    }

    /// 获取数据生成时的仿真时间
    double GetTimestamp() const {
      return _timestamp;  // 返回时间戳
    }

    /// 获取数据生成时的传感器变换
    const rpc::Transform &GetSensorTransform() const {
      return _sensor_transform;  // 返回传感器变换
    }

  protected:

    // 获取与该数据相关的剧集
    const auto &GetEpisode() const {
      return _episode;  // 返回弱引用的剧集代理
    }

  private:

    /// @todo 这个不应该暴露在这个命名空间中。
    friend class client::detail::Simulator;  // 声明 Simulator 类为友元
    client::detail::WeakEpisodeProxy _episode;  // 剧集的弱引用代理

    const size_t _frame;  // 帧数
    const double _timestamp;  // 时间戳
    const rpc::Transform _sensor_transform;  // 传感器变换
  };

} // namespace sensor
} // namespace carla