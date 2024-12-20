// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
// 这是一个预处理指令，用于确保头文件只被包含一次

#include "carla/Buffer.h"
// 引入carla项目中的Buffer.h头文件

#include "carla/Debug.h"
// 引入carla项目中的Debug.h头文件

#include "carla/Memory.h"
// 引入carla项目中的Memory.h头文件

#include "carla/rpc/Actor.h"
// 引入carla项目中rpc模块下的Actor.h头文件
#include "carla/sensor/RawData.h"
// 引入carla项目中sensor模块下的RawData.h头文件

namespace carla {
namespace sensor {

  class SensorData;
// 前置声明SensorData类，告知编译器后续会有这个类的定义，在当前代码位置可以使用该类的指针或引用类型，便于解决类之间相互引用时的编译顺序问题

namespace s11n {

  /// Serializes the current state of the whole episode.
  class ObstacleDetectionEventSerializer {
  public:

    struct Data {

      rpc::Actor self_actor;
// 定义一个rpc::Actor类型的成员变量self_actor

      rpc::Actor other_actor;
// 定义一个rpc::Actor类型的成员变量other_actor

      float distance;
 // 定义一个float类型的成员变量distance，用于表示距离相关的信息

      MSGPACK_DEFINE_ARRAY(self_actor, other_actor, distance)
// 这应该是一个宏用于定义如何将包含self_actor、other_actor、distance这几个成员的结构体进行序列化，告诉序列化库要处理的成员变量列表
    };

    constexpr static auto header_offset = 0u;
// 定义一个常量表达式（constexpr）类型的静态变量header_offset，并初始化为0（无符号整数类型）

    static Data DeserializeRawData(const RawData &message) {
// 定义一个静态成员函数DeserializeRawData，用于从给定的RawData类型的对象（通常是传感器原始数据）中反序列化出Data结构体类型的数据。
// 参数message是一个const引用，避免不必要的数据拷贝，传入要进行反序列化的原始数据

      return MsgPack::UnPack<Data>(message.begin(), message.size());
 // 调用MsgPack序列化库的UnPack函数，将从message对象表示的数据起始位置（message.begin()）到数据末尾（message.size()指定的数据长度范围）的数据进行反序列化，转换为Data结构体类型并返回
    }

    template <typename SensorT>
    static Buffer Serialize(
        const SensorT &,
        rpc::Actor self_actor,
        rpc::Actor other_actor,
        float distance) {
// 定义一个函数模板Serialize，用于将相关数据进行序列化并返回一个Buffer类型的结果
 // 它是一个静态函数，并且是模板函数，可以适用于不同类型的SensorT
 // 第一个参数是一个const引用类型的SensorT对象（具体类型由调用时传入的实际类型决定），后面三个参数分别是自身Actor、其他Actor以及距离信息，这些都是要参与序列化的数据

      return MsgPack::Pack(Data{self_actor, other_actor, distance});
// 创建一个Data结构体对象，使用传入的self_actor、other_actor、distance进行初始化，然后调用MsgPack序列化库的Pack函数（推测用于序列化）将这个Data结构体对象进行序列化，并返回序列化后的结果（存储在Buffer类型中）
    }

    static SharedPtr<SensorData> Deserialize(RawData &&data);
// 定义一个静态成员函数Deserialize，用于从右值引用类型的RawData对象（传感器原始数据）中反序列化出一个指向SensorData类的智能指针（SharedPtr<SensorData>）。
// 这里使用右值引用可以更高效地处理临时对象等情况
  };

} // namespace s11n
} // namespace sensor
} // namespace carla
