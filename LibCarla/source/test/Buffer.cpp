// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Buffer.h" // 引入定义Buffer类的自定义头文件

#include <boost/random/independent_bits.hpp>  // 引入Boost库中的独立位随机数生成器
#include <climits>  // 引入C标准库，包含CHAR_BIT常量（每个字节的位数，通常是8）
#include <random>   // 引入C++标准库中的随机数生成器相关功能

// 定义util命名空间，其中包含buffer子命名空间，用于提供缓冲区相关的实用函数
namespace util {
namespace buffer {

// 生成指定大小的随机缓冲区
shared_buffer make_random(size_t size) {
    // 如果请求的缓冲区大小为0，则返回一个空缓冲区
    if (size == 0u) {
        return make_empty();
    }
    // 使用Boost库中的独立位随机数生成器和C++标准库中的随机设备生成随机字节序列
    // independent_bits_engine是一个随机数引擎，它生成指定位数的独立随机位
    using random_bytes_engine = boost::random::independent_bits_engine<
        std::random_device, // 使用std::random_device作为底层随机数源
        CHAR_BIT,  // 每个字节使用CHAR_BIT位（通常是8位）
        unsigned char>;  // 生成的随机数类型为unsigned char（一个字节）
    random_bytes_engine rbe; // 实例化随机字节生成引擎
    auto buffer = make_empty(size); // 创建一个指定大小的空缓冲区
    std::generate(buffer->begin(), buffer->end(), std::ref(rbe)); // 使用随机数引擎填充缓冲区
    return buffer; // 返回生成的随机缓冲区
}

// 将Buffer对象转换为十六进制字符串表示
std::string to_hex_string(const Buffer &buf, size_t length) {
    length = std::min(static_cast<size_t>(buf.size()), length); // 确保不会超出缓冲区实际大小
    // 分配一个足够大的字符数组来存储转换后的十六进制字符串
    auto buffer = std::make_unique<char[]>(2u * length + 1u); // 每个字节需要两个十六进制字符，加上一个字符串结束符
    // 遍历缓冲区的每个字节，并将其转换为十六进制字符串表示
    for (auto i = 0u; i < length; ++i) {
        sprintf(&buffer[2u * i], "%02x", buf.data()[i]); // 将每个字节格式化为两位十六进制数
    }
    // 如果指定的长度小于缓冲区的实际大小，则在字符串末尾添加省略号
    if (length < buf.size()) {
        return std::string(buffer.get()) + std::string("...");
    }
    return std::string(buffer.get()); // 返回十六进制字符串
}

} // namespace buffer
} // namespace util
