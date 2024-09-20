// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#ifdef LIBCARLA_NO_EXCEPTIONS

namespace std {

  class exception; // 声明 std 命名空间中的 exception 类

} // namespace std

namespace carla {

       /// �û��Զ���ĺ����������� Boost �� throw_exception��
      ///
      /// @important Boost �쳣Ҳ��·�ɵ��˺�����
      ///
      /// ��ʹ�� LIBCARLA_NO_EXCEPTIONS ����ʱ���˺����� LibCarla ��δ���壬
      /// ʹ�� LibCarla ��ģ����Ҫ�ṩ���ʵĶ��塣���� throw_exception �Ĵ���
      /// ���Լ���˺������᷵�أ���ˣ�����û������ throw_exception ���أ�
      /// ��Ϊ��δ����ġ�
  [[ noreturn ]] void throw_exception(const std::exception &e);

} // namespace carla

#else

namespace carla {

  template <typename T>
  [[ noreturn ]] void throw_exception(const T &e) { // 模板函数声明，接受任何类型的异常
    throw e;
  }

} // namespace carla

#endif // LIBCARLA_NO_EXCEPTIONS
