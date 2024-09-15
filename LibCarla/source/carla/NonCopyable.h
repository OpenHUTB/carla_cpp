// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

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
