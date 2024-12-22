// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
/// @file
/// @brief 包含 Carla 框架中与网络流相关的类和函数的声明。
/// 
/// 此文件包含了 Carla 框架中用于网络通信的一些基础类和函数的声明，
/// 包括列表视图、缓冲区、缓冲区视图、调试工具、不可复制类以及网络流相关的类型定义。
#include "carla/ListView.h"/// @brief 包含 ListView 类的声明，用于管理数据的列表视图。
#include "carla/Buffer.h" /// @brief 包含 Buffer 类的声明，用于存储和管理数据缓冲区。
#include "carla/BufferView.h"/// @brief 包含 BufferView 类的声明，提供对 Buffer 中数据的只读视图。
#include "carla/Debug.h"/// @brief 包含调试工具的声明，用于输出调试信息和进行断言检查。
#include "carla/NonCopyable.h"/// @brief 包含 NonCopyable 类的声明，用于禁止类的拷贝操作。
#include "carla/streaming/detail/Types.h"/// @brief 包含网络流相关的类型定义。

#include <boost/asio/buffer.hpp>/// @brief 包含 Boost.Asio 库中用于处理网络缓冲区的函数和类型。

#include <array>/// @brief 包含 std::array 容器的声明，用于存储固定大小的数组。
#include <exception>/// @brief 包含标准异常类的声明，用于处理异常情况。
#include <limits>/// @brief 包含标准库中与限制和常量相关的定义，如数值类型的最大值和最小值。
#include <memory>/// @brief 包含内存管理相关的类和函数，如智能指针。
#include <string>/// @brief 包含 std::string 类的声明，用于处理字符串数据。

namespace carla {
namespace streaming {
namespace detail {
namespace tcp {

    /// @brief 通过TCP套接字发送的一组缓冲区的序列化，作为单个消息发送。
   ///        模板参数@a MaxNumberOfBuffers在编译时限制单个消息中可以包含的缓冲区的最大数量。
   ///
   /// @tparam MaxNumberOfBuffers 单个消息中可以包含的缓冲区的最大数量（编译时常量）。
  template <size_t MaxNumberOfBuffers>
  class MessageTmpl
    : public std::enable_shared_from_this<MessageTmpl<MaxNumberOfBuffers>>,
      private NonCopyable {
  public:
      /// @brief 获取单个消息中可以包含的缓冲区的最大数量。
    ///
    /// @return 返回单个消息中可以包含的缓冲区的最大数量。
    static constexpr size_t max_size() {
      return MaxNumberOfBuffers;
    }

  private:
      /// @brief 私有构造函数，用于限制外部直接创建对象。
   ///
   /// @param size 预留参数，可能用于内部初始化。
    MessageTmpl(size_t) {}
    /// @brief 模板化的构造函数，用于初始化消息对象。
    ///
    /// @param size 缓冲区总数（包括传入的和后续参数中的）。
    /// @param buffer 第一个缓冲区（类型为SharedBufferView）。
    /// @param buffers 可变数量的其他缓冲区。
    ///
    /// @note 该构造函数通过递归调用自身来处理可变数量的缓冲区参数。
    template <typename... Buffers>
    MessageTmpl(size_t size, SharedBufferView buffer, Buffers... buffers)
      : MessageTmpl(size, buffers...) {
      ++_number_of_buffers;
      _total_size += buffer->size();
      _buffer_views[1u + size - _number_of_buffers] = buffer->cbuffer();
      _buffers[size - _number_of_buffers] = buffer;
    }

  public:
      /// @brief 模板化的构造函数，用于创建包含多个缓冲区的消息对象。
    ///
    /// @param buf 第一个缓冲区（类型为SharedBufferView）。
    /// @param buffers 可变数量的其他缓冲区。
    ///
    /// @note 该构造函数通过递归调用自身（利用模板参数包展开）来处理可变数量的缓冲区参数。
    ///       同时，它使用`sizeof...(Buffers)`来获取传入的缓冲区数量，并加1以包括第一个缓冲区。
    ///       在构造之前，会进行静态断言以确保传入的缓冲区数量不超过最大允许值。
    ///
    /// @throws 编译时断言失败，如果传入的缓冲区数量超过`max_size()`返回的值。
    template <typename... Buffers>
    MessageTmpl(SharedBufferView buf, Buffers... buffers)
      : MessageTmpl(sizeof...(Buffers) + 1u, buf, buffers...) {
      static_assert(sizeof...(Buffers) < max_size(), "Too many buffers!");
      // 设置第一个_buffer_view为_total_size的缓冲区
      _buffer_views[0u] = boost::asio::buffer(&_total_size, sizeof(_total_size));
    }

    /// @brief 获取消息的大小（不包括头部。
    ///
    /// @return 返回消息的大小（以字节为单位。
    auto size() const noexcept {
      return _total_size;
    }
    /// @brief 检查消息是否为空。
    ///
    /// @return 如果消息大小为0，则返回true；否则返回false。
    bool empty() const noexcept {
      return size() == 0u;
    }
    /// @brief 获取消息的缓冲区序列。
    ///
    /// @return 返回一个包含所有缓冲区视图的视图对象（可能是`std::vector<boost::asio::const_buffer>`或类似类型，具体取决于`MakeListView`的实现）。
    ///         注意，返回的视图包括_total_size的缓冲区视图以及所有传入的缓冲区视图。
    auto GetBufferSequence() const {
      auto begin = _buffer_views.begin();
      // 创建一个包含_number_of_buffers + 1个缓冲区视图的视图
      return MakeListView(begin, begin + _number_of_buffers + 1u);
    }

  private:
      /// @brief 缓冲区数量（不包括_total_size的缓冲区）。
    message_size_type _number_of_buffers = 0u;
    /// @brief 消息的总大小（以字节为单位，不包括头部）。
    message_size_type _total_size = 0u;
    /// @brief 存储所有传入的缓冲区对象的数组。
    std::array<SharedBufferView, MaxNumberOfBuffers> _buffers;
    /// @brief 存储所有缓冲区视图的数组，包括_total_size的缓冲区视图。
    ///        注意，数组大小比_buffers多一个，以容纳_total_size的缓冲区视图。
    std::array<boost::asio::const_buffer, MaxNumberOfBuffers + 1u> _buffer_views;
  };

  // class MessageTmpl

/// @brief 一个TCP消息类型，最多包含2个缓冲区。这通常用于包含头部和主体的消息。
///
/// @note 该类型是通过将`MessageTmpl`模板的特化参数设置为2来创建的。
  using Message = MessageTmpl<2u>;

} // namespace tcp
} // namespace detail
} // namespace streaming
} // namespace carla
