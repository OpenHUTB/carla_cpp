// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Memory.h" // 提供智能指针和内存管理相关的功能
#include "carla/geom/GeoLocation.h" // 定义了 GeoLocation，表示地理位置信息
#include "carla/rpc/ActorId.h" // 定义了 ActorId，用于标识参与者
#include "carla/sensor/RawData.h" // 定义了 RawData，用于表示原始传感器数据

#include <cstdint> // 提供固定大小的整数类型
#include <cstring> // 提供内存操作函数

namespace carla { // Carla 项目顶级命名空间
namespace sensor { // 包含所有与传感器相关的功能

  class SensorData; // 前向声明，用于表示传感器数据的基类

namespace s11n { // 提供序列化和反序列化功能的命名空间

  class GnssSerializer {  // 数据的序列化器，用于处理 GNSS 数据的序列化和反序列化操作
  public:

    static geom::GeoLocation DeserializeRawData(const RawData &message) {
      // 使用 MsgPack 解包 RawData 中的数据，并将其解析为 GeoLocation 对象
      return MsgPack::UnPack<geom::GeoLocation>(message.begin(), message.size());
    }

    /// @brief 将地理位置信息序列化为二进制数据
    /// @tparam SensorT 传感器类型
    /// @param sensor 传感器对象
    /// @param geo_location 包含地理位置信息的 GeoLocation 对象
    /// @return 序列化后的 Buffer 对象，包含地理位置的二进制表示
    template <typename SensorT>
    static Buffer Serialize(
        const SensorT &,
        const geom::GeoLocation &geo_location
        ) {  // 使用 MsgPack 将 GeoLocation 对象序列化为二进制数据
      return MsgPack::Pack(geo_location);
    }

    static SharedPtr<SensorData> Deserialize(RawData &&data);
  };

} // namespace s11n
} // namespace sensor
} // namespace carla
