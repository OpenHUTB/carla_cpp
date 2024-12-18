// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
#define _GLIBCXX_USE_CXX11_ABI 0

#include <memory>// 引入智能指针相关的头文件 
#include <vector>// 引入向量容器相关的头文件

#include "CarlaPublisher.h"// 引入CarlaPublisher类的头文件

namespace carla {
namespace ros2 {
    /// @brief CarlaDepthCameraPublisher的内部实现结构体（前向声明）  
  /// 这是一个Pimpl（Pointer to IMPLementation）用法，用于隐藏实现细节。
  struct CarlaDepthCameraPublisherImpl;
  /// @brief CarlaCameraInfoPublisher的内部实现结构体（前向声明）  
  /// 这也是一个Pimpl用法，用于隐藏与相机信息相关的实现细节。  
  struct CarlaCameraInfoPublisherImpl;
  /// @class CarlaDepthCameraPublisher  
  /// @brief 用于在ROS 2中发布CARLA深度相机数据的类。     
  /// 这个类继承自CarlaPublisher，专门用于初始化、设置和发布深度相机数据。 
  class CarlaDepthCameraPublisher : public CarlaPublisher {
    public:
        /// @brief 构造函数，用于创建CarlaDepthCameraPublisher对象。  
      /// @param ros_name ROS节点的名称（可选）。  
      /// @param parent 父节点的名称（可选）。  
      CarlaDepthCameraPublisher(const char* ros_name = "", const char* parent = "");
      /// @brief 析构函数，用于销毁CarlaDepthCameraPublisher对象。
      ~CarlaDepthCameraPublisher();
      /// @brief 拷贝构造函数。  
      CarlaDepthCameraPublisher(const CarlaDepthCameraPublisher&);
      /// @brief 拷贝赋值运算符。
      CarlaDepthCameraPublisher& operator=(const CarlaDepthCameraPublisher&);
      /// @brief 移动构造函数。 
      CarlaDepthCameraPublisher(CarlaDepthCameraPublisher&&);
      /// @brief 移动赋值运算符。
      CarlaDepthCameraPublisher& operator=(CarlaDepthCameraPublisher&&);
      /// @brief 初始化函数，用于设置ROS节点和发布者。  
      /// @return 初始化是否成功。  
      bool Init();
      void InitInfoData(uint32_t x_offset, uint32_t y_offset, uint32_t height, uint32_t width, float fov, bool do_rectify);
      bool Publish();

      bool HasBeenInitialized() const;
      void SetImageData(int32_t seconds, uint32_t nanoseconds, size_t height, size_t width, const uint8_t* data);
      void SetCameraInfoData(int32_t seconds, uint32_t nanoseconds);
      const char* type() const override { return "depth camera"; }

    private:
      bool InitImage();
      bool InitInfo();
      bool PublishImage();
      bool PublishInfo();

      void SetInfoRegionOfInterest( uint32_t x_offset, uint32_t y_offset, uint32_t height, uint32_t width, bool do_rectify);
      void SetData(int32_t seconds, uint32_t nanoseconds, size_t height, size_t width, std::vector<uint8_t>&& data);

    private:
      std::shared_ptr<CarlaDepthCameraPublisherImpl> _impl;
      std::shared_ptr<CarlaCameraInfoPublisherImpl> _impl_info;
  };
}
}
