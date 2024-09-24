// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once // ֻ����һ�ε�Ԥ����ָ��

namespace carla { // ����carla�����ռ�
namespace client { // ����client�������ռ�

  enum class GarbageCollectionPolicy { // �����������ղ��Ե�ö����
    Disabled, // ������������
    Enabled,  // ������������
    Inherit   // �̳�������������
  };

} // namespace client
} // namespace carla
