// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
/// \file
/// 包含CARLA流处理相关头文件和Boost.Asio网络库头文件。
#include "carla/Debug.h"/// 包含CARLA的调试功能相关定义。
#include "carla/streaming/EndPoint.h"/// 包含CARLA流处理的端点（EndPoint）类定义。
#include "carla/streaming/Token.h"/// 包含CARLA流处理的令牌（Token）类定义。
#include "carla/streaming/detail/Types.h"/// 包含CARLA流处理细节部分的类型定义。

#include <boost/asio/ip/address.hpp>/// 包含Boost.Asio库中的IP地址类定义。
#include <boost/asio/ip/tcp.hpp>/// 包含Boost.Asio库中的TCP协议类定义。
#include <boost/asio/ip/udp.hpp> /// 包含Boost.Asio库中的UDP协议类定义。
/**
 * @namespace carla::streaming::detail
 * @brief CARLA流传输模块的详细实现命名空间。
 */

 /**
  * @struct token_data
  * @brief 令牌数据结构，用于存储流传输的相关信息。
  * @ingroup carla::streaming::detail
  *
  * 此结构体用于存储流ID、端口号、协议类型和地址类型等流传输的基本信息。
  * 使用#pragma pack(push, 1)确保结构体成员紧密排列，无填充字节。
  */
namespace carla {
namespace streaming {
namespace detail {

#pragma pack(push, 1)

  struct token_data {
      /**
     * @brief 流ID，用于唯一标识一个流。
     */
    stream_id_type stream_id = 0u;
    /**
     * @brief 端口号，用于网络通信。
     */
    uint16_t port = 0u;
    /**
     * @brief 协议类型枚举，指示使用的传输协议。
     */
    enum class protocol : uint8_t {
      not_set,///< 未设置协议
      tcp,///< TCP协议
      udp ///< UDP协议
    } protocol = protocol::not_set;
    /**
    * @brief 地址类型枚举，指示IP地址的版本。
    */
    enum class address : uint8_t {
      not_set,///< 未设置地址
      ip_v4,///< IPv4地址
      ip_v6///< IPv6地址
    } address_type = address::not_set;
    /**
     * @brief 联合体，用于存储IPv4或IPv6地址。
     *
     * 根据address_type的值，此联合体可以存储IPv4或IPv6地址的字节表示。
     */
    union {
      boost::asio::ip::address_v4::bytes_type v4;///< IPv4地址的字节表示
      boost::asio::ip::address_v6::bytes_type v6;///< IPv6地址的字节表示
    } address;
  };

#pragma pack(pop)
  /**
 * @brief 静态断言，用于确保`token_data`结构体的大小与`Token::data`的大小相同。
 *
 * 此断言用于验证`token_data`结构体的大小是否符合预期，即不超过192字节，具体分配如下：
 * - IPv6地址：128字节
 * - 状态：16字节
 * - 端口：16字节
 * - 流ID：32字节
 *
 * 如果`token_data`的大小超过192字节，则编译时会报错。
 */
  static_assert(
      sizeof(token_data) == sizeof(Token::data),
      "Size shouldn't be more than"
      "  v6 address  : 128"
      "  + state     :  16"
      "  + port      :  16"
      "  + stream id :  32"
      "  -----------------"
      "                192");

