// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once // 确保头文件只被包含一次

#include <boost/asio/io_context.hpp> // 引入ASIO库的io_context
#include <boost/asio/ip/address.hpp> // 引入IP地址相关的功能
#include <boost/asio/ip/tcp.hpp> // 引入TCP协议相关的功能
#include <boost/asio/ip/udp.hpp> // 引入UDP协议相关的功能

namespace carla {
namespace streaming {
namespace detail {

// 当不确定时，返回V4地址。
struct FullyDefinedEndPoint {}; // 完全定义的端点结构体
struct PartiallyDefinedEndPoint {}; // 部分定义的端点结构体

template <typename Protocol, typename EndPointType>
class EndPoint; // 声明模板类EndPoint

// 完全定义的端点类
template <typename Protocol>
class EndPoint<Protocol, FullyDefinedEndPoint> {
public:

    // 构造函数，接受一个基本端点并进行移动
    explicit EndPoint(boost::asio::ip::basic_endpoint<Protocol> ep)
      : _endpoint(std::move(ep)) {}

    // 返回地址
    auto address() const {
      return _endpoint.address();
    }

    // 返回端口
    uint16_t port() const {
      return _endpoint.port();
    }

    // 类型转换操作符，转换为基本端点类型
    operator boost::asio::ip::basic_endpoint<Protocol>() const {
      return _endpoint;
    }

private:
    // 存储基本端点
    boost::asio::ip::basic_endpoint<Protocol> _endpoint;
};

// 部分定义的端点类
template <typename Protocol>
class EndPoint<Protocol, PartiallyDefinedEndPoint> {
public:

    // 构造函数，接受端口号
    explicit EndPoint(uint16_t port) : _port(port) {}

    // 返回端口
    uint16_t port() const {
      return _port;
    }

    // 类型转换操作符，转换为基本端点类型，使用默认地址
    operator boost::asio::ip::basic_endpoint<Protocol>() const {
      return {Protocol::v4(), _port}; // 默认使用IPv4地址
    }

private:
    // 存储端口号
    uint16_t _port;
};

} // namespace detail

// 创建本地主机地址的静态方法
static inline auto make_localhost_address() {
    return boost::asio::ip::make_address("127.0.0.1"); // 返回127.0.0.1地址
}

// 根据字符串地址创建IP地址的静态方法
static inline auto make_address(const std::string &address) {
    boost::asio::io_context io_context; // 创建io_context对象
    boost::asio::ip::tcp::resolver resolver(io_context); // 创建解析器
    // 查询IPv4地址对应的主机名
    boost::asio::ip::tcp::resolver::query query(boost::asio::ip::tcp::v4(), address, "", boost::asio::ip::tcp::resolver::query::canonical_name);
    boost::asio::ip::tcp::resolver::iterator iter = resolver.resolve(query); // 执行解析
    boost::asio::ip::tcp::resolver::iterator end; // 结束迭代器
    while (iter != end) // 遍历解析结果
    {
        boost::asio::ip::tcp::endpoint endpoint = *iter++; // 获取当前端点
        return endpoint.address(); // 返回地址
    }
    return boost::asio::ip::make_address(address); // 如果没有找到，则返回给定地址
}

// 创建完全定义的端点的静态方法
template <typename Protocol>
static inline auto make_endpoint(boost::asio::ip::basic_endpoint<Protocol> ep) {
    return detail::EndPoint<Protocol, detail::FullyDefinedEndPoint>{std::move(ep)}; // 使用给定的端点创建EndPoint
}

// 根据字符地址和端口号创建端点的静态方法
template <typename Protocol>
static inline auto make_endpoint(const char *address, uint16_t port) {
    return make_endpoint<Protocol>({make_address(address), port}); // 创建端点
}

// 根据字符串地址和端口号创建端点的静态方法
template <typename Protocol>
static inline auto make_endpoint(const std::string &address, uint16_t port) {
    return make_endpoint<Protocol>(address.c_str(), port); // 调用重载方法
}

// 根据端口号创建部分定义的端点的静态方法
template <typename Protocol>
static inline auto make_endpoint(uint16_t port) {
    return detail::EndPoint<Protocol, detail::PartiallyDefinedEndPoint>{port}; // 创建部分定义的端点
}

} // namespace streaming
} // namespace carla
