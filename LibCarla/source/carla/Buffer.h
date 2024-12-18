// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
// 确保此头文件只被包含一次

#include "carla/Debug.h"
// 包含Carla的调试相关的头文件
#include "carla/Exception.h"
// 包含Carla的异常处理相关的头文件
#include "carla/Logging.h"
// 包含Carla的日志记录相关的头文件

#include <boost/asio/buffer.hpp>
// 包含Boost.Asio的缓冲区相关的头文件，用于处理缓冲区操作

#include <cstdint>
// 包含标准整数类型头文件，用于定义固定宽度的整数类型
#include <limits>
// 包含数值极限相关的头文件，用于获取数据类型的极限值
#include <memory>
// 包含内存管理相关的头文件，如智能指针
#include <type_traits>
// 包含类型特征相关的头文件，用于在编译时获取类型信息

#ifdef LIBCARLA_INCLUDED_FROM_UE4
// 如果是从UE4中包含此库
#include <compiler/enable-ue4-macros.h>
// 包含启用UE4宏的头文件
#include "Containers/Array.h"
// 包含UE4的数组容器相关的头文件
#include <compiler/disable-ue4-macros.h>
// 包含禁用UE4宏的头文件
#endif // 结束预处理器条件编译块

namespace carla {

  class BufferPool;
  class BufferView;

  /// 一块原始数据。
  /// 请注意，如果需要更多容量，则会分配一个新的内存块，并
  /// 删除旧的内存块。这意味着默认情况下，缓冲区只能增长。要释放内存，使用 `clear` 或 `pop`。

  /// 这是一个仅可移动的类型，设计为按值传递时成本较低。如果缓冲区
  /// 从BufferPool中检索，则在销毁时内存会自动推回到池中。

  /// @warning创建一个大于max_size() 的缓冲区是未定义的。
  class Buffer {

    // =========================================================================
    ///@name 成员类型
    // =========================================================================
    /// @{

  public:

    using value_type = unsigned char;
    // 定义缓冲区的值类型为无符号字符类型
    using size_type = uint32_t;
    // 定义缓冲区的大小类型为32位无符号整数
    using iterator = value_type *;
    // 定义迭代器类型为指向值类型的指针，用于遍历缓冲区内容
    using const_iterator = const value_type *;
    // 定义常量迭代器类型为指向常量值类型的指针，用于常量遍历缓冲区内容
    /// @}
    // =========================================================================
    /// @name 构造与析构
    // =========================================================================
    /// @{

  public:

    /// 创建一个空的缓冲区。
    Buffer() = default;
    // 使用默认构造函数创建一个空的Buffer对象，默认初始化成员变量

    /// 创建一个分配了 @a size字节的缓冲区。
    explicit Buffer(size_type size)
      : _size(size),
        _capacity(size),
        _data(std::make_unique<value_type[]>(size)) {}
    // 显式构造函数，接受一个size_type类型的参数size
    // 初始化_size和_capacity为size，表示缓冲区的大小和容量
    // 使用std::make_unique动态分配一块大小为size的value_type类型的数组，并将指针赋给_data

    /// @copydoc Buffer(size_type)
    explicit Buffer(uint64_t size)
      : Buffer([size]() {
          if (size > max_size()) {
            throw_exception(std::invalid_argument("message size too big"));
          }
          return static_cast<size_type>(size);
        }()) {}
    // 显式构造函数，接受一个64位无符号整数类型的参数size
    // 先检查size是否大于max_size()，如果是则抛出异常
    // 然后将size转换为size_type类型并调用Buffer(size_type)构造函数

    /// 将 @a source复制到此缓冲区中。分配所需的内存。
    template <typename T>
    explicit Buffer(const T &source) {
      copy_from(source);
    }
    // 模板构造函数，接受一个任意类型T的参数source
    // 调用copy_from函数将source的内容复制到缓冲区，内部会分配所需内存

    explicit Buffer(const value_type *data, size_type size) {
      copy_from(data, size);
    }
    // 显式构造函数，接受一个指向value_type类型的指针data和size_type类型的参数size
    // 调用copy_from函数将data指向的内存内容复制到缓冲区，内部会分配所需内存

