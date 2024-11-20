// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once// ȷ����ͷ�ļ�ֻ������һ��
#define _GLIBCXX_USE_CXX11_ABI 0// ָ��ʹ�õ�C++ ABI�汾

#include <memory>// ��������ָ��֧��
#include <vector>// ������������֧��

#include "CarlaPublisher.h"// ����CarlaPublisher�Ķ���

namespace carla {
namespace ros2 {

  struct CarlaRadarPublisherImpl;// ǰ������������ʵ��ϸ��

  class CarlaRadarPublisher : public CarlaPublisher {// CarlaRadarPublisher�࣬�����״����ݵķ���
    public:
      CarlaRadarPublisher(const char* ros_name = "", const char* parent = "");// ���캯��������ROS�������ƺ͸������ƣ�Ĭ��Ϊ���ַ���
      ~CarlaRadarPublisher();// ��������
      CarlaRadarPublisher(const CarlaRadarPublisher&);// �������캯��
      CarlaRadarPublisher& operator=(const CarlaRadarPublisher&);// ������ֵ�����
      CarlaRadarPublisher(CarlaRadarPublisher&&);// �ƶ����캯��
      CarlaRadarPublisher& operator=(CarlaRadarPublisher&&);// �ƶ���ֵ�����

      bool Init();// ��ʼ���״﷢����
      bool Publish();// �����״�����
      void SetData(int32_t seconds, uint32_t nanoseconds, size_t height, size_t width, size_t elements, const uint8_t* data);// �����״����ݣ�����ʱ������߶ȡ���ȡ�Ԫ������������ָ��
      const char* type() const override { return "radar"; }// �����״�����

    private:
      void SetData(int32_t seconds, uint32_t nanoseconds, size_t height, size_t width, size_t elements, std::vector<uint8_t>&& data);// ��һ�����ذ汾���������ݣ�ʹ����ֵ�������ƶ�����

    private:
      std::shared_ptr<CarlaRadarPublisherImpl> _impl;// ʹ������ָ�����ʵ��ϸ�ڣ������ڴ�й©
  };
}
}
