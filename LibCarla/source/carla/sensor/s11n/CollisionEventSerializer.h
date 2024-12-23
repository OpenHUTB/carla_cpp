// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Buffer.h"
#include "carla/Debug.h"
#include "carla/Memory.h"
#include "carla/rpc/Actor.h"
#include "carla/geom/Vector3D.h"
#include "carla/sensor/RawData.h"

// 命名空间 carla
namespace carla {
  // 命名空间 sensor
  namespace sensor {

    // 前置声明 SensorData 类
    class SensorData;

    // 命名空间 s11n
    namespace s11n {

      /// 序列化整个事件的当前状态的类
      class CollisionEventSerializer {
      public:
        // 结构体 Data，可能用于存储序列化所需的数据
        struct Data {
          // 表示自身的 Actor 对象
          rpc::Actor self_actor;
          // 表示另一个 Actor 对象
          rpc::Actor other_actor;
          // 表示法向冲量的 Vector3D 对象
          geom::Vector3D normal_impulse;

          // 使用 MSGPACK_DEFINE_ARRAY 宏来定义 msgpack 序列化和反序列化的数组元素，方便序列化和反序列化操作
          MSGPACK_DEFINE_ARRAY(self_actor, other_actor, normal_impulse)
        };

        // 定义一个常量，表示头部偏移量为 0
        constexpr static auto header_offset = 0u;

        // 从原始数据反序列化数据的静态函数
        static Data DeserializeRawData(const RawData &message) {
          // 使用 MsgPack 库的 UnPack 函数将 message 中的数据解包为 Data 类型
          return MsgPack::UnPack<Data>(message.begin(), message.size());
        }

        // 模板函数 Serialize，用于序列化数据，SensorT 是一个模板参数
        template <typename SensorT>
        static Buffer Serialize(
            const SensorT &,
            rpc::Actor self_actor,
            rpc::Actor other_actor,
            geom::Vector3D normal_impulse) {
          // 使用 MsgPack 库的 Pack 函数将输入的数据打包为 Buffer 类型
          return MsgPack::Pack(Data{self_actor, other_actor, normal_impulse});
        }

        // 静态函数 Deserialize，声明但未实现，接收一个右值引用的 RawData 作为输入
        static SharedPtr<SensorData> Deserialize(RawData &&data);
      };

    } // namespace s11n
  } // namespace sensor
} // namespace carla
