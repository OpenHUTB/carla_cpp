// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

//ȷ����ͷ�ļ�ֻ������һ��
#pragma once
//ָ��ʹ�õ�C++ ABI�汾����������������������
#define _GLIBCXX_USE_CXX11_ABI 0

//��������ָ��֧��
#include <memory>
//������������֧��
#include <vector>

//����CarlaPublisher�Ķ���
#include "CarlaPublisher.h"

namespace carla {
namespace ros2 {

  //RGBCameraPublisher��˽��ʵ�ֽṹ
  struct CarlaRGBCameraPublisherImpl;
  //CameraInfoPublisher��˽��ʵ�ֽṹ
  struct CarlaCameraInfoPublisherImpl;

  //CarlaRGBCameraPublisher�࣬���𷢲�RGB�������
  class CarlaRGBCameraPublisher : public CarlaPublisher {
    public:
      //���캯��������ROS�������ƺͿ�ѡ�������ƣ�Ĭ��Ϊ���ַ���
      CarlaRGBCameraPublisher(const char* ros_name = "", const char* parent = "");
      //��������
      ~CarlaRGBCameraPublisher();
      //�������캯��
      CarlaRGBCameraPublisher(const CarlaRGBCameraPublisher&);
      //������ֵ�����
      CarlaRGBCameraPublisher& operator=(const CarlaRGBCameraPublisher&);
      //�ƶ����캯��
      CarlaRGBCameraPublisher(CarlaRGBCameraPublisher&&);
      //�ƶ���ֵ�����
      CarlaRGBCameraPublisher& operator=(CarlaRGBCameraPublisher&&);

      //��ʼ��RGB���������
      bool Init();
      //��ʼ�������Ϣ���ݣ�����ƫ�ơ��ֱ��ʺ��ӳ��Ȳ���
      void InitInfoData(uint32_t x_offset, uint32_t y_offset, uint32_t height, uint32_t width, float fov, bool do_rectify);
      //�����������
      bool Publish();

      //�������������Ƿ��ѳ�ʼ��
      bool HasBeenInitialized() const;
      //����ͼ�����ݣ�����ʱ������߶ȡ���Ⱥ�ͼ������ָ��
      void SetImageData(int32_t seconds, uint32_t nanoseconds, uint32_t height, uint32_t width, const uint8_t* data);
      //���������Ϣ���ݣ�����ʱ���
      void SetCameraInfoData(int32_t seconds, uint32_t nanoseconds);
      //�����������
      const char* type() const override { return "rgb camera"; }

    private:
      //˽�з�������ʼ��ͼ������
      bool InitImage();
      //˽�з�������ʼ�������Ϣ����
      bool InitInfo();
      //˽�з���������ͼ������
      bool PublishImage();
      //˽�з��������������Ϣ����
      bool PublishInfo();

      //��һ�����ذ汾������ͼ�����ݣ�ʹ����ֵ�������ƶ�����
      void SetImageData(int32_t seconds, uint32_t nanoseconds, uint32_t height, uint32_t width, std::vector<uint8_t>&& data);
      //���������Ϣ�еĸ���Ȥ����
      void SetInfoRegionOfInterest(uint32_t x_offset, uint32_t y_offset, uint32_t height, uint32_t width, bool do_rectify);

    private:
      //ʹ������ָ�����ʵ��ϸ�ڣ������ڴ�й©
      //RGBCamera��������ʵ��ϸ��
      std::shared_ptr<CarlaRGBCameraPublisherImpl> _impl;
      //CameraInfo��������ʵ��ϸ��
      std::shared_ptr<CarlaCameraInfoPublisherImpl> _impl_info;
  };
}
}
