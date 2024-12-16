// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Buffer.h" // 引入自定义的头文件（可能是定义了 Buffer 类型的相关内容）

#include <boost/random/independent_bits.hpp>  // 引入 Boost 库中的独立位生成引擎

#include <climits>  // 引入 C 标准库，包含常量，例如 CHAR_BIT（通常是 8）
#include <random>   // 提供标准 C++ 随机数生成器
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
        std::random_device, // 随机数源使用 std::random_device
        CHAR_BIT,  // 每个字节使用 CHAR_BIT 位（通常是 8 位）
        unsigned char>;  // 生成随机字节，类型为 unsigned char
    random_bytes_engine rbe; // 实例化随机字节生成引擎
    auto buffer = make_empty(size); // 创建一个指定大小的空缓冲区（调用 make_empty 函数）
    std::generate(buffer->begin(), buffer->end(), std::ref(rbe)); // 调用 boost 引擎生成随机字节
    return buffer; // 返回生成的随机缓冲区
  }
  // 将 Buffer 对象转换为十六进制字符串
  std::string to_hex_string(const Buffer &buf, size_t length) {
    length = std::min(static_cast<size_t>(buf.size()), length);
        // 创建一个足够大的字符数组来存储十六进制字符串
        // 长度为 2u * length + 1u，因为每个字节需要用两个十六进制数字表示，再加上一个字符串结束符
    auto buffer = std::make_unique<char[]>(2u * length + 1u);
     // 遍历缓冲区的每个字节，将其转化为十六进制字符串表示
    for (auto i = 0u; i < length; ++i)
      sprintf(&buffer[2u * i], "%02x", buf.data()[i]); // 格式化每个字节为 2 位十六进制数字（%02x）
    // 如果要转换的字节数小于缓冲区的总字节数，则在字符串末尾加上省略号
    if (length < buf.size())
      return std::string(buffer.get()) + std::string("...");
    return std::string(buffer.get()); // 返回转换后的十六进制字符串
  }

} // namespace buffer
} // namespace util
