// 版权信息，指出这段代码由巴塞罗那自治大学（UAB）的计算机视觉中心（CVC）创作，
// 并按照MIT许可证的条款进行授权。MIT许可证的副本可以在https://opensource.org/licenses/MIT找到。
 
#pragma once
// 这个指令用于防止头文件被多次包含（即“多重包含”），确保头文件只被编译一次。
 
#include "carla/streaming/detail/MultiStreamState.h"
// 包含MultiStreamState类的定义，这个类可能是用于管理多个流状态的。
 
#include "carla/streaming/detail/Stream.h"
// 包含Stream类的定义，这个类可能代表了一个基本的数据流。
 
namespace carla {
namespace streaming {
 
  /// A stream represents an unidirectional channel for sending data from server
  /// to client. **Multiple** clients can subscribe to this stream using the
  /// stream token. If no client is subscribed, the data flushed down the stream
  /// is discarded.
  // 定义一个别名Stream，它代表了一个从服务器到客户端的单向数据传输通道。
  // 多个客户端可以使用流令牌订阅这个流。如果没有客户端订阅，那么流中的数据将被丢弃。
  using Stream = detail::Stream<detail::MultiStreamState>;
  // 这里使用了别名定义，将detail命名空间下的Stream模板类，使用MultiStreamState作为模板参数实例化，
  // 并将其别名定义为Stream。这意味着这个Stream类将支持多流状态管理。
 
} // namespace streaming
} // namespace carla
