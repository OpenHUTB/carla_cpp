// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
#define _GLIBCXX_USE_CXX11_ABI 0

#include <memory>
#include <vector>

#include "CarlaPublisher.h"

namespace carla {
namespace ros2 {

  // 前向声明实现细节结构体
  struct CarlaOpticalFlowCameraPublisherImpl;
  struct CarlaCameraInfoPublisherImpl;

  // CarlaOpticalFlowCameraPublisher 类负责发布光流摄像头数据
  class CarlaOpticalFlowCameraPublisher : public CarlaPublisher {
    public:
      // 构造函数，接受 ROS 节点名和父级信息
      CarlaOpticalFlowCameraPublisher(const char* ros_name = "", const char* parent = "");
      // 析构函数
      ~CarlaOpticalFlowCameraPublisher();
      // 拷贝构造函数
      CarlaOpticalFlowCameraPublisher(const CarlaOpticalFlowCameraPublisher&);
      // 拷贝赋值运算符
      CarlaOpticalFlowCameraPublisher& operator=(const CarlaOpticalFlowCameraPublisher&);
      // 移动构造函数
      CarlaOpticalFlowCameraPublisher(CarlaOpticalFlowCameraPublisher&&);
      // 移动赋值运算符
      CarlaOpticalFlowCameraPublisher& operator=(CarlaOpticalFlowCameraPublisher&&);

      // 初始化发布器，返回初始化是否成功
      bool Init();
      // 初始化相机信息数据
      void InitInfoData(uint32_t x_offset, uint32_t y_offset, uint32_t height, uint32_t width, float fov, bool do_rectify);
      // 发布数据
      bool Publish();

      // 检查发布器是否已初始化
      bool HasBeenInitialized() const;
      // 设置图像数据
      void SetImageData(int32_t seconds, uint32_t nanoseconds, size_t height, size_t width, const float* data);
      // 设置相机信息数据
      void SetCameraInfoData(int32_t seconds, uint32_t nanoseconds);
      // 返回发布器类型
      const char* type() const override { return "optical flow camera"; }

    private:
      // 初始化图像发布
      bool InitImage();
      // 初始化相机信息发布
      bool InitInfo();
      // 发布图像的方法
      bool PublishImage();
      // 发布信息的方法
      bool PublishInfo();

      // 设置感兴趣区域的信息
      void SetInfoRegionOfInterest( uint32_t x_offset, uint32_t y_offset, uint32_t height, uint32_t width, bool do_rectify);
      // 设置数据
      void SetData(int32_t seconds, uint32_t nanoseconds, size_t height, size_t width, std::vector<uint8_t>&& data);

    private:
      // 实现类的共享指针
      std::shared_ptr<CarlaOpticalFlowCameraPublisherImpl> _impl;
      std::shared_ptr<CarlaCameraInfoPublisherImpl> _impl_info;
  };
}// namespace ros2
}// namespace carla
