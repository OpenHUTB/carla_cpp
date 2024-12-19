// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Buffer.h"

#include <boost/random/independent_bits.hpp>

#include <climits>
#include <random>
// 定义命名空间 util，其中包含子命名空间 buffer
namespace util {
namespace buffer {
// 生成指定大小的随机缓冲区
  shared_buffer make_random(size_t size) {
  	// 如果传入的大小为 0，则返回一个空的缓冲区
    if (size == 0u)
      return make_empty();
        // 使用 boost::random::independent_bits_engine 和 std::random_device 生成随机字节序列
        // independent_bits_engine 是一个随机数引擎，它使用 std::random_device 作为基础随机源
        // 生成指定位数（这里是 CHAR_BIT，通常是 8 位）的独立随机位，并产生无符号字符类型的随机数
    using random_bytes_engine = boost::random::independent_bits_engine<
        std::random_device,
        CHAR_BIT,
        unsigned char>;
    random_bytes_engine rbe;
    auto buffer = make_empty(size);
    std::generate(buffer->begin(), buffer->end(), std::ref(rbe));
    return buffer;
  }

  std::string to_hex_string(const Buffer &buf, size_t length) {
    length = std::min(static_cast<size_t>(buf.size()), length);
        // 创建一个足够大的字符数组来存储十六进制字符串
        // 长度为 2u * length + 1u，因为每个字节需要用两个十六进制数字表示，再加上一个字符串结束符
    auto buffer = std::make_unique<char[]>(2u * length + 1u);
    for (auto i = 0u; i < length; ++i)
      sprintf(&buffer[2u * i], "%02x", buf.data()[i]);
    if (length < buf.size())
      return std::string(buffer.get()) + std::string("...");
    return std::string(buffer.get());
  }

} // namespace buffer
} // namespace util
