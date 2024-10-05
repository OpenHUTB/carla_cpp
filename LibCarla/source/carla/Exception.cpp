// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/Exception.h"  //包含carla命名空间中的Exception类定义
#include <boost/assert/source_location.hpp>//包含Boost库中的source_lacation类定义

// =============================================================================
// -- Define boost::throw_exception --------------------------------------------
// =============================================================================

#ifdef BOOST_NO_EXCEPTIONS   //如果Boost库禁用异常

namespace boost {

  void throw_exception(const std::exception &e) { //定义throw_exception函数
    carla::throw_exception(e);//将异常转发给carla命名空间的throw_exception函数
  }

  void throw_exception(  //重载throw_exception函数，接受source_location参数
      const std::exception &e,
      boost::source_location const & loc) {
    throw_exception(e);
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
