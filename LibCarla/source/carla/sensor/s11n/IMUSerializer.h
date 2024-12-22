// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Buffer.h"
#include "carla/Memory.h"
#include "carla/sensor/RawData.h"

namespace carla {
namespace sensor {

  class SensorData;

namespace s11n {

  class IMUSerializer
  {
  public:

    struct Data {

      geom::Vector3D accelerometer;

      geom::Vector3D gyroscope;

      float compass;
// 使用MSGPACK_DEFINE_ARRAY宏来定义一个数组相关的结构，这里涉及到加速度计（accelerometer）、陀螺仪（gyroscope）和罗盘（compass）相关内容
// 具体的宏展开等行为取决于MSGPACK_DEFINE_ARRAY的定义，可能用于序列化/反序列化等相关操作的配置等情况
      MSGPACK_DEFINE_ARRAY(accelerometer, gyroscope, compass)
    };
// 定义一个模板函数Serialize，它用于将传感器相关的数据进行序列化操作
// 模板参数SensorT表示传感器的数据类型，会根据具体使用时传入的类型来实例化函数
// 参数sensor是要序列化的具体传感器对象（具体类型由SensorT决定）
// 参数accelerometer是一个三维向量类型（geom::Vector3D），代表加速度计的数据
// 参数gyroscope同样是三维向量类型（geom::Vector3D），代表陀螺仪的数据
// 参数compass是一个单精度浮点数类型（float），代表罗盘的数据
    template <typename SensorT>
    static Buffer Serialize(
      const SensorT &sensor,
      const geom::Vector3D &accelerometer,
      const geom::Vector3D &gyroscope,
      const float compass);
// 定义一个静态函数DeserializeRawData，用于从原始数据（RawData类型）中反序列化出Data类型的数据
// 它内部调用了MsgPack::UnPack函数（MsgPack应该是一个序列化/反序列化相关的库），传入原始数据的起始迭代器（message.begin()）和数据大小（message.size()）来进行反序列化操作
    static Data DeserializeRawData(const RawData &message) {
      return MsgPack::UnPack<Data>(message.begin(), message.size());
    }
// 定义一个静态函数Deserialize，用于反序列化操作，它接收一个右值引用类型（RawData &&）的原始数据，并返回一个指向SensorData类型的智能指针（SharedPtr<SensorData>）
// 具体的反序列化逻辑应该在函数内部实现，这里只是函数声明
    static SharedPtr<SensorData> Deserialize(RawData &&data);
  };
// 定义模板函数IMUSerializer::Serialize的具体实现，这里是内联函数（inline），目的可能是为了提高执行效率，避免函数调用开销等
// 同样模板参数SensorT用于指定传感器的数据类型
// 这里虽然形式参数列表里有const SensorT &参数，但函数体中未使用该参数（可能后续会完善使用或者是代码编写过程中的临时情况等）
// 函数功能是调用MsgPack::Pack函数（同样来自相关序列化/反序列化库），将包含加速度计、陀螺仪、罗盘数据的Data结构体进行序列化操作，并返回序列化后的结果（Buffer类型，具体含义应该在代码其他地方有定义，大概率是存储序列化数据的某种容器类型）
  template <typename SensorT>
  inline Buffer IMUSerializer::Serialize(
      const SensorT &,
      const geom::Vector3D &accelerometer,
      const geom::Vector3D &gyroscope,
      const float compass) {
    return MsgPack::Pack(Data{accelerometer, gyroscope, compass});
  }

} // namespace s11n
} // namespace sensor
} // namespace carla
