// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

// 指示头文件只被包含一次
#pragma once
// 定义 GLIBCXX 的使用 C++11 ABI 的宏
#define _GLIBCXX_USE_CXX11_ABI 0

// 引入内存管理和向量库
#include <memory>
#include <vector>

// 引入 CarlaPublisher 类的定义
#include "CarlaPublisher.h"

namespace carla {
namespace ros2 {

    // 前向声明结构体 CarlaSSCameraPublisherImpl 和 CarlaCameraInfoPublisherImpl
  struct CarlaSSCameraPublisherImpl;
  struct CarlaCameraInfoPublisherImpl;

  // 定义 CarlaSSCameraPublisher 类，继承自 CarlaPublisher
  class CarlaSSCameraPublisher : public CarlaPublisher {
    public:
        // 构造函数，接受 ROS 名称和父级名称作为参数，默认为空字符串
      CarlaSSCameraPublisher(const char* ros_name = "", const char* parent = "");
      // 析构函数
      ~CarlaSSCameraPublisher();
      // 拷贝构造函数
      CarlaSSCameraPublisher(const CarlaSSCameraPublisher&);
      // 拷贝赋值运算符
      CarlaSSCameraPublisher& operator=(const CarlaSSCameraPublisher&);
      // 移动构造函数
      CarlaSSCameraPublisher(CarlaSSCameraPublisher&&);
      // 移动赋值运算符
      CarlaSSCameraPublisher& operator=(CarlaSSCameraPublisher&&);

      // 初始化函数，返回布尔值指示是否成功
      bool Init();
      // 初始化相机信息数据，包括偏移量、图像尺寸和视场角等参数
      void InitInfoData(uint32_t x_offset, uint32_t y_offset, uint32_t height, uint32_t width, float fov, bool do_rectify);
      // 发布数据的函数，返回布尔值指示是否成功
      bool Publish();

      // 检查相机是否已初始化
      bool HasBeenInitialized() const;
      // 设置图像数据的函数，接收时间戳和图像数据
      void SetImageData(int32_t seconds, uint32_t nanoseconds, size_t height, size_t width, const uint8_t* data);
      // 设置相机信息数据的函数，接收时间戳
      void SetCameraInfoData(int32_t seconds, uint32_t nanoseconds);
      // 覆盖基类的 type() 方法，返回传感器类型
      const char* type() const override { return "semantic segmentation"; }

    private:
        // 初始化图像数据的私有函数，返回布尔值指示是否成功
      bool InitImage();
      // 初始化信息的私有函数，返回布尔值指示是否成功
      bool InitInfo();
      // 发布图像的私有函数，返回布尔值指示是否成功
      bool PublishImage();
      // 发布信息的私有函数，返回布尔值指示是否成功
      bool PublishInfo();

      // 设置感兴趣区域信息，包括偏移量、尺寸和是否进行校正
      void SetInfoRegionOfInterest( uint32_t x_offset, uint32_t y_offset, uint32_t height, uint32_t width, bool do_rectify);
      // 内部数据设置函数，使用右值引用方式传递数据
      void SetData(int32_t seconds, uint32_t nanoseconds, size_t height, size_t width, std::vector<uint8_t>&& data);

    private:
        // 使用智能指针管理 CarlaSSCameraPublisherImpl 和 CarlaCameraInfoPublisherImpl 的实例
      std::shared_ptr<CarlaSSCameraPublisherImpl> _impl;
      std::shared_ptr<CarlaCameraInfoPublisherImpl> _impl_info;
  };
}// namespace ros2
}// namespace carla
