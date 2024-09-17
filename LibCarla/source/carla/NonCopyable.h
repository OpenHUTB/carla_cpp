// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

<<<<<<< HEAD
#pragma once // ȷ����ͷ�ļ�ֻ�ᱻ����һ��

namespace carla { // ���������ռ� carla���Ա���������ͻ

  /// ��������ڽ�ֹ�������ƶ����캯������ֵ����
  class NonCopyable {
  public:

    NonCopyable() = default; // Ĭ�Ϲ��캯��

    NonCopyable(const NonCopyable &) = delete; // ���ÿ������캯��
    NonCopyable &operator=(const NonCopyable &) = delete; // ���ÿ�����ֵ������

    NonCopyable(NonCopyable &&) = delete; // �����ƶ����캯��
    NonCopyable &operator=(NonCopyable &&) = delete; // �����ƶ���ֵ������
  };

  /// ��������ڽ�ֹ�������캯���͸�ֵ�������������ƶ����캯���͸�ֵ����
  class MovableNonCopyable {
  public:

    MovableNonCopyable() = default; // Ĭ�Ϲ��캯��

    MovableNonCopyable(const MovableNonCopyable &) = delete; // ���ÿ������캯��
    MovableNonCopyable &operator=(const MovableNonCopyable &) = delete; // ���ÿ�����ֵ������

    MovableNonCopyable(MovableNonCopyable &&) = default; // ����Ĭ�ϵ��ƶ����캯��
    MovableNonCopyable &operator=(MovableNonCopyable &&) = default; // ����Ĭ�ϵ��ƶ���ֵ������
  };

} // ���������ռ� carla
=======
#pragma once // 确保该头文件只会被编译一次

namespace carla { // 定义命名空间 carla，以避免命名冲突

  /// 这个类用于禁止拷贝和移动构造函数及赋值操作
  class NonCopyable {
  public:

    NonCopyable() = default; // 默认构造函数

    NonCopyable(const NonCopyable &) = delete; // 禁用拷贝构造函数
    NonCopyable &operator=(const NonCopyable &) = delete; // 禁用拷贝赋值操作符

    NonCopyable(NonCopyable &&) = delete; // 禁用移动构造函数
    NonCopyable &operator=(NonCopyable &&) = delete; // 禁用移动赋值操作符
  };

  /// 这个类用于禁止拷贝构造函数和赋值操作，但允许移动构造函数和赋值操作
  class MovableNonCopyable {
  public:

    MovableNonCopyable() = default; // 默认构造函数

    MovableNonCopyable(const MovableNonCopyable &) = delete; // 禁用拷贝构造函数
    MovableNonCopyable &operator=(const MovableNonCopyable &) = delete; // 禁用拷贝赋值操作符

    MovableNonCopyable(MovableNonCopyable &&) = default; // 允许默认的移动构造函数
    MovableNonCopyable &operator=(MovableNonCopyable &&) = default; // 允许默认的移动赋值操作符
  };

} // 结束命名空间 carla
>>>>>>> 4abc25176ac5d5930e03168b4e3d67d59cc132e7
