// 版权所有 (c) 2017 巴塞罗那自治大学 (UAB) 计算机视觉中心 (CVC)。
//
// 本作品根据 MIT 许可证的条款进行授权。
// 如需获取副本，请访问 https://opensource.org/licenses/MIT。
 
#pragma once   // 防止头文件被重复包含
 
#include <memory>    // 引入内存管理相关的头文件
 
namespace carla {

  /// AtomicSharedPtr是一个非常简单的原子操作智能指针类，支持线程安全的共享指针操作，
  /// 使用了释放-获取（release-acquire）内存顺序来保证数据同步。
  /// 
  /// Release-Acquire（RA）内存顺序是一种用于多线程编程的同步机制，
  /// 它确保一个线程释放（release）对某个变量的修改后，
  /// 另一个线程在获取（acquire）该变量之前，能看到前一个线程的修改。
  /// 这种机制有助于避免读写顺序错误，确保数据一致性和线程安全。
  /// 
  /// 在这个AtomicSharedPtr类中，通过使用底层的原子操作，
  /// 实现了对共享指针的线程安全访问。
  /// 这意味着多个线程可以安全地共享和修改同一个指针，
  /// 而不会出现数据竞争或不一致的情况。
  /// 
  /// 参考链接提供了一个更深入的讨论和示例，
  /// 可以帮助理解Release-Acquire内存顺序和原子操作在多线程编程中的应用。
  /// 参考：https://zhuanlan.zhihu.com/p/669908979
  /// 
  template <typename T>
  class AtomicSharedPtr {
  public:
  	
   	/// 构造函数
    /// 使用转发参数包将传入参数完美转发给内部的 shared_ptr 进行初始化。
    /// 这允许直接构造一个 AtomicSharedPtr，同时传递任意数量和类型的参数给 shared_ptr 的构造函数。
    template <typename... Args>
    explicit AtomicSharedPtr(Args &&... args)
      : _ptr(std::forward<Args>(args)...) {}
      
    /// 拷贝构造函数
    /// 使用 load() 函数来获取当前 AtomicSharedPtr 对象内部的 shared_ptr 值，
    /// 并用它来初始化新的 AtomicSharedPtr 对象。
    /// 这确保了新对象与源对象共享相同的资源，但拥有独立的控制权。
    AtomicSharedPtr(const AtomicSharedPtr &rhs)
      : _ptr(rhs.load()) {}
      
 	 // 删除移动构造函数，不允许对象的移动操作
    AtomicSharedPtr(AtomicSharedPtr &&) = delete;
    
 	  /// store 函数
    /// 使用 release 内存顺序来存储新的 shared_ptr 值。
    /// 这保证了在当前线程释放旧值之前，其他线程不会获取到新值。
    /// 这是线程间同步的一种机制。
    void store(std::shared_ptr<T> ptr) noexcept {
      std::atomic_store_explicit(&_ptr, ptr, std::memory_order_release);
    }
 	  /// reset 函数
    /// 重置内部的 shared_ptr 值。如果提供了新指针，则使用它；否则，使用 nullptr。
    /// 这实际上是 store 函数的一个便捷封装。
    void reset(std::shared_ptr<T> ptr = nullptr) noexcept {
      store(ptr);
    }
    
 	// 加载指针的当前值，使用acquire内存顺序
    std::shared_ptr<T> load() const noexcept {
      return std::atomic_load_explicit(&_ptr, std::memory_order_acquire);
    }
 	
 	  /// compare_exchange 函数
    /// 原子地比较并交换内部的 shared_ptr 值。
    /// 如果当前值与预期值匹配，则将其替换为期望的值。
    /// 这是一种线程安全的条件更新机制。
    bool compare_exchange(std::shared_ptr<T> *expected, std::shared_ptr<T> desired) noexcept {
      return std::atomic_compare_exchange_strong_explicit(
          &_ptr,
          expected,
          desired,
          std::memory_order_acq_rel,
          std::memory_order_acq_rel);  // 原子比较并交换
    } 
    /// 赋值运算符重载（接受 shared_ptr）
    /// 使用提供的 shared_ptr 值来更新内部的指针。
    /// 这允许直接将一个 shared_ptr 赋值给一个 AtomicSharedPtr 对象。
    AtomicSharedPtr &operator=(std::shared_ptr<T> ptr) noexcept {
      store(std::move(ptr)); // 存储新指针
      return *this;  // 返回当前对象的引用
    }
    /// 赋值运算符重载（复制赋值）
    /// 使用另一个 AtomicSharedPtr 对象的值来更新当前对象。
    /// 这确保了两个对象共享相同的资源，但每个对象都有独立的控制权。
    AtomicSharedPtr &operator=(const AtomicSharedPtr &rhs) noexcept {
      store(rhs.load());
      return *this;
    }
 
    AtomicSharedPtr &operator=(AtomicSharedPtr &&) = delete;     // 删除移动赋值运算符，不允许移动赋值
 
  private:
 
    std::shared_ptr<T> _ptr;    // 存储共享的智能指针
  };
 
} // namespace carla
