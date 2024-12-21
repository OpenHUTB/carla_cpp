// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/streaming/detail/StreamStateBase.h"

#include "carla/BufferPool.h"

namespace carla {     // 开始carla命名空间
namespace streaming { // 开始streaming命名空间，表示与流相关的功能
namespace detail {    // 开始detail命名空间，通常用于包含实现细节，对外部隐藏

   // StreamStateBase类的构造函数实现
  // 构造函数接收一个token_type类型的参数token，用于标识或认证流
  StreamStateBase::StreamStateBase(const token_type &token)
    : _token(token), // 初始化成员变量_token，用于存储传入的token
      _buffer_pool(std::make_shared<BufferPool>()) {}// 初始化成员变量_buffer_pool，它是一个指向BufferPool对象的智能指针，用于管理内存缓冲区

// StreamStateBase类的析构函数实现
  // 默认析构函数，自动调用，用于清理资源
  // 由于_buffer_pool是一个智能指针，它会在析构时自动释放其管理的BufferPool对象
  StreamStateBase::~StreamStateBase() = default;

  // StreamStateBase类的成员函数MakeBuffer实现
  // 该函数用于从缓冲区池中获取一个新的缓冲区
  Buffer StreamStateBase::MakeBuffer() {
    auto pool = _buffer_pool;
    return pool->Pop();
  }

} // namespace detail
} // namespace streaming
} // namespace carla
