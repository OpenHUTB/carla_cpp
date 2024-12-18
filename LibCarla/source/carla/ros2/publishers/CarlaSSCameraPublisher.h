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

    // ǰ�������ṹ�� CarlaSSCameraPublisherImpl �� CarlaCameraInfoPublisherImpl
  struct CarlaSSCameraPublisherImpl;
  struct CarlaCameraInfoPublisherImpl;

  // ���� CarlaSSCameraPublisher �࣬�̳��� CarlaPublisher
  class CarlaSSCameraPublisher : public CarlaPublisher {
    public:
        // ���캯�������� ROS ���ƺ͸���������Ϊ������Ĭ��Ϊ���ַ���
      CarlaSSCameraPublisher(const char* ros_name = "", const char* parent = "");
      // ��������
      ~CarlaSSCameraPublisher();
      // �������캯��
      CarlaSSCameraPublisher(const CarlaSSCameraPublisher&);
      // ������ֵ�����
      CarlaSSCameraPublisher& operator=(const CarlaSSCameraPublisher&);
      // �ƶ����캯��
      CarlaSSCameraPublisher(CarlaSSCameraPublisher&&);
      // �ƶ���ֵ�����
      CarlaSSCameraPublisher& operator=(CarlaSSCameraPublisher&&);

      // ��ʼ�����������ز���ֵָʾ�Ƿ�ɹ�
      bool Init();
      // ��ʼ�������Ϣ���ݣ�����ƫ������ͼ��ߴ���ӳ��ǵȲ���
      void InitInfoData(uint32_t x_offset, uint32_t y_offset, uint32_t height, uint32_t width, float fov, bool do_rectify);
      // �������ݵĺ��������ز���ֵָʾ�Ƿ�ɹ�
      bool Publish();

      // �������Ƿ��ѳ�ʼ��
      bool HasBeenInitialized() const;
      // ����ͼ�����ݵĺ���������ʱ�����ͼ������
      void SetImageData(int32_t seconds, uint32_t nanoseconds, size_t height, size_t width, const uint8_t* data);
      // ���������Ϣ���ݵĺ���������ʱ���
      void SetCameraInfoData(int32_t seconds, uint32_t nanoseconds);
      // ���ǻ���� type() ���������ش���������
      const char* type() const override { return "semantic segmentation"; }

    private:
        // ��ʼ��ͼ�����ݵ�˽�к��������ز���ֵָʾ�Ƿ�ɹ�
      bool InitImage();
      // ��ʼ����Ϣ��˽�к��������ز���ֵָʾ�Ƿ�ɹ�
      bool InitInfo();
      // ����ͼ���˽�к��������ز���ֵָʾ�Ƿ�ɹ�
      bool PublishImage();
      // ������Ϣ��˽�к��������ز���ֵָʾ�Ƿ�ɹ�
      bool PublishInfo();

      // ���ø���Ȥ������Ϣ������ƫ�������ߴ���Ƿ����У��
      void SetInfoRegionOfInterest( uint32_t x_offset, uint32_t y_offset, uint32_t height, uint32_t width, bool do_rectify);
      // �ڲ��������ú�����ʹ����ֵ���÷�ʽ��������
      void SetData(int32_t seconds, uint32_t nanoseconds, size_t height, size_t width, std::vector<uint8_t>&& data);

    private:
        // ʹ������ָ����� CarlaSSCameraPublisherImpl �� CarlaCameraInfoPublisherImpl ��ʵ��
      std::shared_ptr<CarlaSSCameraPublisherImpl> _impl;
      std::shared_ptr<CarlaCameraInfoPublisherImpl> _impl_info;
  };
}// namespace ros2
}// namespace carla
