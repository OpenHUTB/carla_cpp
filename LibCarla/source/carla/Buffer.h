// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Debug.h"
#include "carla/Exception.h"
#include "carla/Logging.h"

#include <boost/asio/buffer.hpp>

#include <cstdint>
#include <limits>
#include <memory>
#include <type_traits>

#ifdef LIBCARLA_INCLUDED_FROM_UE4
#include <compiler/enable-ue4-macros.h>
#include "Containers/Array.h"
#include <compiler/disable-ue4-macros.h>
#endif // LIBCARLA_INCLUDED_FROM_UE4

namespace carla {

  class BufferPool;
  class BufferView;

  /// 一块原始数据。
  /// 请注意，如果需要更多容量，则会分配一个新的内存块，并
  /// 删除旧的内存块。这意味着默认情况下，缓冲区只能增长。要释放内存，使用 `clear` 或 `pop`。
  
  /// 这是一个仅可移动的类型，设计为按值传递时成本较低。如果缓冲区
  /// 从 BufferPool 中检索，则在销毁时内存会自动推回到池中。
  
  /// @warning 创建一个大于 max_size() 的缓冲区是未定义的。
  class Buffer {

    // =========================================================================
    ///@name 成员类型
    // =========================================================================
    /// @{

  public:

    using value_type = unsigned char;

    using size_type = uint32_t;

    using iterator = value_type *;

    using const_iterator = const value_type *;

    /// @}
    // =========================================================================
    /// @name 构造与析构
    // =========================================================================
    /// @{

  public:

    /// 创建一个空的缓冲区。
    Buffer() = default;

    /// 创建一个分配了 @a size 字节的缓冲区。
    explicit Buffer(size_type size)
      : _size(size),
        _capacity(size),
        _data(std::make_unique<value_type[]>(size)) {}

    /// @copydoc Buffer(size_type)
    explicit Buffer(uint64_t size)
      : Buffer([size]() {
          if (size > max_size()) {
            throw_exception(std::invalid_argument("message size too big"));
          }
          return static_cast<size_type>(size);
        } ()) {}

    /// 将 @a source 复制到此缓冲区中。分配所需的内存。
    template <typename T>
    explicit Buffer(const T &source) {
      copy_from(source);
    }

    explicit Buffer(const value_type *data, size_type size) {
      copy_from(data, size);
    }

    /// @copydoc Buffer(size_type)
    explicit Buffer(const value_type *data, uint64_t size)
      : Buffer(data, [size]() {
          if (size > max_size()) {
            throw_exception(std::invalid_argument("message size too big"));
          }
          return static_cast<size_type>(size);
        } ()) {}

    Buffer(const Buffer &) = delete;

    Buffer(Buffer &&rhs) noexcept
      : _parent_pool(std::move(rhs._parent_pool)),
        _size(rhs._size),
        _capacity(rhs._capacity),
        _data(rhs.pop()) {}

    ~Buffer() {
      if (_capacity > 0u) {
        ReuseThisBuffer();
      }
    }

    /// @}
    // =========================================================================
    /// @name 任务赋值 
    // =========================================================================
    /// @{

  public:

    Buffer &operator=(const Buffer &) = delete;

    Buffer &operator=(Buffer &&rhs) noexcept {
      _parent_pool = std::move(rhs._parent_pool);
      _size = rhs._size;
      _capacity = rhs._capacity;
      _data = rhs.pop();
      return *this;
    }

    /// @}
    // =========================================================================
    /// @name 数据访问
    // =========================================================================
    /// @{

  public:

    /// 访问位置 @a i 处的字节
    const value_type &operator[](size_t i) const {
      return _data[i];
    }

    /// Access the byte at position @a i.
    value_type &operator[](size_t i) {
      return _data[i];
    }

    /// 直接访问分配的内存，如果没有分配内存则返回 nullptr。
    const value_type *data() const noexcept {
      return _data.get();
    }

    /// Direct access to the allocated memory or nullptr if no memory is
    /// allocated.
    value_type *data() noexcept {
      return _data.get();
    }

    /// 从这个缓冲区创建一个 boost::asio::buffer。
    /// @warning Boost.Asio 缓冲区不拥有数据的所有权。调用者必须确保使用 Asio 缓冲区时不能删除此缓冲区所持有的内存。
    boost::asio::const_buffer cbuffer() const noexcept {
      return {data(), size()};
    }

