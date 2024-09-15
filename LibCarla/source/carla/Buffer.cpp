#include "carla/Buffer.h"  // 包含 Buffer 类的定义

#include "carla/BufferPool.h"  // 包含 BufferPool 类的定义

namespace carla {  // carla 命名空间，封装相关类和函数

  // Buffer 类的成员函数，实现重新利用当前缓冲区
  void Buffer::ReuseThisBuffer() {
    auto pool = _parent_pool.lock();  // 尝试锁定并获取对父池的弱引用
    if (pool != nullptr) {  // 如果成功获取到父池
      pool->Push(std::move(*this));  // 将当前缓冲区移动到父池中
    }
  }

} // namespace carla

