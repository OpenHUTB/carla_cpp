// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
#define _GLIBCXX_USE_CXX11_ABI 0// �������ʹ�þɵ� C++ ABI��ͨ��������ĳЩ��ļ����ԡ�  

#include <memory>// �����ڴ������ص�ͷ�ļ�����Ҫ��������ָ�롣  

#include <vector>

#include "CarlaPublisher.h"

namespace carla {
namespace ros2 {// ���������ռ� carla::ros2����֯��ص���͹��ܡ�  

  struct CarlaTransformPublisherImpl;// ǰ������ CarlaTransformPublisherImpl �ṹ�壬����ʵ�ֽ������� Pimpl ģʽ�С�

  class CarlaTransformPublisher : public CarlaPublisher {  // ���� CarlaTransformPublisher �࣬�̳��� CarlaPublisher �ࡣ
    public:
      CarlaTransformPublisher(const char* ros_name = "", const char* parent = "");// ���캯�������� ROS ���ƺ͸������ƣ�Ĭ��ֵΪ���ַ�����
      ~CarlaTransformPublisher();  // ��������������������Դ��
      CarlaTransformPublisher(const CarlaTransformPublisher&);// �������캯����������θ��Ƹ����ʵ���� 
      CarlaTransformPublisher& operator=(const CarlaTransformPublisher&);  // ������ֵ�������������θ�ֵһ��ʵ������һ��ʵ����
      CarlaTransformPublisher(CarlaTransformPublisher&&);// �ƶ����캯������������ƶ�һ��ʵ����
      CarlaTransformPublisher& operator=(CarlaTransformPublisher&&);// �ƶ���ֵ�������������ν�һ��ʵ������Դת�Ƶ���һ��ʵ����  

      bool Init();// ��ʼ�����������ز���ֵ��ʾ��ʼ���Ƿ�ɹ���  
      bool Publish();// �������������ز���ֵ��ʾ�����Ƿ�ɹ���  
      void SetData(int32_t seconds, uint32_t nanoseconds, const float* translation, const float* rotation);// �������ݺ���������ʱ�䣨������룩�Լ�ƽ�ƺ���ת�ĸ������顣  
      const char* type() const override { return "transform"; }// ��д����� type() �����������ַ��� "transform"����ʾ�÷����ߵ����͡�  


    private:
      std::shared_ptr<CarlaTransformPublisherImpl> _impl; // ʹ������ָ����� CarlaTransformPublisherImpl ��ʵ��������ʵ��ϸ�ڡ�  
  };
  };
}
}