  /**
  * @brief 序列化流端点类，包含客户端订阅流所需的所有必要信息。
  */
  class token_type {
  private:
      /**
     * @brief 根据协议类型获取对应的协议枚举值。
     *
     * @tparam P 协议类型，必须是`boost::asio::ip::tcp`或`boost::asio::ip::udp`。
     * @return 如果P是`boost::asio::ip::tcp`，则返回`token_data::protocol::tcp`；如果是`boost::asio::ip::udp`，则返回`token_data::protocol::udp`。
     * @throws 编译时断言，如果P不是有效的协议类型，则会导致编译错误。
     */
    template <typename P>
    static constexpr auto get_protocol() {
      static_assert(
          std::is_same<P, boost::asio::ip::tcp>::value ||
          std::is_same<P, boost::asio::ip::udp>::value, "Invalid protocol.");
      return std::is_same<P, boost::asio::ip::tcp>::value ?
          token_data::protocol::tcp :
          token_data::protocol::udp;
    }
    /**
    * @brief 获取与令牌关联的端点。
    *
    * @tparam P 协议类型，必须是`boost::asio::ip::tcp`或`boost::asio::ip::udp`。
    * @return 返回与令牌关联的端点。
    * @throws 运行时断言，如果令牌无效或协议不匹配，则断言失败。
    */
    template <typename P>
    boost::asio::ip::basic_endpoint<P> get_endpoint() const {
      DEBUG_ASSERT(is_valid());// 假设is_valid()是一个检查令牌有效性的成员函数
      DEBUG_ASSERT(get_protocol<P>() == _token.protocol);// 检查协议是否匹配
      return {get_address(), _token.port};// 返回端点，包含地址和端口
    }

  public:
      /**
         * @brief 构造函数，根据给定的流ID和端点创建令牌。
         *
         * @tparam Protocol 端点使用的协议类型，必须是`boost::asio::ip::tcp`或`boost::asio::ip::udp`。
         * @param stream_id 流ID，用于唯一标识一个流。
         * @param ep 端点，包含地址和端口信息。
         */
    template <typename Protocol>
    explicit token_type(
        stream_id_type stream_id,
        const EndPoint<Protocol, FullyDefinedEndPoint> &ep) {
      _token.stream_id = stream_id;
      _token.port = ep.port();
      _token.protocol = get_protocol<Protocol>();
      set_address(ep.address());
    }

    template <typename Protocol>
    explicit token_type(
        stream_id_type stream_id,
        EndPoint<Protocol, PartiallyDefinedEndPoint> ep) {
      _token.stream_id = stream_id;
      _token.port = ep.port();
      _token.protocol = get_protocol<Protocol>();
    }


    token_type() = default;
    token_type(const token_type &) = default;

    token_type(const Token &rhs);

    explicit token_type(token_data data) {
      _token = data;
    }

    operator Token() const;

    // We need to return a reference here so we can use the address of the
    // stream id to send it as buffer.
    const auto &get_stream_id() const {
      return _token.stream_id;
    }

    void set_stream_id(stream_id_type id) {
      _token.stream_id = id;
    }

    bool has_address() const {
      return _token.address_type != token_data::address::not_set;
    }

    void set_address(const boost::asio::ip::address &addr);

    boost::asio::ip::address get_address() const;

    auto get_port() const {
      return _token.port;
    }

    bool is_valid() const {
      return has_address() &&
             ((_token.protocol != token_data::protocol::not_set) &&
             (_token.address_type != token_data::address::not_set));
    }

    bool address_is_v4() const {
      return _token.address_type == token_data::address::ip_v4;
    }

    bool address_is_v6() const {
      return _token.address_type == token_data::address::ip_v6;
    }

    bool protocol_is_udp() const {
      return _token.protocol == token_data::protocol::udp;
    }

    bool protocol_is_tcp() const {
      return _token.protocol == token_data::protocol::tcp;
    }

    template <typename Protocol>
    bool has_same_protocol(const boost::asio::ip::basic_endpoint<Protocol> &) const {
      return _token.protocol == get_protocol<Protocol>();
    }

    boost::asio::ip::udp::endpoint to_udp_endpoint() const {
      return get_endpoint<boost::asio::ip::udp>();
    }

    boost::asio::ip::tcp::endpoint to_tcp_endpoint() const {
      return get_endpoint<boost::asio::ip::tcp>();
    }

  private:

    friend class Dispatcher;

    token_data _token;
  };

} // namespace detail
} // namespace streaming
} // namespace carla
