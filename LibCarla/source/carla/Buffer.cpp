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

} // namespace carla

