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
// 拷贝构造函数，用于通过另一个Token对象来初始化token_type对象
  token_type::token_type(const Token &rhs) {
    // 使用std::memcpy将rhs中的数据复制到当前对象的_token成员中  
    // 假设_token是一个结构体或类，且rhs.data[0u]开始的数据与之兼容
    std::memcpy(&_token, &rhs.data[0u], sizeof(_token));
  }
// 类型转换运算符，允许token_type对象被隐式转换为Token类型  
  token_type::operator Token() const {
    Token token;// 创建一个临时的Token对象  
    // 使用std::memcpy将_token成员的数据复制到token.data的开始位置  
    // 注意：这里假设token.data的大小足够容纳_token的数据
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
