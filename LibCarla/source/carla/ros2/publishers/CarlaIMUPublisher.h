// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

// ָʾͷ�ļ�ֻ������һ��  
#pragma once
// ���� GLIBCXX ��ʹ�� C++11 ABI �ĺ�  
#define _GLIBCXX_USE_CXX11_ABI 0

// �����ڴ�����  
#include <memory>

// ���� CarlaPublisher ��Ķ���  
#include "CarlaPublisher.h"

namespace carla {
namespace ros2 {

    // ǰ�������ṹ�� CarlaIMUPublisherImpl  
  struct CarlaIMUPublisherImpl;

  // ���� CarlaIMUPublisher �࣬�̳��� CarlaPublisher  
  class CarlaIMUPublisher : public CarlaPublisher {
    public:
        // ���캯�������� ROS ���ƺ͸���������Ϊ����  
      CarlaIMUPublisher(const char* ros_name = "", const char* parent = "");
      // ��������  
      ~CarlaIMUPublisher();
      // �������캯��
      CarlaIMUPublisher(const CarlaIMUPublisher&);
      // ������ֵ����� 
      CarlaIMUPublisher& operator=(const CarlaIMUPublisher&);
      // �ƶ����캯��
      CarlaIMUPublisher(CarlaIMUPublisher&&);
      // �ƶ���ֵ�����
      CarlaIMUPublisher& operator=(CarlaIMUPublisher&&);

      // ��ʼ�����������ز���ֵָʾ�Ƿ�ɹ�
      bool Init();
      // �������ݵĺ��������ز���ֵָʾ�Ƿ�ɹ�
      bool Publish();
      // ���� IMU ���ݵĺ���������ʱ����ʹ���������
      void SetData(int32_t seconds, uint32_t nanoseconds, float* accelerometer, float* gyroscope, float compass);
      // ���ǻ���� type() ���������ش���������
      const char* type() const override { return "inertial measurement unit"; }

    private:
        // ʹ������ָ����� CarlaIMUPublisherImpl ��ʵ��
      std::shared_ptr<CarlaIMUPublisherImpl> _impl;
  };
}// namespace ros2
}// namespace carla
