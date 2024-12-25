// Copyright (c) 2023 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once// 确保此头文件只被包含一次


#include "carla/Buffer.h"// 包含 Carla 项目中定义的 Buffer 类或相关功能的头文件
#include "carla/Debug.h"// 包含 Carla 项目中用于调试目的的工具或宏的头文件
#include "carla/Exception.h"//包含 Carla 项目中自定义异常类的头文件
#include "carla/Logging.h"// 包含 Carla 项目中日志记录功能的头文件

#include <boost/asio/buffer.hpp>//包含 Boost.Asio 库中关于缓冲区操作的头文件

#include <cstdint>//包含标准库中定义的固定宽度整数类型
#include <limits>//包含标准库中定义的数值极限相关的模板类
#include <memory>//包含标准库中定义的智能指针类
#include <type_traits>//包含标准库中定义的类型特征模板

#ifdef LIBCARLA_INCLUDED_FROM_UE4
#include <compiler/enable-ue4-macros.h>
#include "Containers/Array.h"
#include <compiler/disable-ue4-macros.h>
#endif // LIBCARLA_INCLUDED_FROM_UE4

namespace carla {

  class BufferPool;

   /// 从现有缓冲区创建一个常量视图
  class BufferView : public std::enable_shared_from_this<BufferView> {

    // =========================================================================
    /// @name Member types
    // =========================================================================
    /// @{

  public:
    // 缓冲区中值的类型
    using value_type = unsigned char;
    // 缓冲区大小的类型
    using size_type = uint32_t;
    // 常量迭代器类型
    using const_iterator = const value_type *;

    /// @}
    // =========================================================================
    /// @name Construction and destruction
    // =========================================================================
    /// @{

  public:

    BufferView() = delete;
    BufferView(const BufferView &) = delete;

    // 从一个临时缓冲区创建一个BufferView的智能指针
    static std::shared_ptr<BufferView> CreateFrom(Buffer &&buffer) {
      return std::shared_ptr<BufferView>(new BufferView(std::move(buffer)));
    }

  private:

    // 私有构造函数，接收一个临时缓冲区
    BufferView(Buffer &&rhs) noexcept
      : _buffer(std::move(rhs)) {}

    /// @}
    // =========================================================================
    /// @name Data access
    // =========================================================================
    /// @{

  public:

    // 访问位置@a i的字节
    const value_type &operator[](size_t i) const {
      return _buffer.data()[i];
    }

    // 直接访问分配的内存，如果没有分配内存则返回nullptrv
    const value_type *data() const noexcept {
      return _buffer.data();
    }

    // 将此缓冲区转换为boost::asio::buffer
    ///
    /// @warning Boost.Asio缓冲区不拥有数据，调用者必须确保在asio缓冲区不再使用之前不要删除这块内存
    boost::asio::const_buffer cbuffer() const noexcept {
      return {_buffer.data(), _buffer.size()};
    }

    /// @copydoc cbuffer()
    boost::asio::const_buffer buffer() const noexcept {
      return cbuffer();
    }

    /// @}
    // =========================================================================
    /// @name Capacity
    // =========================================================================
    /// @{

  public:
    // 检查容器是否为空，并返回结果。
    // 如果_buffer中没有元素，则返回true，表示容器为空；否则返回false。
    bool empty() const noexcept {
      return _buffer.size() == 0u;
    }
    // 返回容器中当前存储的元素数量。
    // 这个值表示容器内实际的元素个数。
    size_type size() const noexcept {
      return _buffer.size();
    }
    // 返回容器能存储的最大元素数量。
    // 这是一个静态函数，返回size_type能表示的最大值，通常代表理论最大容量。
    static constexpr size_type max_size() noexcept {
      return (std::numeric_limits<size_type>::max)();
    }
    // 返回容器当前分配的存储空间大小，即容量。
    // 这个值表示在不进行内存重新分配的情况下，容器能存储的元素数量。
    size_type capacity() const noexcept {
      return _buffer.capacity();
    }

    /// @}
    // =========================================================================
    /// @name Iterators
    // =========================================================================
    /// @{

  public:
    // 返回指向容器开始的常量迭代器。
    // 这个迭代器不能用于修改容器中的元素，只能用于读取。
    const_iterator cbegin() const noexcept {
      return _buffer.data();
    }
    // 返回指向容器开始的常量迭代器。
    // 这个迭代器不能用于修改容器中的元素，只能用于读取。
    const_iterator begin() const noexcept {
      return _buffer.cbegin();
    }
    // 返回指向容器末尾的常量迭代器。
    // 这个迭代器指向容器中最后一个元素的下一个位置，表示容器的逻辑结束。
    const_iterator cend() const noexcept {
      return _buffer.cbegin() + _buffer.size();
    }
    // 返回指向容器末尾的常量迭代器。
    // 这个迭代器指向容器中最后一个元素的下一个位置，表示容器的逻辑结束。
    const_iterator end() const noexcept {
      return _buffer.cend();
    }

  private:

    // 用于存储数据的缓冲区
    const Buffer _buffer;
  };

  // BufferView的共享智能指针
  using SharedBufferView = std::shared_ptr<BufferView>;

} // namespace carla
