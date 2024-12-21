// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once  // 防止头文件被多次包含

#include "carla/Buffer.h"  // 引入Buffer类的定义

// 针对Clang编译器的编译警告处理
#if defined(__clang__)  
#  pragma clang diagnostic push   
#  pragma clang diagnostic ignored "-Wold-style-cast"  
#endif
#include "moodycamel/ConcurrentQueue.h"  // 引入moodycamel的并发队列库
#if defined(__clang__)  
#  pragma clang diagnostic pop  
#endif

#include <memory>  // 引入智能指针等内存管理工具

namespace carla {

  /// BufferPool类用于管理Buffer对象的池。
  /// 它确保从池中弹出的Buffer在销毁时能自动返回池中，以便重用内存。
  /// 注意：Buffer的大小只能增加，除非明确清除，否则不会减少。
  /// 池销毁时，分配的内存才会被释放。
  class BufferPool : public std::enable_shared_from_this<BufferPool> {
  public:
    BufferPool() = default;  // 默认构造函数

    explicit BufferPool(size_t estimated_size) : _queue(estimated_size) {}  // 带参数构造函数，设置队列的预估大小

    /// 从池中弹出一个Buffer，如果池为空，则新建一个Buffer。
    Buffer Pop() {
      Buffer item; 
      _queue.try_dequeue(item); 
#if __cplusplus >= 201703L 
      item._parent_pool = weak_from_this(); 
#else
      item._parent_pool = shared_from_this(); 
#endif
      return item;  
    }

  private:
    friend class Buffer;  // 允许Buffer类访问BufferPool的私有成员

    /// 将一个Buffer推回池中。
    void Push(Buffer &&buffer) {  
      _queue.enqueue(std::move(buffer));  
    }

    moodycamel::ConcurrentQueue<Buffer> _queue;  // 用于存储Buffer的并发队列
  };

} // namespace carla
