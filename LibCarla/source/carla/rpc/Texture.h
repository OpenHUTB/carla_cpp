﻿// Copyright (c) 2021 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/MsgPack.h"// ���� MsgPack ��ص�ͷ�ļ��������������л��������л�������Ϣ�����صĹ��ܣ�����ȡ�������ڲ�ʵ�֣�
#include "carla/rpc/FloatColor.h"// ���� FloatColor ��ص�ͷ�ļ����Ʋ� FloatColor �����ڱ�ʾĳ�ָ�����������ɫ�����ݽṹ����
#include "carla/sensor/data/Color.h"// ���� sensor/data/Color ���ͷ�ļ���Ӧ���Ǻʹ����������е���ɫ��ص����ݽṹ����

#include <vector>// �����׼���е��������������ڴ洢��̬��С�����ݼ���

namespace carla {// ���� carla �����ռ䣬����Ӧ�������� carla ��Ŀ��ص����ݣ�����������ռ��¶Ը��๦�ܽ�����֯
namespace rpc {// ��һ������ rpc �������ռ䣬�������ڴ��Զ�̹��̵��ã�RPC����ص��ࡢ������ʵ��

  template<typename T>// ����һ��ģ���� Texture������һ��ͨ�õ������࣬���Ͳ��� T �����Ǿ����ĳ���������ͣ�������ɫ��ص��������ͣ�
    // ͨ��ģ��ķ�ʽ�����Է���ش�����Բ�ͬ�������ݵ����������ߴ��븴����
  class Texture {
  public:
 // Ĭ�Ϲ��캯����ʹ��Ĭ���﷨���������κγ�ʼ����������Ա������ʹ�ø��Ե�Ĭ�ϳ�ʼ��ֵ
    Texture() = default;
// ���캯�������ڴ�������ָ����ȣ�width���͸߶ȣ�height�����������
        // ���� width ��ʾ����Ŀ�ȣ�������Ϊ��λ��ͨ������ͼ������ȶ�ά���ݵĺ���ߴ�������
        // ���� height ��ʾ����ĸ߶ȣ�ͬ��������Ϊ��λ������������ά���ݵ�����ߴ磩
    Texture(uint32_t width, uint32_t height)
      : _width(width), _height(height) {// ���ݴ���Ŀ�Ⱥ͸߶ȣ������洢�������ݵ�������С��ʹ���ܹ����ɶ�Ӧ������Ԫ�أ�_width * _height �� T ����Ԫ�أ�
      _texture_data.resize(_width*_height);
    }
// ��ȡ�����ȵĺ����������������ǰ�Ŀ��ֵ��������Ϊ��λ��
    uint32_t GetWidth() const {
      return _width;
    }
// ��ȡ����߶ȵĺ����������������ǰ�ĸ߶�ֵ��������Ϊ��λ��
    uint32_t GetHeight() const {
      return _height;
    }
// ��������ߴ�ĺ��������ڸı��������Ŀ�Ⱥ͸߶�
        // ���� width ��Ҫ���õ��¿��ֵ��������Ϊ��λ��
        // ���� height ��Ҫ���õ��¸߶�ֵ��������Ϊ��λ��
        // �ڲ�����¿�Ⱥ͸߶ȳ�Ա�������������µĳߴ����µ����������������Ĵ�С
    void SetDimensions(uint32_t width, uint32_t height) {
      _width = width;
      _height = height;
      _texture_data.resize(_width*_height);
    }
// ���ض�����������ָ��λ�ã�����Ϊ (x, y)��Ԫ�ص����ã��������޸ĸ�λ�õ�Ԫ��ֵ
        // ���� x ��ʾ���������еĺ����꣨�����ҵ�����λ�ã���Χ�� 0 ����� - 1��
        // ���� y ��ʾ���������е������꣨���ϵ��µ�����λ�ã���Χ�� 0 ���߶� - 1��
    T& At (uint32_t x, uint32_t y) {
      return _texture_data[y*_width + x];
    }
// ���ض�����������ָ��λ�ã�����Ϊ (x, y)��Ԫ�صĳ����ã����ڻ�ȡ��λ�õ�Ԫ��ֵ���������޸ģ������� const ������ô˺����������
        // ���� x �� y �ĺ���������� At ��������ͬ�����ڶ�λ���������еľ���λ��
    const T& At (uint32_t x, uint32_t y) const {
      return _texture_data[y*_width + x];
    }
 // ��ȡָ���������ݴ洢�����ָ�루const �汾����ͨ�����ָ����Զ�ȡ�������ݣ��������޸�
        // �ⲿ��������������ָ����и�Ч�����ݷ��ʣ����紫�ݸ�������Ҫ��ȡ�������ݵĺ����������
    const T* GetDataPtr() const {
      return _texture_data.data();
    }

  private:
  	
    uint32_t _width = 0; // ����Ŀ�ȣ�������Ϊ��λ����ʼֵΪ 0����ʾĬ��δ��ʼ���Ŀ��״̬
    uint32_t _height = 0;// ����ĸ߶ȣ�������Ϊ��λ����ʼֵΪ 0����ʾĬ��δ��ʼ���ĸ߶�״̬
    std::vector<T> _texture_data;// ���ڴ洢�������ݵ�������������Ԫ������Ϊģ����� T��������С���������Ŀ�Ⱥ͸߶ȶ�̬����

  public: // ʹ�� MsgPack ����ĺ꣬����ָ���ڽ�����Ϣ��������л����ͽ���������л���ʱ���漰�ĳ�Ա��������Щ

    MSGPACK_DEFINE_ARRAY(_width, _height, _texture_data);// ����ָ���˿�ȡ��߶��Լ���������������������Ա����������ز������������ݵĴ���ʹ洢�ȴ���
  };

  using TextureColor = Texture<sensor::data::Color>;// ����һ�����ͱ��� TextureColor�����ǻ��� Texture ģ���࣬��ģ����� T ����ָ��Ϊ sensor::data::Color ���͵���������
  using TextureFloatColor = Texture<FloatColor>; // ����һ�����ͱ��� TextureFloatColor��ͬ������ Texture ģ���࣬��ģ����� T ����ָ��Ϊ FloatColor ���͵���������

}
}
