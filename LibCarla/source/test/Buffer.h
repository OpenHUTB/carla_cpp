// 版权所有 （c） 2017 Universitat Autonoma 计算机视觉中心 （CVC）
// 巴塞罗那 （UAB）。
//
// 本作品根据 MIT 许可证的条款进行许可。
//有关副本，请参阅 <https://opensource.org/licenses/MIT>。

#pragma once
// 引入 carla 的 Buffer 类
#include <carla/Buffer.h>
// 引入一些标准库头文件和 boost 的 asio 库头文件
#include <algorithm>
#include <memory>
#include <ostream>
#include <string>
// 定义一个命名空间 util，用于存放实用工具函数
namespace util {
// 在 util 命名空间下再定义一个命名空间 buffer，用于存放与缓冲区相关的工具函数
namespace buffer {
// 使用 carla 命名空间中的 Buffer 类
  using carla::Buffer;
// 定义两个类型别名，分别是指向 Buffer 的共享指针和指向常量 Buffer 的共享指针
  using shared_buffer = std::shared_ptr<Buffer>;
  using const_shared_buffer = std::shared_ptr<const Buffer>;
// 静态内联函数，创建一个空的共享指针指向的 Buffer，如果指定了大小则创建指定大小的 Buffer
  static inline shared_buffer make_empty(size_t size = 0u) {
    return size == 0u ?
        std::make_shared<Buffer>() :
        std::make_shared<Buffer>(size);
  }
// 声明一个函数，用于创建随机内容的 Buffer
  shared_buffer make_random(size_t size);
// 模板函数，将一个给定的缓冲区对象转换为共享指针指向的 Buffer
  template <typename T>
  static inline shared_buffer make(const T &buffer) {
    return std::make_shared<Buffer>(boost::asio::buffer(buffer));
  }
// 将一个 Buffer 对象转换为字符串
  static inline std::string as_string(const Buffer &buf) {
    return {reinterpret_cast<const char *>(buf.data()), buf.size()};
  }
// 将一个 Buffer 对象转换为十六进制字符串，可指定输出的长度
  std::string to_hex_string(const Buffer &buf, size_t length = 16u);

} //命名空间缓冲区
} // 命名空间实用程序
// 为 carla 命名空间定义输出流操作符重载
namespace carla {
// 重载输出流操作符，用于将 Buffer 对象输出到输出流中
  static inline std::ostream &operator<<(std::ostream &out, const Buffer &buf) {
    out << "[" << buf.size() << " bytes] " << util::buffer::to_hex_string(buf);
    return out;
  }
// 重载相等操作符，用于比较两个 Buffer 对象是否相等
  static inline bool operator==(const Buffer &lhs, const Buffer &rhs) {
    return
        (lhs.size() == rhs.size()) &&
        std::equal(lhs.begin(), lhs.end(), rhs.begin());
  }
// 重载不等操作符，用于比较两个 Buffer 对象是否不相等
  static inline bool operator!=(const Buffer &lhs, const Buffer &rhs) {
    return !(lhs == rhs);
  }

} //命名空间 carla
