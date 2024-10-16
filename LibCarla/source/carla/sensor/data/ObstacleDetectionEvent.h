// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Debug.h" // 包含carla的调试功能
#include "carla/client/detail/ActorVariant.h"
#include "carla/sensor/SensorData.h"  //包含carla传感器数据基类
#include "carla/sensor/s11n/ObstacleDetectionEventSerializer.h" // 包含障碍物检测事件的序列化器


namespace carla {
namespace sensor {
namespace data {

   // 定义一个注册的检测事件类
  class ObstacleDetectionEvent : public SensorData  { // 继承自SensorData基类
    using Super = SensorData; // 使用Super作为SensorData的别名，方便引用基类成员
  protected:

    using Serializer = s11n::ObstacleDetectionEventSerializer; // 使用Serializer作为ObstacleDetectionEventSerializer的别名

    friend Serializer; // 声明Serializer为友元类，允许访问私有成员

    explicit ObstacleDetectionEvent(const RawData &data) // 构造函数，从原始数据中反序列化ObstacleDetectionEvent对象
      : Super(data), // 调用基类构造函数
        _self_actor(nullptr), // 初始化_self_actor为nullptr
        _other_actor(nullptr) { // 初始化_other_actor为nullptr
      auto ddata = Serializer::DeserializeRawData(data); // 反序列化原始数据
      _self_actor = std::move(ddata.self_actor); // 移动self_actor数据
      _other_actor = std::move(ddata.other_actor); // 移动other_actor数据
      _distance = ddata.distance; // 赋值distance
    }

  public:

     // 获取检测到碰撞的“自身”actor
    SharedPtr<client::Actor> GetActor() const {
      return _self_actor.Get(GetEpisode()); // 使用GetEpisode获取当前剧集，并获取actor
    }

    // 获取发生碰撞的“其他”actor
    SharedPtr<client::Actor> GetOtherActor() const {
      return _other_actor.Get(GetEpisode()); // 使用GetEpisode获取当前剧集，并获取actor
    }

    // 获取障碍物距离
    float GetDistance() const {
      return _distance; // 返回距离值
    }

  private:

    client::detail::ActorVariant _self_actor; // 存储自身actor的变体类型

    client::detail::ActorVariant _other_actor; // 存储其他actor的变体类型

    float _distance; // 存储障碍物检测到的距离
  };

} // namespace data
} // namespace sensor
} // namespace carla
