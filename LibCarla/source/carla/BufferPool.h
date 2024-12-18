// 版权所有 (c) 2017 巴塞罗那自治大学 (UAB) 的计算机视觉中心 (CVC)。
//
// 本作品依据 MIT 许可证的条款进行授权。
// 如需许可证副本，请访问 <https://opensource.org/licenses/MIT>。

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

  /// 定义一个缓冲区池类。该类管理的缓冲区在销毁时会自动返回到池中，以便重用分配的内存。
  /// 注意：缓冲区仅通过增长来调整大小，不会自动缩小，除非显式清除。池销毁时才会释放内存。
  class BufferPool : public std::enable_shared_from_this<BufferPool> {  // 定义 BufferPool 类，支持共享指针
  public:

    BufferPool() = default; // 默认构造函数，不执行任何特殊操作

    explicit BufferPool(size_t estimated_size) : _queue(estimated_size) {}  // 带参数的构造函数，根据提供的估计大小初始化内部队列。

    /// 从池中获取一个缓冲区。如果队列为空，则创建一个新的缓冲区。
    /// 返回的缓冲区将设置其父池引用（C++17使用弱引用，以避免循环引用）。
    Buffer Pop() {
      Buffer item; // 临时创建一个Buffer实例，用于尝试从队列中获取缓冲区
      _queue.try_dequeue(item); // 尝试从并发队列中弹出一个缓冲区
#if __cplusplus >= 201703L // 检查是否支持 C++17
      item._parent_pool = weak_from_this();  // 设置父池为弱引用，C++17及以上版本使用弱引用，防止循环引用
#else
      item._parent_pool = shared_from_this();  // 设置父池为共享引用，C++17以下版本使用共享引用
#endif
      return item;  // 返回可能未正确初始化的Buffer对象
    }

  private:

    friend class Buffer;  // 允许Buffer类访问BufferPool的私有成员

    /// 将一个缓冲区推回到池中。接受一个右值引用的Buffer对象。
    void Push(Buffer &&buffer) {  // 定义 Push 方法，接受一个右值引用的 Buffer
      _queue.enqueue(std::move(buffer));  // 将 Buffer 移动到队列中
    }
    /// 用于存储缓冲区的并发队列。
    moodycamel::ConcurrentQueue<Buffer> _queue;  // 定义并发队列用于存储 Buffer
  };

} // namespace carla
