// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
// 预处理指令，确保该头文件在整个项目编译过程中只会被包含一次
#include "carla/Debug.h"
// 引入carla项目中与调试相关的头文件
#include "carla/rpc/Location.h"
// 引入carla项目里rpc模块下关于Location（通常用于表示位置信息）的头文件
#include "carla/sensor/data/Array.h"
// 引入carla项目中传感器数据里关于Array（数组相关）的头文件
#include "carla/sensor/s11n/LidarSerializer.h"
// 引入carla项目中传感器序列化相关模块（s11n）里关于激光雷达序列化（LidarSerializer）的头文件

namespace carla {
namespace sensor {
namespace data {

  /// Measurement produced by a Lidar. Consists of an array of 3D points plus
  /// some extra meta-information about the Lidar.
  class LidarMeasurement : public Array<data::LidarDetection>  {
    static_assert(sizeof(data::LidarDetection) == 4u * sizeof(float), "Location size missmatch");
// 使用静态断言（static_assert）检查data::LidarDetection类型的大小是否等于4倍float类型的大小，如果不相等则输出给定的错误信息（"Location size missmatch"）

    using Super = Array<data::LidarDetection>;
 // 使用using关键字定义一个类型别名Super

  protected:
    using Serializer = s11n::LidarSerializer;
// 使用using关键字定义一个类型别名Serializer

    friend Serializer;
// 声明s11n::LidarSerializer类为友元类，意味着LidarSerializer类可以访问LidarMeasurement类的私有和保护成员

    explicit LidarMeasurement(RawData &&data)
      : Super(std::move(data), [](const RawData &d) {
      return Serializer::GetHeaderOffset(d);
    }) {}
// 定义构造函数，接收一个右值引用类型的RawData（传感器原始数据）参数。
// 传递给父类构造函数的参数除了移动语义传入的原始数据（std::move(data)）外，还有一个匿名函数（lambda表达式）
 // 这个匿名函数以RawData类型的引用作为参数，在函数体中调用Serializer（即LidarSerializer类）的GetHeaderOffset函数
  private:

    auto GetHeader() const {
      return Serializer::DeserializeHeader(Super::GetRawData());
    }
// 定义一个常成员函数GetHeader
// 具体做法是先通过调用父类（Super）的GetRawData函数获取原始数据，然后将其作为参数传递给Serializer（LidarSerializer类）的DeserializeHeader函数
// 利用该函数进行反序列化操作来获取头部信息并返回，这样后续其他成员函数就能基于这个头部信息进行相应的操作了。

  public:
    /// Horizontal angle of the Lidar at the time of the measurement.
    auto GetHorizontalAngle() const {
      return GetHeader().GetHorizontalAngle();
    }
 // 定义一个常成员函数GetHorizontalAngle
 // 它通过先调用GetHeader函数获取头部信息，然后再调用头部信息对象的GetHorizontalAngle函数来获取具体的水平角度值并返回。

    /// Number of channels of the Lidar.
    auto GetChannelCount() const {
      return GetHeader().GetChannelCount();
    }
// 定义一个常成员函数GetChannelCount
// 同样是先获取头部信息（调用GetHeader函数），再调用头部信息对象的GetChannelCount函数来获取具体的通道数量值并返回。

    /// Retrieve the number of points that @a channel generated. Points are
    /// sorted by channel, so this method allows to identify the channel that
    /// generated each point.
    auto GetPointCount(size_t channel) const {
      return GetHeader().GetPointCount(channel);
    }
// 定义一个常成员函数GetPointCount，用于获取指定通道（由参数channel指定）所产生的点数信息。
 // 还是先获取头部信息，然后调用头部信息对象的GetPointCount函数，并传入指定的通道索引（channel）来获取对应通道的点数并返回
 // 由于点是按照通道进行排序的，所以这个函数可以帮助确定每个点是由哪个通道生成的。

  };

} // namespace data
} // namespace sensor
} // namespace carla