    /// @copydoc Buffer(size_type)
    explicit Buffer(const value_type *data, uint64_t size)
      : Buffer(data, [size]() {
          if (size > max_size()) {
            throw_exception(std::invalid_argument("message size too big"));
          }
          return static_cast<size_type>(size);
        }()) {}
    // 显式构造函数，接受一个指向value_type类型的指针data和64位无符号整数类型的参数size
    // 先检查size是否大于max_size()，如果是则抛出异常
    // 然后将size转换为size_type类型并调用Buffer(const value_type *, size_type)构造函数

    Buffer(const Buffer &) = delete;
    // 禁用拷贝构造函数，表明此对象不支持拷贝构造

    Buffer(Buffer &&rhs) noexcept
      : _parent_pool(std::move(rhs._parent_pool)),
        _size(rhs._size),
        _capacity(rhs._capacity),
        _data(rhs.pop()) {}
    // 移动构造函数，接受一个右值引用rhs
    // 将rhs的_parent_pool通过std::move移动给当前对象的_parent_pool
    // 将rhs的_size和_capacity赋值给当前对象的_size和_capacity
    // 调用rhs的pop函数获取rhs中的数据指针并赋给当前对象的_data

    ~Buffer() {
      if (_capacity > 0u) {
        ReuseThisBuffer();
      }
    }
    // 析构函数，当_capacity大于0时调用ReuseThisBuffer函数进行缓冲区资源的处理

    /// @}
    // =========================================================================
    /// @name 任务赋值 
    // =========================================================================
    /// @{

  public:

    Buffer &operator=(const Buffer &) = delete;
    // 禁用拷贝赋值运算符，表明此对象不支持拷贝赋值

    Buffer &operator=(Buffer &&rhs) noexcept {
      _parent_pool = std::move(rhs._parent_pool);
      _size = rhs._size;
      _capacity = rhs._capacity;
      _data = rhs.pop();
      return *this;
    }
    // 移动赋值运算符，接受一个右值引用rhs
    // 将rhs的_parent_pool通过std::move移动给当前对象的_parent_pool
    // 将rhs的_size和_capacity赋值给当前对象的_size和_capacity
    // 调用rhs的pop函数获取rhs中的数据指针并赋给当前对象的_data
    // 返回当前对象的引用

    /// @}
    // =========================================================================
    /// @name 数据访问
    // =========================================================================
    /// @{

  public:

    /// 访问位置 @a i处的字节
    const value_type &operator[](size_t i) const {
      return _data[i];
    }
    // 重载常量下标运算符，返回位置i处的常量字节引用

    /// Access the byte at position @a i.
    value_type &operator[](size_t i) {
      return _data[i];
    }
    // 重载下标运算符，返回位置i处的字节引用

    /// 直接访问分配的内存，如果没有分配内存则返回 nullptr。
    const value_type *data() const noexcept {
      return _data.get();
    }
    // 返回常量的指向分配内存的指针，如果_data为空则返回nullptr

    /// Direct access to the allocated memory or nullptr if no memory is
    /// allocated.
    value_type *data() noexcept {
      return _data.get();
    }
    // 返回指向分配内存的指针，如果_data为空则返回nullptr

    /// 从这个缓冲区创建一个boost::asio::buffer。
    /// @warning Boost.Asio缓冲区不拥有数据的所有权。调用者必须确保使用Asio缓冲区时不能删除此缓冲区所持有的内存。
    boost::asio::const_buffer cbuffer() const noexcept {
      return {data(), size()};
    }
    // 创建一个常量的Boost.Asio缓冲区，包含当前缓冲区的数据和大小

    /// @copydoc cbuffer()
    boost::asio::const_buffer buffer() const noexcept {
      return cbuffer();
    }
    // 创建一个常量的Boost.Asio缓冲区，调用cbuffer函数

    /// @copydoc cbuffer()
    boost::asio::mutable_buffer buffer() noexcept {
      return {data(), size()};
    }
    // 创建一个可修改的Boost.Asio缓冲区，包含当前缓冲区的数据和大小

    /// @}
    // =========================================================================
    /// @name 容量
    // =========================================================================
    /// @{

  public:

    bool empty() const noexcept {
      return _size == 0u;
    }
    // 判断缓冲区是否为空，即_size是否为0

    size_type size() const noexcept {
      return _size;
    }
    // 返回缓冲区的大小，即当前存储的数据量

