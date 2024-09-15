// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/Exception.h"  // 包含 CARLA 异常处理的头文件
#include <boost/assert/source_location.hpp>  // 包含 Boost 库中的源位置定义

// =============================================================================
// -- Define boost::throw_exception --------------------------------------------
// =============================================================================

#ifdef BOOST_NO_EXCEPTIONS  // 如果编译器不支持异常

namespace boost {

  // 定义 boost::throw_exception，当异常被抛出时调用 carla::throw_exception
  void throw_exception(const std::exception &e) {
    carla::throw_exception(e);  // 调用 CARLA 的异常处理函数
  }

  // 定义重载版本，接受 Boost 的源位置作为参数，但不做额外处理
  void throw_exception(
      const std::exception &e,
      boost::source_location const & loc) {
    throw_exception(e);  // 直接调用之前定义的函数
  }

} // namespace boost

#endif // BOOST_NO_EXCEPTIONS

// =============================================================================
// -- Workaround for Boost.Asio bundled with rpclib ----------------------------
// =============================================================================

#ifdef ASIO_NO_EXCEPTIONS  // 如果 Boost.Asio 不支持异常

#include <exception>  // 包含标准异常处理相关的头文件
#include <system_error>  // 包含系统错误处理相关的头文件
#include <typeinfo>  // 包含类型信息相关的头文件

namespace clmdep_asio {  // 命名空间 clmdep_asio，表示与 Boost.Asio 兼容的部分
namespace detail {  // 命名空间 detail，表示内部实现细节

  // 模板函数定义，处理各种异常类型
  template <typename Exception>
  void throw_exception(const Exception& e) {
    carla::throw_exception(e);  // 调用 CARLA 的异常处理函数
  }

  // 明确实例化模板函数，指定处理的异常类型
  template void throw_exception<std::bad_cast>(const std::bad_cast &);  // 处理 std::bad_cast 异常
  template void throw_exception<std::exception>(const std::exception &);  // 处理 std::exception 异常
  template void throw_exception<std::system_error>(const std::system_error &);  // 处理 std::system_error 异常

} // namespace detail
} // namespace clmdep_asio

#endif // ASIO_NO_EXCEPTIONS

