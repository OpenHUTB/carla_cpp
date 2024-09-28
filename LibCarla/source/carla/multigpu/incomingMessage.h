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
   /// 返回消息的大小（长度）。  
    /// 这个函数返回的是从TCP连接中读取的消息大小（字节数）。
    auto size() const {
      return _size;
    }
   /// 弹出（移动）当前消息使用的Buffer对象。  
    /// 这个函数将内部的_buffer对象以移动语义的方式返回，允许调用者接管该缓冲区的所有权。  
    /// 调用此函数后，IncomingMessage对象不再拥有该缓冲区。
    auto pop() {
      return std::move(_buffer);
    }

  private:
     /// 存储消息的大小（字节数）。  
    /// 这个变量用于存储从TCP连接中读取的消息大小
    carla::streaming::detail::message_size_type _size = 0u;
    /// 存储消息数据的缓冲区。  
    /// 这个Buffer对象用于存储实际的消息数据。  
    Buffer _buffer;
  };


} // namespace multigpu
} // namespace carla
