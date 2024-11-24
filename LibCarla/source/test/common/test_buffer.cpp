// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "test.h"

#include <carla/Buffer.h>
#include <carla/BufferPool.h>

#include <array>
#include <list>
#include <set>
#include <string>
#include <vector>

using namespace util::buffer;
// 测试 Buffer 的编译兼容性
TEST(buffer, compile) {
  carla::Buffer buffer;
  { std::array<boost::asio::const_buffer, 3u> s; buffer.copy_from(s); }// 使用 std::array<boost::asio::const_buffer, 3u> 进行 copy_from 操作
  { std::array<boost::asio::mutable_buffer, 3u> s; buffer.copy_from(s); }// 使用 std::array<boost::asio::mutable_buffer, 3u> 进行 copy_from 操作
  { std::vector<boost::asio::const_buffer> s; buffer.copy_from(s); }// 使用 std::vector<boost::asio::const_buffer> 进行 copy_from 操作
  { std::vector<boost::asio::mutable_buffer> s; buffer.copy_from(s); } // 使用 std::vector<boost::asio::mutable_buffer> 进行 copy_from 操作
  { std::list<boost::asio::const_buffer> s; buffer.copy_from(s); }// 使用 std::list<boost::asio::const_buffer> 进行 copy_from 操作
  { std::list<boost::asio::mutable_buffer> s; buffer.copy_from(s); }// 使用 std::list<boost::asio::mutable_buffer> 进行 copy_from 操作
  { std::set<boost::asio::const_buffer> s; buffer.copy_from(s); }// 使用 std::set<boost::asio::const_buffer> 进行 copy_from 操作
  { std::set<boost::asio::mutable_buffer> s; buffer.copy_from(s); }// 使用 std::set<boost::asio::mutable_buffer> 进行 copy_from 操作

  { boost::asio::const_buffer v; buffer.copy_from(v); }// 使用单个 boost::asio::const_buffer 进行 copy_from 操作  
  { boost::asio::mutable_buffer v; buffer.copy_from(v); }// 使用单个 boost::asio::mutable_buffer 进行 copy_from 操作
  { int v[3u]; buffer.copy_from(v); }// 使用 int 数组进行 copy_from 操作
  { std::vector<int> v; buffer.copy_from(v); }// 使用 std::vector<int> 进行 copy_from 操作
  { std::string v; buffer.copy_from(v); }// 使用 std::string 进行 copy_from 操作
  { std::wstring v; buffer.copy_from(v); }// 使用 std::wstring 进行 copy_from 操作
  { struct C { int x = 0; } v[3u]; buffer.copy_from(v); }// 使用自定义结构体数组进行 copy_from 操作
  { struct C { int x = 0; }; std::array<C, 3u> v; buffer.copy_from(v); }// 使用自定义结构体 std::array 进行 copy_from 操作
  { struct C { int x = 0; }; std::vector<C> v; buffer.copy_from(v); }// 使用自定义结构体 std::vector 进行 copy_from 操作
}
// 测试从多个缓冲区序列复制到单个缓冲区
TEST(buffer, copy_buffer_sequence) {
  constexpr auto number_of_buffers = 15u;
  const std::string str = "WXdI<x->+<If$ua>$pu1AUBmS]?_PT{3z$B7L(E|?$]";
  std::string message;
  std::array<Buffer, number_of_buffers> buffers;
  std::array<boost::asio::const_buffer, number_of_buffers> sequence;
  for (auto i = 0u; i < number_of_buffers; ++i) {
    message += str;
    buffers[i].copy_from(str);
    sequence[i] = buffers[i].buffer();
  }
// 从多个缓冲区序列创建一个新的缓冲区
  auto result = Buffer(sequence);
  ASSERT_EQ(result.size(), message.size());
  auto result_str = as_string(result);
  ASSERT_EQ(result_str, message);
}
// 测试将字符串转换为缓冲区和从缓冲区转换回字符串
TEST(buffer, to_from_string) {
  const std::string str = "The quick brown fox jumps over the lazy dog";
// 从字符串创建缓冲区
  Buffer buffer(str);
  ASSERT_EQ(buffer.size(), str.size());
// 将缓冲区转换回字符串
  const std::string result = as_string(buffer);
  ASSERT_EQ(result, str);
}
// 测试将向量转换为缓冲区和从缓冲区转换回向量
TEST(buffer, to_from_vector) {
  constexpr auto size = 1000u;
  using T = size_t;
  std::vector<T> v;
  v.reserve(size);
  for (auto i = 0u; i < size; ++i) {
    v.emplace_back(i);
  }
// 从向量创建缓冲区
  Buffer buffer(v);
  ASSERT_EQ(buffer.size(), sizeof(T) * size);
  auto begin = reinterpret_cast<const T *>(buffer.data());
  std::vector<T> result(begin, begin + size);
  ASSERT_EQ(result, v);
}
// 测试缓冲区的复制
TEST(buffer, copy) {
  auto msg = make_random(1024u);
  auto cpy = make_empty();
  cpy->copy_from(*msg);
  ASSERT_EQ(msg->size(), cpy->size());
  ASSERT_EQ(*cpy, *msg);
}
// 测试带偏移量的缓冲区复制
TEST(buffer, copy_with_offset) {
  const char str0[] = "Hello";
  const char str1[] = "buffer!";
  Buffer buffer;
  auto offset = static_cast<Buffer::size_type>(sizeof(str0));
  buffer.copy_from(
      offset,
      reinterpret_cast<const unsigned char *>(&str1),
      std::strlen(str1));
  std::memcpy(buffer.data(), str0, std::strlen(str0));
  buffer[std::strlen(str0)] = ' ';
  auto str = std::string(str0) + " " + str1;
  ASSERT_EQ(buffer.size(), str.size());
  ASSERT_EQ(as_string(buffer), str.c_str());
}
// 测试使用 memcpy 进行缓冲区复制
TEST(buffer, memcpy) {
  auto msg = make_random(1024u);
  auto cpy = make_empty(msg->size());
  ASSERT_EQ(msg->size(), cpy->size());
  auto buffer = cpy->buffer();
  std::memcpy(buffer.data(), msg->data(), buffer.size());
  ASSERT_EQ(*cpy, *msg);
}

#ifndef LIBCARLA_NO_EXCEPTIONS
// 测试缓冲区大小过大时抛出异常
TEST(buffer, message_too_big) {
  ASSERT_THROW(Buffer(4294967296ul), std::invalid_argument);
  Buffer buf;
  ASSERT_THROW(buf.reset(4294967296ul), std::invalid_argument);
}
#endif // LIBCARLA_NO_EXCEPTIONS
// 测试缓冲区池
TEST(buffer, buffer_pool) {
  const std::string str = "Hello buffer!";
  auto pool = std::make_shared<carla::BufferPool>();
  {
    auto buff = pool->Pop();
    buff.copy_from(str);
  }
  auto buff1 = pool->Pop();
  ASSERT_EQ(as_string(buff1), str);
  auto buff2 = pool->Pop();
  ASSERT_NE(as_string(buff2), str);
  // 现在清空缓存池来测试缓存里面的弱引用
  pool.reset();
}
