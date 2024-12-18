// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Buffer.h"  // 包含用于处理缓冲区数据的相关定义
#include "carla/Debug.h"  // 包含调试相关的宏和工具
#include "carla/Memory.h"  // 包含智能指针和内存管理工具
#include "carla/geom/Transform.h"  // 包含几何变换工具，例如位姿和旋转
#include "carla/geom/Vector3DInt.h"  // 包含整数三维向量定义
#include "carla/sensor/RawData.h"// 包含传感器原始数据的相关定义
#include "carla/sensor/data/ActorDynamicState.h"  // 包含动态对象状态的定义

#include <cstdint>// 标准库，用于固定宽度的整数类型

namespace carla {
namespace sensor {

  class SensorData;//传感器数据的基类声明

namespace s11n {

  /// Serializes the current state of the whole episode.
  class EpisodeStateSerializer {
  public:

    enum SimulationState {  //枚举类，用于表示模拟状态的类型
      None               = (0x0 << 0),  // 默认状态，无特定更新
      MapChange          = (0x1 << 0),  // 表示地图变更的状态
      PendingLightUpdate = (0x1 << 1)  // 表示待处理的交通信号灯更新
    };

#pragma pack(push, 1)
    struct Header {  // 数据包的头部结构，用于描述序列化数据的元信息
      uint64_t episode_id;  // 会话唯一标识符
      double platform_timestamp;  // 时间戳，表示当前平台的时间
      float delta_seconds;  // 当前状态与上一状态之间的时间差
      geom::Vector3DInt map_origin;  // 地图的原点位置（三维整数坐标）
      SimulationState simulation_state = SimulationState::None;  // 当前的模拟状态
    };
#pragma pack(pop)

    constexpr static auto header_offset = sizeof(Header);  // 数据头部的偏移量，用于快速定位数据正文

    //反序列化数据包头部
    static const Header &DeserializeHeader(const RawData &message) {  // 反序列化数据包头部
      return *reinterpret_cast<const Header *>(message.begin());  // 返回解析后的'Header'结构体的引用
    }

    template <typename SensorT>//序列化传感器数据
    static Buffer Serialize(const SensorT &, Buffer &&buffer) { // Sensor为输入的传感器对像，buffer为输入的缓冲区数据
      return std::move(buffer); // 直接返回传入的缓冲区数据
    }

    static SharedPtr<SensorData> Deserialize(RawData &&data);  //data输入的原始数据
  };  // 返回反序列化后的传感器数据对象的智能指针

} // namespace s11n
} // namespace sensor
} // namespace carla
