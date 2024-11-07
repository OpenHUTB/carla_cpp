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

  static_assert(
      sizeof(token_data) == sizeof(Token::data),
      "Size shouldn't be more than"
      "  v6 address  : 128"
      "  + state     :  16"
      "  + port      :  16"
      "  + stream id :  32"
      "  -----------------"
      "                192");

  /// Serializes a stream endpoint. Contains all the necessary information for a
  /// client to subscribe to a stream.
  class token_type {
  private:

    template <typename P>
    static constexpr auto get_protocol() {
      static_assert(
          std::is_same<P, boost::asio::ip::tcp>::value ||
          std::is_same<P, boost::asio::ip::udp>::value, "Invalid protocol.");
      return std::is_same<P, boost::asio::ip::tcp>::value ?
          token_data::protocol::tcp :
          token_data::protocol::udp;
    }

    template <typename P>
    boost::asio::ip::basic_endpoint<P> get_endpoint() const {
      DEBUG_ASSERT(is_valid());
      DEBUG_ASSERT(get_protocol<P>() == _token.protocol);
      return {get_address(), _token.port};
    }

  public:
  
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
