#include "carla/Buffer.h"  // 包含 Buffer 头文件，定义 Buffer 类

#include "carla/BufferPool.h"  // 包含 BufferPool 头文件，定义 BufferPool 类

namespace carla {

  void Buffer::ReuseThisBuffer() {  // 定义 Buffer 类的 ReuseThisBuffer 方法
    auto pool = _parent_pool.lock();
    if (pool != nullptr) {
      pool->Push(std::move(*this));
    }
  }

} // namespace carla
