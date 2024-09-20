#include "carla/Buffer.h"  // ���� Buffer ͷ�ļ������� Buffer ��

#include "carla/BufferPool.h"  // ���� BufferPool ͷ�ļ������� BufferPool ��

namespace carla {

  void Buffer::ReuseThisBuffer() {  // ���� Buffer ��� ReuseThisBuffer ����
    auto pool = _parent_pool.lock();
    if (pool != nullptr) {
      pool->Push(std::move(*this));
    }
  }

} // namespace carla
