// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Sensor/AsyncDataStream.h"

#include <compiler/disable-ue4-macros.h>
#include <carla/streaming/Stream.h>
#include <boost/optional.hpp>
#include <compiler/enable-ue4-macros.h>

// =============================================================================
// -- FDataStreamTmpl ----------------------------------------------------------
// =============================================================================

/// 用于将传感器数据发送到客户端的流式处理通道。每个传感器
/// 都有自己的 FDataStream。但请注意，此类不提供 send 函数。
/// 为了发送数据，需要使用 “MakeAsyncDataStream” 函数
/// 创建一个 FAsyncDataStream。
/// FAsyncDataStream 允许从任何线程发送数据。
template <typename T>
class FDataStreamTmpl
{
public:

  using StreamType = T;

  FDataStreamTmpl() = default;

  FDataStreamTmpl(StreamType InStream) : Stream(std::move(InStream)) {}

  /// 创建 FAsyncDataStream 对象。
  ///
  /// @pre This functions needs to be called in the game-thread.
  template <typename SensorT>
  FAsyncDataStreamTmpl<T> MakeAsyncDataStream(const SensorT &Sensor, double Timestamp)
  {
    check(Stream.has_value());
    return FAsyncDataStreamTmpl<T>{Sensor, Timestamp, *Stream};
  }

  bool IsStreamReady()
  {
    return Stream.has_value();
  }

  /// 返回允许订阅此流的令牌。
  auto GetToken() const
  {
    check(Stream.has_value());
    return Stream->token();
  }

  uint64_t GetSensorType()
  {
    check(Stream.has_value());
    return Stream->get_stream_id();
  }

  bool AreClientsListening()
  {
    check(Stream.has_value());
    return Stream->AreClientsListening();
  }

private:

  boost::optional<StreamType> Stream;
};

// =============================================================================
// -- FDataStream and FDataMultiStream -----------------------------------------
// =============================================================================

using FDataStream = FDataStreamTmpl<carla::streaming::Stream>;

using FDataMultiStream = FDataStreamTmpl<carla::streaming::Stream>;
