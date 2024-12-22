//==============================================================================
// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.
//==============================================================================

#pragma once

#include <memory>  // for std::shared_ptr

namespace carla {

/**
 * @brief 线程安全的原子智能指针封装类
 *
 * AtomicSharedPtr提供了一个简单的原子操作智能指针实现，支持线程安全的共享指针操作。
 * 使用release-acquire内存序来确保线程间的正确同步：
 * - release操作确保在此之前的所有内存写入都对其他线程可见
 * - acquire操作确保在此之后的所有内存读取都能看到其他线程的release操作
 *
 * @tparam T 指针指向的数据类型
 */
template <typename T>
class AtomicSharedPtr {
public:
    //--------------------------------------------------------------------------
    // 构造函数
    //--------------------------------------------------------------------------

    /**
     * @brief 构造函数，完美转发参数到shared_ptr
     * @tparam Args 可变参数包类型
     * @param args 构造参数
     */
    template <typename... Args>
    explicit AtomicSharedPtr(Args&&... args)
        : _ptr(std::forward<Args>(args)...) {}

    /**
     * @brief 拷贝构造函数
     * @param rhs 源对象
     */
    AtomicSharedPtr(const AtomicSharedPtr& rhs)
        : _ptr(rhs.load()) {}

    /// 禁用移动构造函数
    AtomicSharedPtr(AtomicSharedPtr&&) = delete;

    //--------------------------------------------------------------------------
    // 原子操作方法
    //--------------------------------------------------------------------------

    /**
     * @brief 原子存储新的智能指针
     * @param ptr 要存储的新指针
     */
    void store(std::shared_ptr<T> ptr) noexcept {
        std::atomic_store_explicit(
            &_ptr, 
            ptr,
            std::memory_order_release  // 确保之前的写入对其他线程可见
        );
    }

    /**
     * @brief 重置指针值
     * @param ptr 新的指针值，默认为nullptr
     */
    void reset(std::shared_ptr<T> ptr = nullptr) noexcept {
        store(ptr);
    }

    /**
     * @brief 原子加载当前指针值
     * @return 当前存储的智能指针
     */
    std::shared_ptr<T> load() const noexcept {
        return std::atomic_load_explicit(
            &_ptr,
            std::memory_order_acquire  // 确保能看到其他线程的release操作
        );
    }

    /**
     * @brief 原子比较并交换操作
     * @param expected 预期的指针值
     * @param desired 希望设置的新值
     * @return 操作是否成功
     */
    bool compare_exchange(
        std::shared_ptr<T>* expected,
        std::shared_ptr<T> desired) noexcept {
        return std::atomic_compare_exchange_strong_explicit(
            &_ptr,
            expected,
            desired,
            std::memory_order_acq_rel,  // 同时具有acquire和release语义
            std::memory_order_acq_rel
        );
    }

    //--------------------------------------------------------------------------
    // 运算符重载
    //--------------------------------------------------------------------------

    /**
     * @brief 智能指针赋值运算符
     * @param ptr 要赋值的智能指针
     * @return 当前对象引用
     */
    AtomicSharedPtr& operator=(std::shared_ptr<T> ptr) noexcept {
        store(std::move(ptr));
        return *this;
    }

    /**
     * @brief 拷贝赋值运算符
     * @param rhs 源对象
     * @return 当前对象引用
     */
    AtomicSharedPtr& operator=(const AtomicSharedPtr& rhs) noexcept {
        store(rhs.load());
        return *this;
    }

    /// 禁用移动赋值运算符
    AtomicSharedPtr& operator=(AtomicSharedPtr&&) = delete;

private:
    std::shared_ptr<T> _ptr;  ///< 内部存储的智能指针
};

} // namespace carla
