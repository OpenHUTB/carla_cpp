// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once // 指示这个头文件只应该被包含一次

#include "carla/Debug.h" // 包含CARLA的调试功能
#include "carla/rpc/Location.h" // 包含CARLA的RPC位置定义
#include "carla/sensor/data/Array.h" // 包含CARLA传感器数据数组定义
#include "carla/sensor/s11n/SemanticLidarSerializer.h"

namespace carla {
namespace sensor {
namespace data {

  /// 由激光雷达产生的测量结果。包括一系列3D点以及一些关于激光雷达的额外元信息。
  class SemanticLidarMeasurement : public Array<data::SemanticLidarDetection>  { // 定义一个类，继承自Array，用于存储和处理语义Lidar的测量数据
    static_assert(sizeof(data::SemanticLidarDetection) == 6u * sizeof(float), "SemanticLidarDetection size missmatch"); // 确保SemanticLidarDetection的大小正确
    using Super = Array<data::SemanticLidarDetection>; // 使用Super作为基类的别名

  protected:
    using Serializer = s11n::SemanticLidarSerializer; // 使用using声明Serializer为SemanticLidarSerializer的别名，方便后续代码中使用

    friend Serializer; // 允许Serializer访问SemanticLidarMeasurement的私有成员

    explicit SemanticLidarMeasurement(RawData &&data) // 构造函数，接受原始数据
      : Super(std::move(data), [](const RawData &d) { // 调用基类的构造函数
      return Serializer::GetHeaderOffset(d); // 获取数据头部的偏移量
    }) {}

  private:

    auto GetHeader() const { // 私有成员函数，用于获取序列化数据的头部信息
      return Serializer::DeserializeHeader(Super::GetRawData()); // 调用Serializer的静态方法DeserializeHeader，从基类的RawData中反序列化头部信息
    }

  public:

    /// 测量时激光雷达的水平角度。
    auto GetHorizontalAngle() const { // 获取Lidar的水平角度
      return GetHeader().GetHorizontalAngle(); // 从头部信息中获取水平角度
    }

    /// 激光雷达的通道数量。
    auto GetChannelCount() const { // 获取Lidar的通道数
      return GetHeader().GetChannelCount(); // 从头部信息中获取通道数
    }

    /// 获取由特定通道生成的点的数量。点是按通道排序的，因此这个方法可以用来识别生成每个点的通道。
    ///
    auto GetPointCount(size_t channel) const { // 获取特定通道生成的点的数量
      return GetHeader().GetPointCount(channel); // 从头部信息中获取特定通道的点的数量
    }
  };

} // namespace data
} // namespace sensor
} // namespace carla
