// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <cstdint>
#include <vector>
#include <cstdio>

namespace carla {

namespace ros2 { // carla命名空间下的ros2子命名空间
  class ROS2; // 声明ROS2类
}

namespace sensor { // carla命名空间下的sensor子命名空间，用于定义传感器相关的数据和序列化

namespace s11n { // sensor命名空间下的s11n子命名空间，用于定义序列化相关的类
  class RadarSerializer; // 声明RadarSerializer类，用于雷达数据的序列化
}

namespace data {

  struct RadarDetection { // 定义RadarDetection结构体，用于存储雷达检测到的对象信息
    float velocity; // 速度，单位为米每秒（m/s）
    float azimuth;  // 方位角，单位为弧度（rad），表示目标相对于正北方向的角度
    float altitude; // 高度角，单位为弧度（rad），表示目标相对于地平面的高度
    float depth;    // 深度，单位为米（m），表示目标距离雷达的距离
  };

  class RadarData { // 定义RadarData类，用于存储雷达数据
    static_assert(sizeof(float) == sizeof(uint32_t), "Invalid float size"); // 静态断言，确保float类型的大小与uint32_t一致
    static_assert(sizeof(float) * 4 == sizeof(RadarDetection), "Invalid RadarDetection size"); // 静态断言，确保RadarDetection结构体的大小为4个float的大小

  public:
    explicit RadarData() = default; // RadarData类的显式默认构造函数

    constexpr static auto detection_size = sizeof(RadarDetection); // RadarData类中用于获取RadarDetection结构体大小的静态常量

    RadarData &operator=(RadarData &&) = default;  // RadarData类的移动赋值运算符，使用默认实现

    /// 设置雷达数据的新分辨率
    ///分配或释放内存空间，如果需要的话
    ///
    ///@warning 这个操作成本较高，不应该每帧调用
    void SetResolution(uint32_t resolution) {
       // 清空_detections向量，并将未使用的容量缩减到0
      _detections.clear();
      _detections.shrink_to_fit();
      // 为_detections向量预留新的容量
      _detections.reserve(resolution);
    }

    // 返回当前检测的数量
    size_t GetDetectionCount() const {
      return _detections.size();
    }

    /// 删除当前的检测数据
    /// 这个操作不会改变分辨率或已分配的内存
    void Reset() {
      _detections.clear();
    }

    /// 添加一个新的检测对象到雷达数据中
    void WriteDetection(RadarDetection detection) {
      _detections.push_back(detection);
    }

  private:
    std::vector<RadarDetection> _detections; ///用于存储RadarDetection对象的动态数组

  friend class s11n::RadarSerializer; // 声明RadarSerializer类和ROS2类为RadarData类的友元类
  friend class carla::ros2::ROS2;
  };

} // namespace s11n
} // namespace sensor
} // namespace carla
