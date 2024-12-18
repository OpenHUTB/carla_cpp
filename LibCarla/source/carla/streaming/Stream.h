// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
// 表明该代码遵循MIT许可协议，可通过对应链接查看协议具体内容
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

// 引入Carla项目中streaming模块细节相关的MultiStreamState头文件，从命名推测，它可能用于处理多流相关的状态信息，具体功能取决于其内部定义
#include "carla/streaming/detail/MultiStreamState.h"
// 引入Carla项目中streaming模块细节相关的Stream头文件，大概率是用于构建流相关的功能，比如流的创建、数据传输等操作，其内部应有相应的实现逻辑
#include "carla/streaming/detail/Stream.h"

namespace carla {
namespace streaming {

  // 对Stream类型进行定义（通过using关键字起别名），这里将Stream定义为detail::Stream<detail::MultiStreamState>这种模板类型。
  // 意味着Stream类型代表了一个单向的通道（从注释可知，是用于从服务器向客户端发送数据的通道），并且它基于detail::MultiStreamState相关的状态来进行操作，
  // 多个客户端可以通过流令牌（stream token，一种可能用于标识和访问流的机制，虽然这里未详细展开说明）来订阅这个流。
  // 如果没有客户端订阅该流，那么沿着这个流推送（flushed down）的数据将会被丢弃。
  /// A stream represents an unidirectional channel for sending data from server
  /// to client. **Multiple** clients can subscribe to this stream using the
  /// stream token. If no client is subscribed, the data flushed down the stream
  /// is discarded.
  using Stream = detail::Stream<detail::MultiStreamState>;

} // namespace streaming
} // namespace carla
