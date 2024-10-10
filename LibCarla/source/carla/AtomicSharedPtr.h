// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.
 
#pragma once
 
#include <memory>
 
namespace carla {

  /// AtomicSharedPtr是一个非常简单的原子操作智能指针类，支持线程安全的共享指针操作，使用了释放-获取（release-acquire）内存顺序。
  /// Release-Acquire确保一个线程的内存释放（release）在另一个线程的获取（acquire）之前完成。
  /// 它保证线程之间的数据同步，避免出现读写顺序错误，确保一个线程看到的数据是另一个线程已经更新过的。
  /// 参考：https://zhuanlan.zhihu.com/p/669908979
  template <typename T>
  class AtomicSharedPtr {
  public:
  	
 	// 构造函数，使用转发参数包将传入参数完美转发给shared_ptr进行初始化
    template <typename... Args>
    explicit AtomicSharedPtr(Args &&... args)
      : _ptr(std::forward<Args>(args)...) {}
      
 	// 拷贝构造函数，使用load()函数来获取当前智能指针的值并初始化
    AtomicSharedPtr(const AtomicSharedPtr &rhs)
      : _ptr(rhs.load()) {}
      
 	 // 删除移动构造函数，不允许对象的移动操作
    AtomicSharedPtr(AtomicSharedPtr &&) = delete;
    
 	// 存储新指针，使用release内存顺序，保证所有释放在操作获取之前完成
    void store(std::shared_ptr<T> ptr) noexcept {
      std::atomic_store_explicit(&_ptr, ptr, std::memory_order_release);
    }
 	
    void reset(std::shared_ptr<T> ptr = nullptr) noexcept {
      store(ptr);
    }
    
 	// 加载指针的当前值，使用acquire内存顺序
    std::shared_ptr<T> load() const noexcept {
      return std::atomic_load_explicit(&_ptr, std::memory_order_acquire);
    }
 	
 	// 原子比较并交换操作，如果当前指针值与预期值匹配，则替换为desired值
    bool compare_exchange(std::shared_ptr<T> *expected, std::shared_ptr<T> desired) noexcept {
      return std::atomic_compare_exchange_strong_explicit(
          &_ptr,
          expected,
          desired,
          std::memory_order_acq_rel,
          std::memory_order_acq_rel);  // 原子比较并交换
    } 
    // 重载赋值运算符，使用智能指针存储新值
    AtomicSharedPtr &operator=(std::shared_ptr<T> ptr) noexcept {
      store(std::move(ptr)); // 存储新指针
      return *this;  // 返回当前对象的引用
    }
    // 重载赋值运算符，支持复制赋值
    AtomicSharedPtr &operator=(const AtomicSharedPtr &rhs) noexcept {
      store(rhs.load());
      return *this;
    }
 
    AtomicSharedPtr &operator=(AtomicSharedPtr &&) = delete;
 
  private:
 
    std::shared_ptr<T> _ptr;
  };
 
} // namespace carla
