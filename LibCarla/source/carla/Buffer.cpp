#include "carla/Buffer.h"  // 包含 Buffer 头文件，定义 Buffer 类

#include "carla/BufferPool.h"  // 包含 BufferPool 头文件，定义 BufferPool 类

namespace carla {

  void Buffer::ReuseThisBuffer() {  // 定义 Buffer 类的 ReuseThisBuffer 方法
    auto pool = _parent_pool.lock();  // 尝试锁定指向父池的弱指针
    if (pool != nullptr) {   // 检查池是否有效（非空）
      pool->Push(std::move(*this));    // 将当前 Buffer 实例移动到池的 Push 方法中
    }
  }

} // namespace carla
