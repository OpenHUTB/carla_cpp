// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/Exception.h"  // 引入Carla异常处理头文件
#include <boost/assert/source_location.hpp>  // 引入Boost的源位置头文件

// =============================================================================
// -- Define boost::throw_exception --------------------------------------------
// =============================================================================

#ifdef BOOST_NO_EXCEPTIONS  // 如果禁用异常

namespace boost {   // 在boost命名空间中

  void throw_exception(const std::exception &e) {  // 定义抛出异常函数
    carla::throw_exception(e);  // 调用Carla的抛出异常函数 
  }

  void throw_exception(  // 重载的抛出异常函数，带有源位置信息
      const std::exception &e,
      boost::source_location const & loc) {  // 接受异常和源位置
    throw_exception(e);  // 调用上面定义的抛出异常函数
  }

} // namespace boost

#endif // BOOST_NO_EXCEPTIONS

// =============================================================================
// -- Workaround for Boost.Asio bundled with rpclib ----------------------------
// =============================================================================

#ifdef ASIO_NO_EXCEPTIONS

#include <exception>
#include <system_error>
#include <typeinfo>

namespace clmdep_asio {
namespace detail {

  template <typename Exception>
  void throw_exception(const Exception& e) {
    carla::throw_exception(e);
  }

  template void throw_exception<std::bad_cast>(const std::bad_cast &);
  template void throw_exception<std::exception>(const std::exception &);
  template void throw_exception<std::system_error>(const std::system_error &);

} // namespace detail
} // namespace clmdep_asio

#endif // ASIO_NO_EXCEPTIONS
