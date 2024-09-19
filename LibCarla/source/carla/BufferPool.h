// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Buffer.h"

#if defined(__clang__)
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Wold-style-cast"
#endif
#include "moodycamel/ConcurrentQueue.h"
#if defined(__clang__)
#  pragma clang diagnostic pop
#endif

#include <memory>

namespace carla {    // ���� carla �����ռ�

  

  class BufferPool : public std::enable_shared_from_this<BufferPool> {
  public:

    BufferPool() = default;    // Ĭ�Ϲ��캯��

    explicit BufferPool(size_t estimated_size) : _queue(estimated_size) {}

  
    Buffer Pop() {
      Buffer item;
      _queue.try_dequeue(item); // we don't care if it fails.
#if __cplusplus >= 201703L // C++17
      item._parent_pool = weak_from_this();
#else
      item._parent_pool = shared_from_this();
#endif
      return item;
    }

  private:

    friend class Buffer;

    void Push(Buffer &&buffer) {
      _queue.enqueue(std::move(buffer));
    }

    moodycamel::ConcurrentQueue<Buffer> _queue;
  };

} // namespace carla
