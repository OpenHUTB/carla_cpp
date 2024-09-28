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

    boost::asio::mutable_buffer size_as_buffer() {
      return boost::asio::buffer(&_size, sizeof(_size));
    }

    boost::asio::mutable_buffer buffer() {
      DEBUG_ASSERT(_size > 0u);
      _buffer.reset(_size);
      return _buffer.buffer();
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
