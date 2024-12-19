// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Debug.h"
#include "carla/sensor/data/ActorDynamicState.h"
#include "carla/sensor/data/Array.h"
#include "carla/sensor/s11n/EpisodeStateSerializer.h"

// 定义在carla命名空间下的sensor命名空间，再嵌套一个data命名空间，用于对传感器相关数据结构等进行更细分的组织
namespace carla {
namespace sensor {
namespace data {

  /// 表示给定帧处剧集的状态。
  class RawEpisodeState : public Array<ActorDynamicState> {
    using Super = Array<ActorDynamicState>;
  protected:

 // 定义用于序列化该类数据的序列化器类型为EpisodeStateSerializer，表明使用这个序列化器来处理数据的序列化和反序列化等相关操作
    using Serializer = s11n::EpisodeStateSerializer;

// 将EpisodeStateSerializer声明为友元类，这样它可以访问本类的私有成员，方便进行序列化相关操作
    friend Serializer;

// 显式构造函数，接受一个右值引用的RawData类型参数，用于初始化基类Array，通过调用Serializer的header_offset和移动传入的数据来完成初始化
    explicit RawEpisodeState(RawData &&data)
      : Super(Serializer::header_offset, std::move(data)) {}

  private:

// 获取头部信息的函数，通过调用Serializer的反序列化头部函数，传入基类的原始数据来获取相关头部信息
    auto GetHeader() const {
      return Serializer::DeserializeHeader(Super::GetRawData());
    }

  public:

    /// 获取生成此数据的剧集的唯一ID。
    uint64_t GetEpisodeId() const {
      return GetHeader().episode_id;
    }

    /// 模拟时间戳，即从当前剧集开始到现在经过的模拟秒数。
    double GetGameTimeStamp() const {
      return GetTimestamp();
    }

    /// 获取进行此次测量的帧的时间戳，时间单位为秒，该时间戳由操作系统提供。
    double GetPlatformTimeStamp() const {
      return GetHeader().platform_timestamp;
    }

    /// 获取自上一帧以来经过的模拟秒数。
    double GetDeltaSeconds() const {
      return GetHeader().delta_seconds;
    }

    geom::Vector3DInt GetMapOrigin() const {
      return GetHeader().map_origin;
    }

    /// 获取模拟状态标志，用于表示当前模拟的一些状态情况
    Serializer::SimulationState GetSimulationState() const {
      return GetHeader().simulation_state;
    }

  };

} // namespace data
} // namespace sensor
} // namespace carla
