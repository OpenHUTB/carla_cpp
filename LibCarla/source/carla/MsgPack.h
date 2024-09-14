// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once  // ȷ����ͷ�ļ���������һ��

#include "carla/Buffer.h"  // ���� Buffer ��Ķ���

#include <rpc/msgpack.hpp>  // ���� MessagePack ��ͷ�ļ�

namespace carla {  // carla �����ռ�

  class MsgPack {  // MsgPack �ඨ��
  public:

    // ģ�巽�������л����� obj Ϊ Buffer ����
    template <typename T>
    static Buffer Pack(const T &obj) {
      namespace mp = ::clmdep_msgpack;  // Ϊ MessagePack �����ռ������

      mp::sbuffer sbuf;  // ����һ�� MessagePack �Ļ���������
      mp::pack(sbuf, obj);  // ʹ�� MessagePack ���л����� obj �� sbuf ��

      // �� sbuf �е����ݺʹ�С��װ�� Buffer �����в�����
      return Buffer(reinterpret_cast<const unsigned char *>(sbuf.data()), sbuf.size());
    }

    // ģ�巽������ Buffer �������л�Ϊ���� T
    template <typename T>
    static T UnPack(const Buffer &buffer) {
      namespace mp = ::clmdep_msgpack;  // Ϊ MessagePack �����ռ������

      // ʹ�� MessagePack �� Buffer �е����ݷ����л�Ϊ���� T
      return mp::unpack(reinterpret_cast<const char *>(buffer.data()), buffer.size()).template as<T>();
    }

    // ģ�巽������ԭʼ���ݣ�unsigned char *���ʹ�С�����л�Ϊ���� T
    template <typename T>
    static T UnPack(const unsigned char *data, size_t size) {
      namespace mp = ::clmdep_msgpack;  // Ϊ MessagePack �����ռ������

      // ʹ�� MessagePack ��ԭʼ���ݷ����л�Ϊ���� T
      return mp::unpack(reinterpret_cast<const char *>(data), size).template as<T>();
    }
  };

} // namespace carla

