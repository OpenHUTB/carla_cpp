// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
#define _GLIBCXX_USE_CXX11_ABI 0

#include <memory>
#include <vector>//���������ͷ�ļ� 

#include "CarlaPublisher.h"//����CarlaPublisher.h������� 

namespace carla {
namespace ros2 {
//���������ռ�carla::ros2,������֯���� 
  struct CarlaTransformPublisherImpl;//����һ��˽�еĽṹ��CarlaTransformPublisherImpl 
//����CarlaTransformPublisher�࣬�̳���CarlaPublisher�� 
  class CarlaTransformPublisher : public CarlaPublisher {
    public:
      CarlaTransformPublisher(const char* ros_name = "", const char* parent = "");//���캯��������ROS�ڵ����ƺ͸��ڵ�������Ϊ���� 
      ~CarlaTransformPublisher();//��������������������Դ 
      CarlaTransformPublisher(const CarlaTransformPublisher&);//���쿽������ 
      CarlaTransformPublisher& operator=(const CarlaTransformPublisher&);//�������㸳ֵ����� 
      CarlaTransformPublisher(CarlaTransformPublisher&&);//�ƶ����캯�� 
      CarlaTransformPublisher& operator=(CarlaTransformPublisher&&);//�ƶ���ֵ����� 

      bool Init();//��ʼ���������������÷����� 
      bool Publish();//�������������ڷ����任��Ϣ 
      void SetData(int32_t seconds, uint32_t nanoseconds, const float* translation, const float* rotation);//��������ʱ�����������룩��ƽ����������ת���� 
      const char* type() const override { return "transform"; }//�����������ƣ���д���෽�� 

    private://˽�г�Ա������ָ��CarlaTransformPublisherImpl������ָ�� 
      std::shared_ptr<CarlaTransformPublisherImpl> _impl;//����ʵ������ڲ����� 
  };
}
}
