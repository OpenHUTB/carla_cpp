// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <compiler/disable-ue4-macros.h>
#include <carla/Buffer.h>
#include <carla/Logging.h>
#include <carla/sensor/SensorRegistry.h>
#include <carla/sensor/s11n/SensorHeaderSerializer.h>
#include <carla/streaming/Stream.h>
#include <compiler/enable-ue4-macros.h>

template <typename T>
class FDataStreamTmpl;

// =============================================================================
// -- FAsyncDataStreamTmpl -----------------------------------------------------
// =============================================================================

//用于向客户端发送传感器数据的流媒体通道，支持发送
///异步数据。“发送”功能发送的数据被传递给
///序列化程序在carla:：sensor:SensorRegistry上注册了传感器
///被顺流而下。
///
///@warning这是一个一次性对象，需要为每个对象创建一个新对象
///新消息。
///
///FAsyncDataStream还有一个carla:：Buffer池，允许重用
///分配内存，尽可能使用它。
class FAsyncDataStreamTmpl
{
public:

  using StreamType = T;

  FAsyncDataStreamTmpl(FAsyncDataStreamTmpl &&) = default;

  /// 返回允许订阅此流的令牌。
  auto GetToken() const
  {
    return Stream.GetToken();
  }

  /// 从池中弹出缓冲区。池中的缓冲区可以重用以前消息分配的内存，从而显著提高大消息的性能。
  carla::Buffer PopBufferFromPool()
  {
    return Stream.MakeBuffer();
  }

  /// 向下游发送一些数据。
  template <typename SensorT, typename... ArgsT>
  void Send(SensorT &Sensor, ArgsT &&... Args);

  template <typename SensorT, typename... ArgsT>
  void SerializeAndSend(SensorT &Sensor, ArgsT &&... Args);

  /// 允许更改标头的帧号
  void SetFrameNumber(uint64_t FrameNumber)
  {
    carla::sensor::s11n::SensorHeaderSerializer::Header *HeaderStr =
      reinterpret_cast<carla::sensor::s11n::SensorHeaderSerializer::Header *>(Header.data());
    if (HeaderStr)
    {
      if (HeaderStr->frame != FrameNumber)
      {
        carla::log_info("Re-framing sensor type ", HeaderStr->sensor_type, " from ", HeaderStr->frame, " to ", FrameNumber);
        HeaderStr->frame = FrameNumber;
      }
    }
  }

  /// 返回此流的传感器类型
  uint64_t GetSensorType()
  {
    carla::sensor::s11n::SensorHeaderSerializer::Header *HeaderStr =
      reinterpret_cast<carla::sensor::s11n::SensorHeaderSerializer::Header *>(Header.data());
    if (HeaderStr)
    {
      return HeaderStr->sensor_type;
    }
    return 0u;
  }

  /// 返回传感器的变换
  FTransform GetSensorTransform()
  {
    carla::sensor::s11n::SensorHeaderSerializer::Header *HeaderStr =
      reinterpret_cast<carla::sensor::s11n::SensorHeaderSerializer::Header *>(Header.data());
    if (HeaderStr)
    {
      return FTransform(HeaderStr->sensor_transform);
    }
    return FTransform();
  }

  /// 返回传感器的时间戳
  double GetSensorTimestamp()
  {
    carla::sensor::s11n::SensorHeaderSerializer::Header *HeaderStr =
      reinterpret_cast<carla::sensor::s11n::SensorHeaderSerializer::Header *>(Header.data());
    if (HeaderStr)
    {
      return HeaderStr->timestamp;
    }
    return 0.0;
  }

private:

  friend class FDataStreamTmpl<T>;

  /// @这个函数需要在游戏线程中调用。
  template <typename SensorT>
  explicit FAsyncDataStreamTmpl(
      const SensorT &InSensor,
      double Timestamp,
      StreamType InStream);

  StreamType Stream;

  carla::Buffer Header;
};

// =============================================================================
// -- FAsyncDataStream and FAsyncDataMultiStream -------------------------------
// =============================================================================

using FAsyncDataStream = FAsyncDataStreamTmpl<carla::streaming::Stream>;

using FAsyncDataMultiStream = FAsyncDataStreamTmpl<carla::streaming::Stream>;

// =============================================================================
// -- FAsyncDataStreamTmpl implementation --------------------------------------
// =============================================================================

template <typename T>
template <typename SensorT, typename... ArgsT>
inline void FAsyncDataStreamTmpl<T>::SerializeAndSend(SensorT &Sensor, ArgsT &&... Args)
{
  // 序列化数据
  carla::Buffer Data(carla::sensor::SensorRegistry::Serialize(Sensor, std::forward<ArgsT>(Args)...));

  // 创建缓冲区视图
  auto ViewHeader = carla::BufferView::CreateFrom(std::move(Header));
  auto ViewData = carla::BufferView::CreateFrom(std::move(Data));

  // 发送视图
  Stream.Write(ViewHeader, ViewData);
}

template <typename T>
template <typename SensorT, typename... ArgsT>
inline void FAsyncDataStreamTmpl<T>::Send(SensorT &Sensor, ArgsT &&... Args)
{
  // 创建缓冲区视图
  auto ViewHeader = carla::BufferView::CreateFrom(std::move(Header));

  // 发送视图
  Stream.Write(ViewHeader, std::forward<ArgsT>(Args)...);
}
