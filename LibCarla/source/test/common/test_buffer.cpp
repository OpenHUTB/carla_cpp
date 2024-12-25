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
// 目的是验证 Buffer 类的 copy_from 方法在接受多种不同类型的缓冲区容器和单个缓冲区类型作为参数时，能否顺利通过编译。
// 通过在不同的代码块中使用各种不同类型的缓冲区相关数据结构来调用 copy_from 方法，检查编译过程中是否出现问题。
// 这有助于确保 Buffer 类在面对多种常见和不同使用场景下的编译兼容性。
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
// 此测试用例先创建多个缓冲区，将相同的字符串内容复制到每个缓冲区中，形成一个缓冲区序列
// 然后从这个缓冲区序列创建一个新的缓冲区，并验证新缓冲区的大小和内容是否符合预期
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
  // 使用之前构建好的包含多个 const_buffer 元素的 sequence 数组来创建一个新的 Buffer 对象，
  // 这个操作会将 sequence 中各个缓冲区的数据合并到新创建的缓冲区中，后续将验证合并后的结果是否正确。
  auto result = Buffer(sequence);
  ASSERT_EQ(result.size(), message.size());
  // 将新生成的缓冲区转换为字符串形式，并与之前拼接好的 message 字符串进行对比，验证内容是否一致。
  auto result_str = as_string(result);
  ASSERT_EQ(result_str, message);
}
// 测试将字符串转换为缓冲区和从缓冲区转换回字符串
// 这个测试用例先从一个给定的字符串创建缓冲区，验证缓冲区大小是否和字符串长度相等
// 然后再将缓冲区转换回字符串，验证转换后的字符串是否和原始字符串一致
TEST(buffer, to_from_string) {
  const std::string str = "The quick brown fox jumps over the lazy dog";
  // 从字符串创建缓冲区
  // 使用给定的字符串 str 通过 Buffer 类的构造函数创建一个缓冲区对象，
  // 此时缓冲区应该包含了与字符串对应的字符数据，并且其大小理论上应等于字符串的长度。
  Buffer buffer(str);
  ASSERT_EQ(buffer.size(), str.size());
  // 将缓冲区转换回字符串
  // 通过调用 as_string 函数（假设该函数的功能是将缓冲区转换为对应的字符串表示），将之前创建的缓冲区转换回字符串，
  // 然后与原始字符串 str 进行比较，验证转换后的字符串内容是否与原始字符串一致，以此来检验转换功能的正确性。
  const std::string result = as_string(buffer);
  ASSERT_EQ(result, str);
}
// 测试将向量转换为缓冲区和从缓冲区转换回向量
// 该测试用例先创建一个指定大小且填充了特定数据的向量，然后从这个向量创建缓冲区
// 接着从缓冲区中提取数据重新构建一个向量，最后验证新构建的向量和原始向量是否相等
TEST(buffer, to_from_vector) {
  constexpr auto size = 1000u;
  using T = size_t;
  std::vector<T> v;
  v.reserve(size);
  // 循环向向量中添加数据，从 0 开始依次递增，填充一个包含指定数量元素的向量，用于后续转换为缓冲区以及相关验证操作。
  for (auto i = 0u; i < size; ++i) {
    v.emplace_back(i);
  }
  // 从向量创建缓冲区
  // 通过调用 Buffer 类的构造函数（假设其支持从向量创建缓冲区的功能），将填充好数据的向量 v 转换为缓冲区对象，
  // 同时验证生成的缓冲区大小是否等于向量中元素所占的字节数总和（通过 sizeof(T) * size 计算），以此来检验转换时缓冲区大小设置的正确性。
  Buffer buffer(v);
  ASSERT_EQ(buffer.size(), sizeof(T) * size);
  auto begin = reinterpret_cast<const T *>(buffer.data());
  // 从缓冲区中提取数据，根据缓冲区的数据指针和元素类型 T，重新构建一个向量对象，
  // 这里通过指定起始和结束迭代器的方式，将缓冲区中的数据转换回向量表示形式，以便后续与原始向量进行对比验证。
  std::vector<T> result(begin, begin + size);
  ASSERT_EQ(result, v);
}
// 测试缓冲区的复制
// 此测试用例先创建一个随机内容的缓冲区，然后创建一个空缓冲区并将前者的内容复制过来
// 最后验证两个缓冲区的大小和内容是否一致
TEST(buffer, copy) {
  auto msg = make_random(1024u);
  // 创建一个空的缓冲区指针（假设 make_empty 函数返回一个指向可用于后续操作的缓冲区的指针，初始为空），用于接收复制的数据。
  auto cpy = make_empty();
  cpy->copy_from(*msg);
  ASSERT_EQ(msg->size(), cpy->size());
  ASSERT_EQ(*cpy, *msg);
}
// 测试带偏移量的缓冲区复制
// 这个测试用例先创建一个缓冲区，然后使用指定的偏移量将一部分数据复制到缓冲区中
// 接着手动在缓冲区合适位置添加一些字符，最后验证缓冲区的大小和内容是否符合预期
TEST(buffer, copy_with_offset) {
  const char str0[] = "Hello";
  const char str1[] = "buffer!";
  Buffer buffer;
  auto offset = static_cast<Buffer::size_type>(sizeof(str0));
  // 使用 copy_from 方法，在指定的偏移量位置，将 str1 字符串对应的字符数据复制到 buffer 缓冲区中，
    // 这里涉及到根据偏移量计算正确的复制位置以及处理不同长度的数据复制操作，后续将验证整体缓冲区内容是否正确。
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
// 该测试用例先创建一个随机内容的缓冲区，然后创建一个相同大小的空缓冲区
// 接着使用memcpy函数将原缓冲区内容复制到新缓冲区，最后验证两个缓冲区内容是否一致
TEST(buffer, memcpy) {
  auto msg = make_random(1024u);
  // 创建一个与 msg 缓冲区大小相同的空缓冲区指针（通过调用 make_empty 函数并指定大小），用于后续使用 memcpy 进行数据复制。
  auto cpy = make_empty(msg->size());
  ASSERT_EQ(msg->size(), cpy->size());
  auto buffer = cpy->buffer();
  std::memcpy(buffer.data(), msg->data(), buffer.size());
  ASSERT_EQ(*cpy, *msg);
}

#ifndef LIBCARLA_NO_EXCEPTIONS
// 测试缓冲区大小过大时抛出异常
// 这个测试用例验证当尝试创建一个过大尺寸的缓冲区（超出允许范围）时，是否会正确抛出std::invalid_argument异常
// 分别测试创建缓冲区对象和重置缓冲区大小时的情况
TEST(buffer, message_too_big) {
  ASSERT_THROW(Buffer(4294967296ul), std::invalid_argument);
  Buffer buf;
  ASSERT_THROW(buf.reset(4294967296ul), std::invalid_argument);
}
#endif // LIBCARLA_NO_EXCEPTIONS
// 测试缓冲区池
// 此测试用例先创建一个缓冲区池，从池中获取一个缓冲区并复制特定字符串进去
// 然后再次从池中获取缓冲区，验证其内容是否符合预期，以及获取不同的缓冲区时内容是否不同
// 最后通过重置缓冲区池来测试池中弱引用相关的情况
TEST(buffer, buffer_pool) {
  const std::string str = "Hello buffer!";
  // 创建一个共享指针指向 carla::BufferPool 对象，用于管理缓冲区的分配和复用等操作，后续将从这个池中获取缓冲区进行相关测试。
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
