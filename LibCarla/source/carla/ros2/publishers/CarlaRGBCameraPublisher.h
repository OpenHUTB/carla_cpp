// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

//确保该头文件只被包含一次
#pragma once
//指定使用的C++ ABI版本，可能与编译器兼容性相关
#define _GLIBCXX_USE_CXX11_ABI 0

//引入智能指针支持
#include <memory>
//引入向量容器支持
#include <vector>

//引入CarlaPublisher的定义
#include "CarlaPublisher.h"

namespace carla {
namespace ros2 {

  //RGBCameraPublisher的私有实现结构
  struct CarlaRGBCameraPublisherImpl;
  //CameraInfoPublisher的私有实现结构
  struct CarlaCameraInfoPublisherImpl;

  //CarlaRGBCameraPublisher类，负责发布RGB相机数据
  class CarlaRGBCameraPublisher : public CarlaPublisher {
    public:
      //构造函数，接受ROS主题名称和可选父级名称，默认为空字符串
      CarlaRGBCameraPublisher(const char* ros_name = "", const char* parent = "");
      //析构函数
      ~CarlaRGBCameraPublisher();
      //拷贝构造函数
      CarlaRGBCameraPublisher(const CarlaRGBCameraPublisher&);
      //拷贝赋值运算符
      CarlaRGBCameraPublisher& operator=(const CarlaRGBCameraPublisher&);
      //移动构造函数
      CarlaRGBCameraPublisher(CarlaRGBCameraPublisher&&);
      //移动赋值运算符
      CarlaRGBCameraPublisher& operator=(CarlaRGBCameraPublisher&&);

      //初始化RGB相机发布器
      bool Init();
      //初始化相机信息数据，包括偏移、分辨率和视场等参数
      void InitInfoData(uint32_t x_offset, uint32_t y_offset, uint32_t height, uint32_t width, float fov, bool do_rectify);
      //发布相机数据
      bool Publish();

      //检查相机发布器是否已初始化
      bool HasBeenInitialized() const;
      //设置图像数据，包含时间戳、高度、宽度和图像数据指针
      void SetImageData(int32_t seconds, uint32_t nanoseconds, uint32_t height, uint32_t width, const uint8_t* data);
      //设置相机信息数据，包含时间戳
      void SetCameraInfoData(int32_t seconds, uint32_t nanoseconds);
      //返回相机类型
      const char* type() const override { return "rgb camera"; }

    private:
      //私有方法：初始化图像数据
      bool InitImage();
      //私有方法：初始化相机信息数据
      bool InitInfo();
      //私有方法：发布图像数据
      bool PublishImage();
      //私有方法：发布相机信息数据
      bool PublishInfo();

      //另一个重载版本的设置图像数据，使用右值引用来移动数据
      void SetImageData(int32_t seconds, uint32_t nanoseconds, uint32_t height, uint32_t width, std::vector<uint8_t>&& data);
      //设置相机信息中的感兴趣区域
      void SetInfoRegionOfInterest(uint32_t x_offset, uint32_t y_offset, uint32_t height, uint32_t width, bool do_rectify);

    private:
      //使用智能指针管理实现细节，避免内存泄漏
      //RGBCamera发布器的实现细节
      std::shared_ptr<CarlaRGBCameraPublisherImpl> _impl;
      //CameraInfo发布器的实现细节
      std::shared_ptr<CarlaCameraInfoPublisherImpl> _impl_info;
  };
}
}
