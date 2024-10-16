// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
#define _GLIBCXX_USE_CXX11_ABI 0 //设置 C++11 ABI 的使用方式。

#include <memory>
#include <vector>

#include "CarlaPublisher.h"

namespace carla {
namespace ros2 { //定义了代码所属的命名空间

  struct CarlaISCameraPublisherImpl;
  struct CarlaCameraInfoPublisherImpl; //是内部实现的结构体声明，具体实现可能在对应的源文件中。

  class CarlaISCameraPublisher : public CarlaPublisher {
    public:
      CarlaISCameraPublisher(const char* ros_name = "", const char* parent = ""); //带默认参数的构造函数，用于创建CarlaISCameraPublisher对象，可以指定 ROS 名称和父节点名称。
      ~CarlaISCameraPublisher(); //析构函数，释放资源
      CarlaISCameraPublisher(const CarlaISCameraPublisher&);
      CarlaISCameraPublisher& operator=(const CarlaISCameraPublisher&);
      CarlaISCameraPublisher(CarlaISCameraPublisher&&);
      CarlaISCameraPublisher& operator=(CarlaISCameraPublisher&&);

      bool Init(); //初始化发布者
      void InitInfoData(uint32_t x_offset, uint32_t y_offset, uint32_t height, uint32_t width, float fov, bool do_rectify); //初始化相机信息数据。
      bool Publish(); //发布相机数据和相机信息数据
      bool HasBeenInitialized() const; //检查发布者是否已被初始化。
      void SetImageData(int32_t seconds, uint32_t nanoseconds, size_t height, size_t width, const uint8_t* data); //设置图像数据。

      void SetCameraInfoData(int32_t seconds, uint32_t nanoseconds); //设置相机信息数据。
      const char* type() const override { return "instance segmentation"; } //返回发布的数据类型为 “instance segmentation”。

    private:
      bool InitImage(); //用于初始化图像
      bool InitInfo(); //用相机信息的发布。
      bool PublishImage();用于发布图像信息。
      bool PublishInfo();用于发布相机信息。

      void SetInfoRegionOfInterest( uint32_t x_offset, uint32_t y_offset, uint32_t height, uint32_t width, bool do_rectify); //设置相机信息的感兴趣区域。
      void SetData(int32_t seconds, uint32_t nanoseconds, size_t height, size_t width, std::vector<uint8_t>&& data); //设置数据，包括时间戳、高度、宽度和图像数据

    private:
      std::shared_ptr<CarlaISCameraPublisherImpl> _impl; //指向内部实现的智能指针，用于管理实例分割相机数据的发布。
      std::shared_ptr<CarlaCameraInfoPublisherImpl> _impl_info; //指向内部实现的智能指针，用于管理相机信息的发布。
  };
}
}