    /// @copydoc cbuffer()
    boost::asio::const_buffer buffer() const noexcept {
      return cbuffer();
    }

    /// @copydoc cbuffer()
    boost::asio::mutable_buffer buffer() noexcept {
      return {data(), size()};
    }

    /// @}
    // =========================================================================
    /// @name 容量
    // =========================================================================
    /// @{

  public:

    bool empty() const noexcept {
      return _size == 0u;
    }

    size_type size() const noexcept {
      return _size;
    }

    static constexpr size_type max_size() noexcept {
      return (std::numeric_limits<size_type>::max)();
    }

    size_type capacity() const noexcept {
      return _capacity;
    }

    /// @}
    // =========================================================================
    /// @name 迭代器
    // =========================================================================
    /// @{

  public:

    const_iterator cbegin() const noexcept {
      return _data.get();
    }

    const_iterator begin() const noexcept {
      return cbegin();
    }

    iterator begin() noexcept {
      return _data.get();
    }

    const_iterator cend() const noexcept {
      return cbegin() + size();
    }

    const_iterator end() const noexcept {
      return cend();
    }

    iterator end() noexcept {
      return begin() + size();
    }

    /// @}
    // =========================================================================
    /// @name 修改器
    // =========================================================================
    /// @{

  public:

     /// 重置缓冲区的大小。如果容量不足，当前内存将被丢弃，并分配一个新的大小为 @a size 的内存块。
    /// allocated.
    void reset(size_type size) {
      if (_capacity < size) {
        log_debug("allocating buffer of", size, "bytes");
        _data = std::make_unique<value_type[]>(size);
        _capacity = size;
      }
      _size = size;
    }

    /// @copydoc reset(size_type)
    void reset(uint64_t size) {
      if (size > max_size()) {
        throw_exception(std::invalid_argument("message size too big"));
      }
      reset(static_cast<size_type>(size));
    }

	    /// 调整缓冲区的大小。如果容量不足，将分配一个新的大小为 @a size 的内存块，并复制数据。
    void resize(uint64_t size) {
      if(_capacity < size) {
        std::unique_ptr<value_type[]> data = std::move(_data);
        uint64_t old_size = size;
        reset(size);
        copy_from(data.get(), static_cast<size_type>(old_size));
      }
      _size = static_cast<size_type>(size);
    }

    /// 释放此缓冲区的内容，并将其大小和容量设置为零。
    std::unique_ptr<value_type[]> pop() noexcept {
      _size = 0u;
      _capacity = 0u;
      return std::move(_data);
    }

    	/// 清除此缓冲区的内容，并将其大小和容量设置为零。
	    /// 删除已分配的内存。
    void clear() noexcept {
      pop();
    }

    /// @}
    // =========================================================================
    /// @name copy_from
    // =========================================================================
    /// @{

  public:

    /// 将 @a source 复制到此缓冲区。如果需要，则分配内存。
    template <typename T>
    void copy_from(const T &source) {
      copy_from(0u, source);
    }

    /// Copy @a size bytes of the memory pointed by @a data into this buffer.
    /// Allocates memory if necessary.
    void copy_from(const value_type *data, size_type size) {
      copy_from(0u, data, size);
    }

    /// 将 @a source 复制到此缓冲区，在前面留出 @a offset 字节未初始化的空间。
	  /// 如果需要，则分配内存。
    void copy_from(size_type offset, const Buffer &rhs) {
      copy_from(offset, rhs.buffer());
    }

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

    /// @copydoc copy_from(size_type, const Buffer &)
    template <typename T>
    typename std::enable_if<!boost::asio::is_const_buffer_sequence<T>::value>::type
    copy_from(size_type offset, const T &source) {
      copy_from(offset, boost::asio::buffer(source));
    }

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

    /// 将 @a data 指向的内存中的 @a size 字节复制到此缓冲区，
	  /// 在前面留出 @a offset 字节未初始化的空间。
	  /// 如果需要，则分配内存。
    void copy_from(size_type offset, const value_type *data, size_type size) {
      copy_from(offset, boost::asio::buffer(data, size));
    }

    /// @}

  private:

    void ReuseThisBuffer();

    friend class BufferPool;
    friend class BufferView;

    std::weak_ptr<BufferPool> _parent_pool;

    size_type _size = 0u;

    size_type _capacity = 0u;

    std::unique_ptr<value_type[]> _data = nullptr;
  };

} // namespace carla
