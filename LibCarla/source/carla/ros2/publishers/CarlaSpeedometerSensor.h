// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

//ָʾͷ�ļ�ֻ������һ��
#pragma once
//���� GLIBCXX ��ʹ�� C++11 ABI �ĺ�
#define _GLIBCXX_USE_CXX11_ABI 0

//�����ڴ�����������
#include <memory>
#include <vector>

//���� CarlaPublisher ��Ķ���
#include "CarlaPublisher.h"

namespace carla {
namespace ros2 {

    //ǰ�������ṹ�� CarlaSpeedometerSensorImpl
  struct CarlaSpeedometerSensorImpl;

  //���� CarlaSpeedometerSensor �࣬�̳��� CarlaPublisher
  class CarlaSpeedometerSensor : public CarlaPublisher {
    public:
        //���캯�������� ROS ���ƺ͸���������Ϊ������Ĭ��Ϊ���ַ���
      CarlaSpeedometerSensor(const char* ros_name = "", const char* parent = "");
      //��������
      ~CarlaSpeedometerSensor();
      //�������캯��
      CarlaSpeedometerSensor(const CarlaSpeedometerSensor&);
      //������ֵ�����
      CarlaSpeedometerSensor& operator=(const CarlaSpeedometerSensor&);
      //�ƶ����캯��
      CarlaSpeedometerSensor(CarlaSpeedometerSensor&&);
      //�ƶ���ֵ�����
      CarlaSpeedometerSensor& operator=(CarlaSpeedometerSensor&&);
      
      //��ʼ�����������ز���ֵָʾ�Ƿ�ɹ�
      bool Init();
      //�������ݵĺ��������ز���ֵָʾ�Ƿ�ɹ�
      bool Publish();
      //�����ٶ����ݵĺ���������һ������ֵ��Ϊ����
      void SetData(float data);
      //���ǻ���� type() ���������ش���������
      const char* type() const override { return "speedometer"; }

    private:
        //ʹ������ָ����� CarlaSpeedometerSensorImpl ��ʵ��
      std::shared_ptr<CarlaSpeedometerSensorImpl> _impl;
  };
}//namespace ros2
}//namespace carla