    static constexpr size_type max_size() noexcept {
      return (std::numeric_limits<size_type>::max)();
    }
    // 返回缓冲区大小类型的最大值，为常量表达式

    size_type capacity() const noexcept {
      return _capacity;
    }
    // 返回缓冲区的容量，即分配的内存大小

    /// @}
    // =========================================================================
    /// @name 迭代器
    // =========================================================================
    /// @{

  public:

    const_iterator cbegin() const noexcept {
      return _data.get();
    }
    // 返回常量迭代器的起始位置，即_data指向的内存起始位置

    const_iterator begin() const noexcept {
      return cbegin();
    }
    // 返回常量迭代器的起始位置，调用cbegin函数

    iterator begin() noexcept {
      return _data.get();
    }
    // 返回迭代器的起始位置，即_data指向的内存起始位置

    const_iterator cend() const noexcept {
      return cbegin() + size();
    }
    // 返回常量迭代器的结束位置，即起始位置加上缓冲区大小

    const_iterator end() const noexcept {
      return cend();
    }
    // 返回常量迭代器的结束位置，调用cend函数

    iterator end() noexcept {
      return begin() + size();
    }
    // 返回迭代器的结束位置，即起始位置加上缓冲区大小

    /// @}
    // =========================================================================
    /// @name 修改器
    // =========================================================================
    /// @{

  public:

    /// 重置缓冲区的大小。如果容量不足，当前内存将被丢弃，并分配一个新的大小为 @a size的内存块。
    /// allocated.
    void reset(size_type size) {
      if (_capacity < size) {
        log_debug("allocating buffer of", size, "bytes");
        _data = std::make_unique<value_type[]>(size);
        _capacity = size;
      }
      _size = size;
    }
    // 如果传入的size大于当前容量_capacity
    // 则打印调试信息，表示正在分配指定大小的缓冲区
    // 然后重新分配一块大小为size的内存，更新_capacity为size
    // 最后将_size设置为size，表示缓冲区的新大小

    /// @copydoc reset(size_type)
    void reset(uint64_t size) {
      if (size > max_size()) {
        throw_exception(std::invalid_argument("message size too big"));
      }
      reset(static_cast<size_type>(size));
    }
    // 如果传入的64位无符号整数size大于max_size()
    // 则抛出异常，表示消息大小太大
    // 否则将size转换为size_type类型并调用reset(size_type)函数

    /// 调整缓冲区的大小。如果容量不足，将分配一个新的大小为 @a size的内存块，并复制数据。
    void resize(uint64_t size) {
      if (_capacity < size) {
        std::unique_ptr<value_type[]> data = std::move(_data);
        uint64_t old_size = size;
        reset(size);
        copy_from(data.get(), static_cast<size_type>(old_size));
      }
      _size = static_cast<size_type>(size);
    }
    // 如果传入的size大于当前容量_capacity
    // 则先将当前_data保存到临时变量data中
    // 记录旧的size
    // 调用reset函数重新分配一块大小为size的内存
    // 然后调用copy_from函数将旧数据复制到新的缓冲区中
    // 最后将_size设置为转换后的size

    /// 释放此缓冲区的内容，并将其大小和容量设置为零。
    std::unique_ptr<value_type[]> pop() noexcept {
      _size = 0u;
      _capacity = 0u;
      return std::move(_data);
    }
    // 将_size和_capacity设置为0，表示缓冲区为空
    // 通过std::move返回_data，释放对原数据的所有权

    /// 清除此缓冲区的内容，并将其大小和容量设置为零。
    /// 删除已分配的内存。
    void clear() noexcept {
      pop();
    }
    // 调用pop函数清除缓冲区内容，释放内存并设置大小和容量为0

    /// @}
    // =========================================================================
    /// @name copy_from
    // =========================================================================
    /// @{

  public:

    /// 将 @a source复制到此缓冲区。如果需要，则分配内存。
    template <typename T>
    void copy_from(const T &source) {
      copy_from(0u, source);
    }
    // 模板函数，接受一个任意类型T的参数source
    // 调用copy_from的另一个重载函数，偏移量为0

    /// Copy @a size bytes of the memory pointed by @a data into this buffer.
    /// Allocates memory if necessary.
    void copy_from(const value_type *data, size_type size) {
      copy_from(0u, data, size);
    }
    // 函数接受一个指向value_type类型的指针data和size_type类型的参数size
    // 调用copy_from的另一个重载函数，偏移量为0

