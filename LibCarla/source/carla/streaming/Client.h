// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once// 预编译指令，防止头文件被重复包含。

#include "carla/Logging.h"// 包含 carla 库中的日志相关的头文件。
#include "carla/ThreadPool.h"// 包含 carla 库中的线程池相关的头文件。
#include "carla/streaming/Token.h"// 包含 carla 库中流处理相关的令牌（Token）头文件。

#include "carla/streaming/detail/tcp/Client.h"// 包含 carla 库中流处理细节中 TCP 客户端相关的头文件。
#include "carla/streaming/low_level/Client.h"// 包含 carla 库中流处理低层级客户端相关的头文件。

#include <boost/asio/io_context.hpp>// 包含 Boost.Asio 库中的输入输出上下文（io_context）头文件。

namespace carla {
namespace streaming {

using stream_token = detail::token_type;// 定义别名 stream_token 为 detail 命名空间中的 token_type。

// 注释：一个能够订阅多个流的客户端。
class Client {// 定义一个名为 Client 的类。
using underlying_client = low_level::Client<detail::tcp::Client>;// 定义一个类型别名 underlying_client，表示低层级客户端，该客户端使用 detail::tcp::Client 作为模板参数。

public:

    Client() = default;// 默认构造函数，不进行任何特殊操作。

    explicit Client(const std::string &fallback_address)
      : _client(fallback_address) {}
    // 带有一个字符串参数的构造函数，初始化内部的 _client 对象，传入的参数为备用地址（fallback_address）。

    ~Client() {
      _service.Stop();
    }
    // 析构函数，停止内部的线程池服务 _service。

    // 警告：不能对同一个流（即使是多流（MultiStream））订阅两次。
    template <typename Functor>
    void Subscribe(const Token &token, Functor &&callback) {
      _client.Subscribe(_service.io_context(), token, std::forward<Functor>(callback));
    }
    // 模板函数，用于订阅一个令牌（Token）对应的流，并传入一个回调函数（Functor），内部调用底层客户端的订阅方法，并传入线程池的输入输出上下文（io_context）、令牌和回调函数。

    void UnSubscribe(const Token &token) {
      _client.UnSubscribe(token);
    }
    // 函数，用于取消订阅一个令牌（Token）对应的流，内部调用底层客户端的取消订阅方法。

    void Run() {
      _service.Run();
    }
    // 函数，启动线程池服务，以同步方式运行。

    void AsyncRun(size_t worker_threads) {
      _service.AsyncRun(worker_threads);
    }
    // 函数，启动线程池服务，以异步方式运行，并指定工作线程数量。

private:

     // 注释：这两个参数的顺序非常重要。

    ThreadPool _service;// 定义一个线程池对象 _service。

    underlying_client _client; // 定义一个底层客户端对象 _client。

};

} // namespace streaming
} // namespace carla