// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/streaming/detail/Token.h"
#include "carla/streaming/detail/tcp/Client.h"

#include <boost/asio/io_context.hpp>

#include <memory>	// 引入C++标准库的内存管理头文件
#include <unordered_map>	// 引入C++标准库的无序映射容器头文件

namespace carla {
namespace streaming {
namespace low_level {

  /// A client able to subscribe to multiple streams. Accepts an external
  /// io_context.
  ///
  /// @warning The client should not be destroyed before the @a io_context is
  /// stopped.
  template <typename T>
  class Client {
  public:
  	
    using underlying_client = T;
    using protocol_type = typename underlying_client::protocol_type;
    using token_type = carla::streaming::detail::token_type;	// 使用类型别名来简化代码中对模板参数T及相关类型的引用

    explicit Client(boost::asio::ip::address fallback_address)
      : _fallback_address(std::move(fallback_address)) {}	// 构造函数，接受一个boost::asio::ip::address类型的备用地址作为参数
	
    explicit Client(const std::string &fallback_address)
      : Client(carla::streaming::make_address(fallback_address)) {}		// 构造函数，接受一个字符串形式的备用地址作为参数，会先将其转换为boost::asio::ip::address类型，再调用另一个构造函数进行初始化
	
    explicit Client()
      : Client(carla::streaming::make_localhost_address()) {}	// 默认构造函数，会先获取本地主机地址作为备用地址，再调用另一个构造函数进行初始化
	
    ~Client() {
      for (auto &pair : _clients) {
        pair.second->Stop();		// 析构函数，用于在对象销毁时清理资源，会遍历所有已订阅的客户端并调用它们的停止方法
      }
    }	

    /// @warning cannot subscribe twice to the same stream (even if it's a
    /// MultiStream).
    template <typename Functor>
    void Subscribe(
        boost::asio::io_context &io_context,
        token_type token,	// 订阅流的方法，接受io_context、令牌以及回调函数作为参数
        Functor &&callback) {
      DEBUG_ASSERT_EQ(_clients.find(token.get_stream_id()), _clients.end());	// 断言确保当前要订阅的流ID在已订阅客户端的映射容器中不存在，即不能两次订阅同一个流
      if (!token.has_address()) {
        token.set_address(_fallback_address);	// 如果传入的令牌没有地址，就将备用地址设置给令牌
      }
      auto client = std::make_shared<underlying_client>(	// 创建一个底层客户端的智能指针，并通过底层客户端的构造函数进行初始化，传入io_context、令牌和回调函数
          io_context,
          token,
          std::forward<Functor>(callback));
      client->Connect();	// 让客户端尝试连接到对应的流
      _clients.emplace(token.get_stream_id(), std::move(client));	// 将创建好的客户端智能指针以流ID为键存入到_clients映射容器中，以便后续管理和操作
    }

    void UnSubscribe(token_type token) {	// 取消订阅流的方法，接受一个令牌作为参数
      log_debug("calling sensor UnSubscribe()");	// 输出一条调试信息，表示正在调用取消订阅操作
      auto it = _clients.find(token.get_stream_id());	// 在已订阅客户端的映射容器中查找与传入令牌的流ID对应的客户端指针
      if (it != _clients.end()) {	// 如果找到了对应的客户端指针，就调用它的停止方法停止相关操作，并从映射容器中删除该客户端的记录
        it->second->Stop();
        _clients.erase(it);
      }
    }

  private:	

    boost::asio::ip::address _fallback_address;	// 存储备用的IP地址，在构造函数中进行初始化，可能在流连接出现问题需要使用备用地址时发挥作用

    std::unordered_map<	// 一个无序映射容器，存储底层客户端的智能指针，用于管理和操作订阅的各个流对应的客户端
        detail::stream_id_type,
        std::shared_ptr<underlying_client>> _clients;
  };

} // namespace low_level
} // namespace streaming
} // namespace carla
