// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

// ָʾͷ�ļ�ֻ������һ��
#pragma once
// ���� GLIBCXX ��ʹ�� C++11 ABI �ĺ�
#define _GLIBCXX_USE_CXX11_ABI 0

// �����ڴ�����������
#include <memory>
#include <vector>

// ���� CarlaPublisher ��Ķ���
#include "CarlaPublisher.h"

namespace carla {
namespace ros2 {

    // ǰ�������ṹ�� CarlaLidarPublisherImpl
  struct CarlaLidarPublisherImpl;

  // ���� CarlaLidarPublisher �࣬�̳��� CarlaPublisher 
  class CarlaLidarPublisher : public CarlaPublisher {
    public:
        // ���캯�������� ROS ���ƺ͸���������Ϊ������Ĭ��Ϊ���ַ���
      CarlaLidarPublisher(const char* ros_name = "", const char* parent = "");
      // ��������
      ~CarlaLidarPublisher();
      // �������캯��
      CarlaLidarPublisher(const CarlaLidarPublisher&);
      // ������ֵ�����
      CarlaLidarPublisher& operator=(const CarlaLidarPublisher&);
      // �ƶ����캯��
      CarlaLidarPublisher(CarlaLidarPublisher&&);
      // �ƶ���ֵ�����
      CarlaLidarPublisher& operator=(CarlaLidarPublisher&&);

      // ��ʼ�����������ز���ֵָʾ�Ƿ�ɹ�
      bool Init();
      // �������ݵĺ��������ز���ֵָʾ�Ƿ�ɹ�
      bool Publish();
      // ���ü����״����ݵĺ���������ʱ������߶ȡ���Ⱥ�����ָ��
      void SetData(int32_t seconds, uint32_t nanoseconds, size_t height, size_t width, float* data);
      // ���ǻ���� type() ���������ش���������
      const char* type() const override { return "lidar"; }

    private:
        // ˽���������ݵĺ���������ʱ������߶ȡ���Ⱥ��� rvalue ���÷�ʽ���ݵ���������
      void SetData(int32_t seconds, uint32_t nanoseconds, size_t height, size_t width, std::vector<uint8_t>&& data);

    private:
        // ʹ������ָ����� CarlaLidarPublisherImpl ��ʵ��
      std::shared_ptr<CarlaLidarPublisherImpl> _impl;
  };
}// namespace ros2
}// namespace carla