    /// 将 @a source复制到此缓冲区，在前面留出 @a offset字节未初始化的空间。
    /// 如果需要，则分配内存。
    void copy_from(size_type offset, const Buffer &rhs) {
      copy_from(offset, rhs.buffer());
    }
    // 函数接受一个size_type类型的偏移量offset和一个常量Buffer引用rhs
    // 调用copy_from的另一个重载函数，将rhs转换为缓冲区类型

    /// @copydoc copy_from(size_type, const Buffer &)
    template <typename T>
    typename std::enable_if<boost::asio::is_const_buffer_sequence<T>::value>::type
    copy_from(size_type offset, const T &source) {
      reset(boost::asio::buffer_size(source) + offset);
      DEBUG_ASSERT(boost::asio::buffer_size(source) == size() - offset);
      DEBUG_ONLY(auto bytes_copied = )
      boost::asio::buffer_copy(buffer() + offset, source);
      DEBUG_ASSERT(bytes_copied == size() - offset);
    }
    // 如果类型T是常量缓冲区序列类型
    // 先根据source的大小和偏移量重置缓冲区大小
    // 调试断言source的大小等于缓冲区剩余大小（总大小减去偏移量）
    // 调试时记录复制的字节数
    // 调试断言复制的字节数等于缓冲区剩余大小

    /// @copydoc copy_from(size_type, const Buffer &)
    template <typename T>
    typename std::enable_if<!boost::asio::is_const_buffer_sequence<T>::value>::type
    copy_from(size_type offset, const T &source) {
      copy_from(offset, boost::asio::buffer(source));
    }
    // 如果类型T不是常量缓冲区序列类型
    // 则将source转换为缓冲区类型并调用copy_from的另一个重载函数

#ifdef LIBCARLA_INCLUDED_FROM_UE4
    /// @copydoc copy_from(size_type, const Buffer &)
    template <typename T>
    void copy_from(size_type offset, const TArray<T> &source) {
      copy_from(
          offset,
          reinterpret_cast<const value_type *>(source.GetData()),
          sizeof(T) * source.Num());
    }
#endif // LIBCARLA_INCLUDED_FROM_UE4
    // 如果是从UE4中包含此库，当类型T是TArray类型时
    // 根据source的内容和大小调用copy_from的另一个重载函数

    /// 将 @a data指向的内存中的 @a size字节复制到此缓冲区，
    /// 在前面留出 @a offset字节未初始化的空间。
    /// 如果需要，则分配内存。
    void copy_from(size_type offset, const value_type *data, size_type size) {
      copy_from(offset, boost::asio::buffer(data, size));
    }
    // 函数接受一个size_type类型的偏移量offset、一个指向value_type类型的指针data和size_type类型的参数size
    // 将data和size转换为缓冲区类型并调用copy_from的另一个重载函数

    /// @}

  private:

    void ReuseThisBuffer();
    // 私有函数，用于重新使用此缓冲区资源，具体实现未给出


    friend class BufferPool;
// 声明BufferPool类为友元类，这意味着BufferPool类可以访问Buffer类的私有和保护成员

friend class BufferView;
// 声明BufferView类为友元类，BufferView类可以访问Buffer类的私有和保护成员

std::weak_ptr<BufferPool> _parent_pool;
// 定义一个弱智能指针 _parent_pool，类型为指向BufferPool类的智能指针。
// 用于关联此缓冲区与某个BufferPool（可能是缓冲区的来源池），弱指针不会增加所指向对象的引用计数

size_type _size = 0u;
// 定义一个size_type类型（前面定义为uint32_t）的成员变量_size，并初始化为0，表示缓冲区当前存储的数据量为0字节

size_type _capacity = 0u;
// 定义一个size_type类型的成员变量_capacity，并初始化为0，表示缓冲区当前的容量为0字节

std::unique_ptr<value_type[]> _data = nullptr;
// 定义一个独占智能指针 _data，指向一个value_type类型（前面定义为unsigned char）的数组。
// 初始化为nullptr，表示当前没有分配用于存储数据的内存块。
// 这个指针将用于存储缓冲区中的实际数据内容
};

} // namespace carla
