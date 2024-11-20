// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
// ����һ��Ԥ����ָ�����ȷ��ͷ�ļ�ֻ������һ�Σ������ظ����������
#define _GLIBCXX_USE_CXX11_ABI 0

#include <memory>
#include <vector>
// ����C++ ��׼���е�����ָ�루memory���Ͷ�̬���飨vector�����ͷ�ļ�����������ڴ����Ͷ�̬��С���ݵĴ洢����
#include "CarlaPublisher.h"
// �����Զ����CarlaPublisher.hͷ�ļ����Ʋ����ж������뷢������صĻ����������Ҫ�����͡������ȹ���ǰ��̳л�ʹ��
namespace carla {
namespace ros2 {

  struct CarlaOpticalFlowCameraPublisherImpl;
  struct CarlaCameraInfoPublisherImpl;
  // ǰ�����������ṹ�����ͣ�����ֻ�����������ǵĴ��ڣ����嶨������������ط���ͨ����.cpp�ļ��У������������Խ��ѭ�����õȱ������⣬
  // ���������п�����ʹ�����ǵ�ָ�����͵ȣ�����������֪�����ǵ��ڲ��ṹ
  class CarlaOpticalFlowCameraPublisher : public CarlaPublisher {
    public:
      CarlaOpticalFlowCameraPublisher(const char* ros_name = "", const char* parent = "");
      // ���캯�������ڴ���CarlaOpticalFlowCameraPublisher��Ķ��󣬿��Դ���ROS���ƺ͸��ڵ�������Ϊ����
      // ���������������ʹ��Ĭ�ϵĿ��ַ�����Ϊ����ֵ
      ~CarlaOpticalFlowCameraPublisher();
      // ���������������ڶ�������ʱ���б�Ҫ����Դ�������������ͷŶ�̬������ڴ��

      CarlaOpticalFlowCameraPublisher(const CarlaOpticalFlowCameraPublisher&);
      // �������캯��������ʹ����һ��ͬ���Ͷ�������ʼ����ǰ���󣬽��������ǳ������ز���
      CarlaOpticalFlowCameraPublisher& operator=(const CarlaOpticalFlowCameraPublisher&);
      // ������ֵ��������أ����ڽ�һ��ͬ���Ͷ����ֵ������ǰ����ͬ���漰�����ǳ�������߼�
      CarlaOpticalFlowCameraPublisher(CarlaOpticalFlowCameraPublisher&&);
      // �ƶ����캯����ʵ�ֽ���Դ��һ���������ٵĶ����ƶ������´����Ķ��󣬱��ⲻ��Ҫ�Ŀ�����������������ֵ���ó���
      CarlaOpticalFlowCameraPublisher& operator=(CarlaOpticalFlowCameraPublisher&&);
      // �ƶ���ֵ��������أ�ִ�������ƶ����캯������Դ�ƶ����������ڶ���֮�丳ֵʱ�Ż�����
      bool Init();
      // ��ʼ�����������ڶԷ�������ص�һЩ��Դ��״̬�Ƚ��г�ʼ������������һ������ֵ��ʾ��ʼ���Ƿ�ɹ�
      void InitInfoData(uint32_t x_offset, uint32_t y_offset, uint32_t height, uint32_t width, float fov, bool do_rectify);
      // ���ڳ�ʼ�������Ϣ��ص����ݣ��������ͼ����ˮƽ�ʹ�ֱ�����ƫ�������߶ȡ���ȡ��ӳ����Լ��Ƿ���н����Ȳ���
      bool Publish();
      // ִ�з�����������������ݷ�����ȥ,���ط����Ƿ�ɹ��Ĳ���ֵ

      bool HasBeenInitialized() const;
      // ���ڲ�ѯ�����Ƿ��Ѿ���ɳ�ʼ��������һ������ֵ

      void SetImageData(int32_t seconds, uint32_t nanoseconds, size_t height, size_t width, const float* data);
      // ����ͼ�����ݣ�����ʱ�����������벿�֣���ͼ��ĸ߶ȡ�����Լ�ָ��ͼ�����ݵ�ָ��
      void SetCameraInfoData(int32_t seconds, uint32_t nanoseconds);
      // ���������Ϣ���ݵ�ʱ���������������벿�ֵ�ʱ��ֵ
      const char* type() const override { return "optical flow camera"; }
      // ��д���ࣨCarlaPublisher���е��麯��type�����ر�ʾ��ǰ���������͵��ַ��������ﷵ��"optical flow camera"�����ڱ�ʶ����һ���������������

    private:
      bool InitImage();
      // ˽�к��������ڳ�ʼ��ͼ����ص���Դ��״̬�����س�ʼ���Ƿ�ɹ��Ĳ���ֵ
      bool InitInfo();
      // ˽�к��������ڳ�ʼ�������Ϣ��ص���Դ��״̬�����س�ʼ���Ƿ�ɹ��Ĳ���ֵ
      bool PublishImage();
      // ˽�к���������ִ��ͼ�����ݵķ������������ط����Ƿ�ɹ��Ĳ���ֵ
      bool PublishInfo();
      // ˽�к���������ִ�������Ϣ���ݵķ������������ط����Ƿ�ɹ��Ĳ���ֵ
      void SetInfoRegionOfInterest( uint32_t x_offset, uint32_t y_offset, uint32_t height, uint32_t width, bool do_rectify);
      // ˽�к������������������Ϣ�еĸ���Ȥ���򣬴���ˮƽ�ʹ�ֱ�����ƫ�������߶ȡ�����Լ��Ƿ���н����Ȳ���
      void SetData(int32_t seconds, uint32_t nanoseconds, size_t height, size_t width, std::vector<uint8_t>&& data);
      // ˽�к�����������������
      // ����ʹ����ֵ���ý��ն�̬���飬������Ϊ�˸�Ч������ʱ���ݣ����ⲻ��Ҫ�Ŀ���
    private:
      std::shared_ptr<CarlaOpticalFlowCameraPublisherImpl> _impl;
      // ����һ������ָ�룬ָ��CarlaOpticalFlowCameraPublisherImpl�ṹ�����ͣ����ڹ�������Ͷ������������
      std::shared_ptr<CarlaCameraInfoPublisherImpl> _impl_info;
      // ���Ƶأ�����һ������ָ�룬ָ��CarlaCameraInfoPublisherImpl�ṹ�����ͣ����ڹ��������Ϣ������ص�ʵ�ֶ������������
  };
  };
}
}
