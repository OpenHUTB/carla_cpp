// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/BufferPool.h"
#include "carla/Debug.h"
#include "carla/Exception.h"
#include "carla/Logging.h"
#include "carla/streaming/detail/Types.h"
#include "carla/Time.h"

#include <exception>

namespace carla {
namespace multigpu {

  /// 这个类是一个辅助类，用于读取传入的TCP消息
  class IncomingMessage {
  public:
  /// 构造函数，接收一个Buffer对象并将其移动赋值给内部_buffer。  
  /// 这个构造函数用于初始化IncomingMessage对象，接收一个已经分配好的缓冲区。  
    explicit IncomingMessage(Buffer &&buffer) : _buffer(std::move(buffer)) {}
  /// 返回一个boost::asio::mutable_buffer，指向消息大小的存储位置。  
  /// 这个函数用于在接收消息时，首先读取消息的大小（长度）。
    boost::asio::mutable_buffer size_as_buffer() {
      return boost::asio::buffer(&_size, sizeof(_size));
    }
   /// 返回一个boost::asio::mutable_buffer，指向消息数据的存储位置。  
    /// 在调用这个函数之前，必须确保_size已经被正确设置（即先调用了size_as_buffer()并读取了消息大小）。  
    /// 此函数重置_buffer的大小以匹配消息的实际大小，并返回指向该缓冲区的引用。  
    boost::asio::mutable_buffer buffer() {
      DEBUG_ASSERT(_size > 0u); // 确保消息大小大于0 
      _buffer.reset(_size); // 根据消息大小重置缓冲区
      return _buffer.buffer(); // 返回缓冲区的引用
    }

    auto size() const {
      return _size;
    }

    auto pop() {
      return std::move(_buffer);
    }

  private:

    carla::streaming::detail::message_size_type _size = 0u;

    Buffer _buffer;
  };


} // namespace multigpu
} // namespace carla
