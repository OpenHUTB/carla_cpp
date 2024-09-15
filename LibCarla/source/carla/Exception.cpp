// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/Exception.h"  // ���� CARLA �쳣�����ͷ�ļ�
#include <boost/assert/source_location.hpp>  // ���� Boost ���е�Դλ�ö���

// =============================================================================
// -- Define boost::throw_exception --------------------------------------------
// =============================================================================

#ifdef BOOST_NO_EXCEPTIONS  // �����������֧���쳣

namespace boost {

  // ���� boost::throw_exception�����쳣���׳�ʱ���� carla::throw_exception
  void throw_exception(const std::exception &e) {
    carla::throw_exception(e);  // ���� CARLA ���쳣������
  }

  // �������ذ汾������ Boost ��Դλ����Ϊ���������������⴦��
  void throw_exception(
      const std::exception &e,
      boost::source_location const & loc) {
    throw_exception(e);  // ֱ�ӵ���֮ǰ����ĺ���
  }

} // namespace boost

#endif // BOOST_NO_EXCEPTIONS

// =============================================================================
// -- Workaround for Boost.Asio bundled with rpclib ----------------------------
// =============================================================================

#ifdef ASIO_NO_EXCEPTIONS  // ��� Boost.Asio ��֧���쳣

#include <exception>  // ������׼�쳣������ص�ͷ�ļ�
#include <system_error>  // ����ϵͳ��������ص�ͷ�ļ�
#include <typeinfo>  // ����������Ϣ��ص�ͷ�ļ�

namespace clmdep_asio {  // �����ռ� clmdep_asio����ʾ�� Boost.Asio ���ݵĲ���
namespace detail {  // �����ռ� detail����ʾ�ڲ�ʵ��ϸ��

  // ģ�庯�����壬��������쳣����
  template <typename Exception>
  void throw_exception(const Exception& e) {
    carla::throw_exception(e);  // ���� CARLA ���쳣������
  }

  // ��ȷʵ����ģ�庯����ָ��������쳣����
  template void throw_exception<std::bad_cast>(const std::bad_cast &);  // ���� std::bad_cast �쳣
  template void throw_exception<std::exception>(const std::exception &);  // ���� std::exception �쳣
  template void throw_exception<std::system_error>(const std::system_error &);  // ���� std::system_error �쳣

} // namespace detail
} // namespace clmdep_asio

#endif // ASIO_NO_EXCEPTIONS

