// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/streaming/detail/Token.h"

#include "carla/Exception.h"

#include <cstring>
#include <exception>

namespace carla {
namespace streaming {
namespace detail {

  void token_type::set_address(const boost::asio::ip::address &addr) {
    // 首先检查传入的地址是否是IPv4地址。
    if (addr.is_v4()) {
      // 如果是IPv4地址，则将token中的地址类型设置为IPv4。
      _token.address_type = token_data::address::ip_v4;
      // 并将IPv4地址转换为字节数组后存储到token的相应位置。
      _token.address.v4 = addr.to_v4().to_bytes();
    // 否则，检查传入的地址是否是IPv6地址。  
    } else if (addr.is_v6()) {
      // 如果是IPv6地址，则将token中的地址类型设置为IPv6。
      _token.address_type = token_data::address::ip_v6;
      // 并将IPv6地址转换为字节数组后存储到token的相应位置。  
      _token.address.v6 = addr.to_v6().to_bytes();
    // 如果传入的地址既不是IPv4也不是IPv6，则抛出异常。
    } else {
      // 抛出一个std::invalid_argument异常，异常信息为"invalid ip address!"，表示传入了无效的IP地址。
      throw_exception(std::invalid_argument("invalid ip address!"));
    }
  }

  token_type::token_type(const Token &rhs) {
    std::memcpy(&_token, &rhs.data[0u], sizeof(_token));
  }

  token_type::operator Token() const {
    Token token;
    std::memcpy(&token.data[0u], &_token, token.data.size());
    return token;
  }

  boost::asio::ip::address token_type::get_address() const {
    if (_token.address_type == token_data::address::ip_v4) {
      return boost::asio::ip::address_v4(_token.address.v4);
    }
    return boost::asio::ip::address_v6(_token.address.v6);
  }

} // namespace detail
} // namespace streaming
} // namespace carla
