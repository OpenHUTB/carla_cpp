// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/rpc/Metadata.h"

#include <rpc/client.h>

namespace carla {
namespace rpc {

  class Client {
  public:
//表示与远程服务进行交互的客户端
    template <typename... Args>
    explicit Client(Args &&... args)
      : _client(std::forward<Args>(args)...) {}
//std::forward 来确保参数的转发，避免不必要的拷贝操作
    void set_timeout(int64_t value) {
      _client.set_timeout(value);
    }

    auto get_timeout() const {
      return _client.get_timeout();
    }
//调用底层 rpc::client 对象的 set_timeout 方法来设置超时
    template <typename... Args>
    auto call(const std::string &function, Args &&... args) {
      return _client.call(function, Metadata::MakeSync(), std::forward<Args>(args)...);//这是一个同步调用
    }

    template <typename... Args>
    void async_call(const std::string &function, Args &&... args) {
      _client.async_call(function, Metadata::MakeAsync(), std::forward<Args>(args)...);
    }
  private:
//async_call 方法用于执行异步的 RPC 调用
    ::rpc::client _client;
  };

} // namespace rpc
} // namespace carla
//这种设计使得客户端和远程服务之间的通信变得简单且高效，适合用在像 Carla 这样的自动驾驶模拟器中
