// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once  // 保证此头文件只被包含一次
#define _GLIBCXX_USE_CXX11_ABI 0  // 定义宏以控制C++11 ABI兼容性

#include <memory>  // 引入智能指针库
#include <vector>  // 引入向量库

#include "CarlaPublisher.h"  // 引入CarlaPublisher类的头文件

namespace carla {  // 定义carla命名空间
namespace ros2 {  // 定义ros2命名空间

  struct CarlaDVSCameraPublisherImpl;  // 前向声明CarlaDVSCameraPublisherImpl结构体
  struct CarlaCameraInfoPublisherImpl;  // 前向声明CarlaCameraInfoPublisherImpl结构体
  struct CarlaPointCloudPublisherImpl;  // 前向声明CarlaPointCloudPublisherImpl结构体

  class CarlaDVSCameraPublisher : public CarlaPublisher {  // 定义CarlaDVSCameraPublisher类，继承自CarlaPublisher
    public:
      CarlaDVSCameraPublisher(const char* ros_name = "", const char* parent = "");  // 构造函数，带ROS名称和父对象名称参数
      ~CarlaDVSCameraPublisher();  // 析构函数
      CarlaDVSCameraPublisher(const CarlaDVSCameraPublisher&);  // 拷贝构造函数
      CarlaDVSCameraPublisher& operator=(const CarlaDVSCameraPublisher&);  // 拷贝赋值运算符
      CarlaDVSCameraPublisher(CarlaDVSCameraPublisher&&);  // 移动构造函数
      CarlaDVSCameraPublisher& operator=(CarlaDVSCameraPublisher&&);  // 移动赋值运算符

      bool Init();  // 初始化函数
      void InitInfoData(uint32_t x_offset, uint32_t y_offset, uint32_t height, uint32_t width, float fov, bool do_rectify);  // 初始化信息数据
      bool Publish();  // 发布函数

      bool HasBeenInitialized() const;  // 检查是否已经初始化
      void SetImageData(int32_t seconds, uint32_t nanoseconds, size_t elements, size_t height, size_t width, const uint8_t* data);  // 设置图像数据
      void SetCameraInfoData(int32_t seconds, uint32_t nanoseconds);  // 设置相机信息数据
      void SetPointCloudData(size_t height, size_t width, size_t elements, const uint8_t* data);  // 设置点云数据
      const char* type() const override { return "dvs camera"; }  // 重写类型函数，返回"dvs camera"

    private:
      bool InitImage();  // 初始化图像函数
      bool InitInfo();  // 初始化信息函数
      bool InitPointCloud();  // 初始化点云函数

      void SetInfoRegionOfInterest(uint32_t x_offset, uint32_t y_offset, uint32_t height, uint32_t width, bool do_rectify);  // 设置感兴趣区域信息
      void SetData(int32_t seconds, uint32_t nanoseconds, size_t height, size_t width, std::vector<uint8_t>&& data);  // 设置数据
      void SetPointCloudData(size_t height, size_t width, std::vector<uint8_t>&& data);  // 设置点云数据
      bool PublishImage();  // 发布图像函数
      bool PublishInfo();  // 发布信息函数
      bool PublishPointCloud();  // 发布点云函数

    private:
      std::shared_ptr<CarlaDVSCameraPublisherImpl> _impl;  // 智能指针，指向CarlaDVSCameraPublisherImpl的实现
      std::shared_ptr<CarlaCameraInfoPublisherImpl> _info;  // 智能指针，指向CarlaCameraInfoPublisherImpl的实现
      std::shared_ptr<CarlaPointCloudPublisherImpl> _point_cloud;  // 智能指针，指向CarlaPointCloudPublisherImpl的实现
  };
}
}
