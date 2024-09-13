#include "carla/Buffer.h"

#include "carla/BufferPool.h"

namespace carla {

  void Buffer::ReuseThisBuffer() {
    auto pool = _parent_pool.lock();
    if (pool != nullptr) {
      pool->Push(std::move(*this));
    }
  }

} // Ãû³Æ¿Õ¼ä 
