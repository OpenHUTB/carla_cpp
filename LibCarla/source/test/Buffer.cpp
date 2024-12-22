//==============================================================================
// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.
//==============================================================================

#include "Buffer.h"

// 标准库
#include <climits>    // for CHAR_BIT
#include <random>     // for std::random_device

// Boost库
#include <boost/random/independent_bits.hpp>  // for independent_bits_engine

namespace util {
namespace buffer {

//------------------------------------------------------------------------------
// 随机缓冲区生成
//------------------------------------------------------------------------------

/**
 * @brief 生成指定大小的随机缓冲区
 * 
 * 使用boost::random::independent_bits_engine和std::random_device生成
 * 高质量的随机字节序列。每个字节都是独立生成的8位随机数。
 *
 * @param size 需要生成的随机缓冲区大小（字节）
 * @return shared_buffer 包含随机数据的共享缓冲区
 */
shared_buffer make_random(size_t size) {
    // 处理特殊情况：空缓冲区
    if (size == 0u) {
        return make_empty();
    }

    // 定义随机字节生成引擎类型
    using random_bytes_engine = boost::random::independent_bits_engine<
        std::random_device,    // 使用硬件随机源
        CHAR_BIT,             // 每次生成8位
        unsigned char>;       // 输出类型为无符号字符
    
    // 创建并填充随机缓冲区
    random_bytes_engine rbe;
    auto buffer = make_empty(size);
    std::generate(
        buffer->begin(),      // 缓冲区起始位置
        buffer->end(),        // 缓冲区结束位置
        std::ref(rbe)        // 使用引用包装器避免拷贝
    );
    
    return buffer;
}

//------------------------------------------------------------------------------
// 缓冲区转换工具
//------------------------------------------------------------------------------

/**
 * @brief 将缓冲区内容转换为十六进制字符串
 * 
 * 将Buffer对象的内容转换为可读的十六进制字符串表示。如果指定的长度
 * 小于缓冲区实际大小，则在末尾添加省略号(...)。
 *
 * @param buf 要转换的缓冲区对象
 * @param length 要转换的最大字节数
 * @return std::string 十六进制格式的字符串
 */
std::string to_hex_string(const Buffer& buf, size_t length) {
    // 确保不超过实际缓冲区大小
    length = std::min(static_cast<size_t>(buf.size()), length);
    
    // 分配足够的空间存储十六进制字符串
    // 每个字节需要2个字符，再加1个用于字符串结束符
    auto buffer = std::make_unique<char[]>(2u * length + 1u);
    
    // 转换每个字节为十六进制表示
    for (auto i = 0u; i < length; ++i) {
        sprintf(
            &buffer[2u * i],  // 写入位置
            "%02x",           // 格式：两位十六进制
            buf.data()[i]     // 源字节数据
        );
    }
    
    // 构造结果字符串
    if (length < buf.size()) {
        // 如果未转换完整缓冲区，添加省略号
        return std::string(buffer.get()) + "...";
    } else {
        // 返回完整的十六进制字符串
        return std::string(buffer.get());
    }
}

} // namespace buffer
} // namespace util
