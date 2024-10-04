// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once  // 防止头文件重复包含

#include "carla/Buffer.h"  // 包含 Buffer 的定义
#include "carla/Debug.h"   // 包含调试相关的定义
#include "carla/streaming/Token.h"  // 包含 Token 的定义

#include <memory>  // 包含智能指针支持

namespace carla {  // 定义 carla 命名空间
namespace streaming {  // 定义 streaming 子命名空间
namespace detail {  // 定义 detail 子命名空间

  class Dispatcher;  // 前向声明 Dispatcher 类

  template <typename StreamStateT>  // 模板类，类型参数为 StreamStateT
  class Stream {  // 定义 Stream 类
  public:

    Stream() = delete;  // 删除默认构造函数

    Stream(const Stream &) = default;  // 默认拷贝构造函数
    Stream(Stream &&) = default;  // 默认移动构造函数

    Stream &operator=(const Stream &) = default;  // 默认拷贝赋值运算符
    Stream &operator=(Stream &&) = default;  // 默认移动赋值运算符

    /// 与此流关联的 Token。客户端可使用该 Token 订阅此流。
    Token token() const {  // 获取流的 Token
      return _shared_state->token();  // 返回共享状态中的 Token
    }

    /// 从与此流关联的缓冲池中获取一个缓冲区。被丢弃的缓冲区将被重用以避免内存分配。
    ///
    /// @note 重用缓冲区针对所有通过流发送的消息都是大且（大致上）相同大小的用例进行了优化。
    Buffer MakeBuffer() {  // 创建缓冲区
      auto state = _shared_state;  // 获取共享状态
      return state->MakeBuffer();  // 返回从共享状态创建的缓冲区
    }

    /// 将 @a buffers 刷新到流中。不会进行复制。
    template <typename... Buffers>  // 支持多个缓冲区类型
    void Write(Buffers &&... buffers) {  // 写入缓冲区
      _shared_state->Write(std::move(buffers)...);  // 调用共享状态的写入方法
    }

    /// 复制 @a data 并将其刷新到流中。
    template <typename T>  // 类型模板
    Stream &operator<<(const T &data) {  // 重载输出运算符
      auto buffer = MakeBuffer();  // 创建一个缓冲区
      buffer.copy_from(data);  // 从数据复制内容到缓冲区
      Write(std::move(buffer));  // 将缓冲区写入流
      return *this;  // 返回当前对象
    }

    /// 检查是否有客户端正在监听
    bool AreClientsListening() {  // 判断客户端是否在监听
      return _shared_state ? _shared_state->AreClientsListening() : false;  // 返回共享状态的监听状态
    }

  private:

    friend class detail::Dispatcher;  // 声明 Dispatcher 为友元类，允许其访问私有成员

    Stream(std::shared_ptr<StreamStateT> state)  // 带参数的构造函数
      : _shared_state(std::move(state)) {  // 初始化共享状态
      DEBUG_ASSERT(_shared_state != nullptr);  // 调试断言，确保共享状态不为空
    }

    std::shared_ptr<StreamStateT> _shared_state;  // 共享状态指针
  };

} // namespace detail
} // namespace streaming
} // namespace carla
