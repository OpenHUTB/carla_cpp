#include "carla/Buffer.h"  // ���� Buffer ��Ķ���

#include "carla/BufferPool.h"  // ���� BufferPool ��Ķ���

namespace carla {  // carla �����ռ䣬��װ�����ͺ���

  // Buffer ��ĳ�Ա������ʵ���������õ�ǰ������
  void Buffer::ReuseThisBuffer() {
    auto pool = _parent_pool.lock();  // ������������ȡ�Ը��ص�������
    if (pool != nullptr) {  // ����ɹ���ȡ������
      pool->Push(std::move(*this));  // ����ǰ�������ƶ���������
    }
  }

<<<<<<< HEAD
} // namespace carla

=======
} //namespace carla
>>>>>>> 5e39fcd54d1550e768c08d3a906986144c8ba7ac
