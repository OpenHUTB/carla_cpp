// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once  // 确保此头文件只被包含一次

#include "carla/Buffer.h"  // 包含 Buffer 头文件，定义 Buffer 类

#if defined(__clang__)  // 检查是否使用 Clang 编译器
#  pragma clang diagnostic push   // 保存当前的编译警告状态
#  pragma clang diagnostic ignored "-Wold-style-cast"  // 忽略旧风格的类型转换警告
#endif
#include "moodycamel/ConcurrentQueue.h"  // 包含 moodycamel 的并发队列实现
#if defined(__clang__)  // 再次检查是否使用 Clang 编译器
#  pragma clang diagnostic pop  // 恢复之前保存的编译警告状态
#endif

#include <memory>  // 包含内存管理相关的头文件

namespace carla {

  /// 一个缓冲区池。 从这个池中弹出的缓冲区在销毁时会自动返回到池中，
  /// 这样分配的内存可以被重用。
  /// @warning 缓冲区仅通过增长来调整其大小，除非明确地清除它们，否则不会缩小。
  /// 分配的内存在此池被销毁时才会被删除。

  class BufferPool : public std::enable_shared_from_this<BufferPool> {  // 定义 BufferPool 类，支持共享指针
  public:

    BufferPool() = default;  // 默认构造函数

    explicit BufferPool(size_t estimated_size) : _queue(estimated_size) {}  // 带参数的构造函数，初始化队列大小

  /// 从队列中弹出一个缓冲区，如果队列为空，则创建一个新的缓冲区。
    Buffer Pop() {
      Buffer item; // 创建一个 Buffer 实例
      _queue.try_dequeue(item); // 尝试从队列中弹出，失败则不处理
#if __cplusplus >= 201703L // 检查是否支持 C++17
      item._parent_pool = weak_from_this();  // 设置父池为弱引用
#else
      item._parent_pool = shared_from_this();  // 设置父池为共享引用
#endif
      return item;  // 返回弹出的 Buffer
    }

  private:

    friend class Buffer;  // 允许 Buffer 类访问私有成员

    void Push(Buffer &&buffer) {  // 定义 Push 方法，接受一个右值引用的 Buffer
      _queue.enqueue(std::move(buffer));  // 将 Buffer 移动到队列中
    }

    moodycamel::ConcurrentQueue<Buffer> _queue;  // 定义并发队列用于存储 Buffer
  };

} // namespace carla
