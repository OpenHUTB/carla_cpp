// 版权所有 (c) 2023 巴塞罗那自治大学 (UAB) 的计算机视觉中心 (CVC)。
//
// 本作品依据 MIT 许可证的条款进行授权。
// 如需许可证副本，请访问 <https://opensource.org/licenses/MIT>。

#pragma once //这是一个预处理指令，用于确保当前头文件只被包含（include）一次，避免重复包含导致的编译错误。

// 包含CARLA项目相关的头文件
#include "carla/Buffer.h" // 包含Buffer类的定义，通常用于数据缓存。
#include "carla/Debug.h" // 包含调试相关的宏和函数定义。
#include "carla/Exception.h" // 包含异常类的定义，用于错误处理。
#include "carla/Logging.h" // 包含日志记录相关的宏和函数定义。

// 包含第三方库的头文件
#include <boost/asio/buffer.hpp> // 包含Boost.Asio库中用于处理缓冲区的功能。

// 包含标准库的头文件
#include <cstdint> // 包含标准整数类型的定义。
#include <limits> // 包含各种类型属性的限制值，如最大值和最小值。
#include <memory> // 包含内存管理相关的类和函数，如智能指针。
#include <type_traits> // 包含类型特征相关的模板类和函数，用于类型检查和转换。

#ifdef LIBCARLA_INCLUDED_FROM_UE4
#include <compiler/enable-ue4-macros.h> // 启用UE4特定的编译器宏
#include "Containers/Array.h" // 包含UE4的Array容器类，用于存储和管理动态数组。
#include <compiler/disable-ue4-macros.h> // 禁用UE4特定的编译器宏
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

    BufferView() = delete;  // 删除默认构造函数
    BufferView(const BufferView &) = delete; // 删除拷贝构造函数

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
    ///
    /// 这个成员函数返回一个`boost::asio::const_buffer`对象，该对象封装了内部数据缓冲区。
    /// 它通过调用`cbuffer()`成员函数来获取底层缓冲区，并返回一个不可变的缓冲区视图。
    /// 这个函数被标记为`noexcept`，意味着它保证不会抛出异常。
    boost::asio::const_buffer buffer() const noexcept {
      // 调用cbuffer()成员函数来获取底层的const缓冲区，并返回它。
      // 这里假设cbuffer()返回了一个适当的、用于asio库操作的缓冲区。
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
