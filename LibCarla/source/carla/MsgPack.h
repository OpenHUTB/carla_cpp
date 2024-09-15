// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
#include "carla/Buffer.h" // ���� Buffer ͷ�ļ���Buffer �����ڴ������ݻ�����
#include <rpc/msgpack.hpp> // ���� MessagePack ��ͷ�ļ�

namespace carla {

  class MsgPack {
  public:

    // ģ�巽���������� obj ���л�Ϊ Buffer
    template <typename T>
    static Buffer Pack(const T &obj) {
      namespace mp = ::clmdep_msgpack; // ʹ���Զ��������ռ� clmdep_msgpack
      mp::sbuffer sbuf; // ����һ�� MessagePack �Ļ��������� sbuf
      mp::pack(sbuf, obj); // ʹ�� MessagePack �� pack ���������� obj ���л��� sbuf ��
      // ���� Buffer ���󣬽� sbuf �����ݺʹ�С���ݸ� Buffer
      return Buffer(reinterpret_cast<const unsigned char *>(sbuf.data()), sbuf.size());
    }

    // ģ�巽������ Buffer �����л�Ϊ���� T
    template <typename T>
    static T UnPack(const Buffer &buffer) {
      namespace mp = ::clmdep_msgpack; // ʹ���Զ��������ռ� clmdep_msgpack
      // ʹ�� MessagePack �� unpack ������ Buffer �е����ݷ����л�Ϊ���� T
      // reinterpret_cast ���ڽ� unsigned char* ���͵� data ת��Ϊ char* ����
      return mp::unpack(reinterpret_cast<const char *>(buffer.data()), buffer.size()).template as<T>();
    }

    // ģ�巽������ԭʼ���ݷ����л�Ϊ���� T
    template <typename T>
    static T UnPack(const unsigned char *data, size_t size) {
      namespace mp = ::clmdep_msgpack; // ʹ���Զ��������ռ� clmdep_msgpack
      // ʹ�� MessagePack �� unpack ������ԭʼ���ݷ����л�Ϊ���� T
      // reinterpret_cast ���ڽ� unsigned char* ���͵� data ת��Ϊ char* ����
      return mp::unpack(reinterpret_cast<const char *>(data), size).template as<T>();
    }
  };
