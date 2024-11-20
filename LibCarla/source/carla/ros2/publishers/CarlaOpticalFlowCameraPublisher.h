// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
// 这是一个预处理指令，用于确保头文件只被包含一次，避免重复定义等问题
#define _GLIBCXX_USE_CXX11_ABI 0

#include <memory>
#include <vector>
// 引入C++ 标准库中的智能指针（memory）和动态数组（vector）相关头文件，方便进行内存管理和动态大小数据的存储操作
#include "CarlaPublisher.h"
// 引入自定义的CarlaPublisher.h头文件，推测其中定义了与发布者相关的基类或其他必要的类型、函数等供当前类继承或使用
namespace carla {
namespace ros2 {

  struct CarlaOpticalFlowCameraPublisherImpl;
  struct CarlaCameraInfoPublisherImpl;
  // 前向声明两个结构体类型，这里只是声明了它们的存在，具体定义可能在其他地方（通常是.cpp文件中），这样做可以解决循环引用等编译问题，
  // 并且在类中可以先使用它们的指针类型等，而不用完整知道它们的内部结构
  class CarlaOpticalFlowCameraPublisher : public CarlaPublisher {
    public:
      CarlaOpticalFlowCameraPublisher(const char* ros_name = "", const char* parent = "");
      // 构造函数，用于创建CarlaOpticalFlowCameraPublisher类的对象，可以传入ROS名称和父节点名称作为参数
      // 如果不传参数，则使用默认的空字符串作为参数值
      ~CarlaOpticalFlowCameraPublisher();
      // 析构函数，用于在对象销毁时进行必要的资源清理工作，比如释放动态分配的内存等

      CarlaOpticalFlowCameraPublisher(const CarlaOpticalFlowCameraPublisher&);
      // 拷贝构造函数，用于使用另一个同类型对象来初始化当前对象，进行深拷贝或浅拷贝相关操作
      CarlaOpticalFlowCameraPublisher& operator=(const CarlaOpticalFlowCameraPublisher&);
      // 拷贝赋值运算符重载，用于将一个同类型对象的值赋给当前对象，同样涉及深拷贝或浅拷贝的逻辑
      CarlaOpticalFlowCameraPublisher(CarlaOpticalFlowCameraPublisher&&);
      // 移动构造函数，实现将资源从一个即将销毁的对象“移动”到新创建的对象，避免不必要的拷贝开销，常用于右值引用场景
      CarlaOpticalFlowCameraPublisher& operator=(CarlaOpticalFlowCameraPublisher&&);
      // 移动赋值运算符重载，执行类似移动构造函数的资源移动操作，用于对象之间赋值时优化性能
      bool Init();
      // 初始化函数，用于对发布者相关的一些资源、状态等进行初始化操作，返回一个布尔值表示初始化是否成功
      void InitInfoData(uint32_t x_offset, uint32_t y_offset, uint32_t height, uint32_t width, float fov, bool do_rectify);
      // 用于初始化相机信息相关的数据，传入相机图像在水平和垂直方向的偏移量、高度、宽度、视场角以及是否进行矫正等参数
      bool Publish();
      // 执行发布操作，将相关数据发布出去,返回发布是否成功的布尔值

      bool HasBeenInitialized() const;
      // 用于查询对象是否已经完成初始化，返回一个布尔值

      void SetImageData(int32_t seconds, uint32_t nanoseconds, size_t height, size_t width, const float* data);
      // 设置图像数据，传入时间戳（秒和纳秒部分）、图像的高度、宽度以及指向图像数据的指针
      void SetCameraInfoData(int32_t seconds, uint32_t nanoseconds);
      // 设置相机信息数据的时间戳，传入秒和纳秒部分的时间值
      const char* type() const override { return "optical flow camera"; }
      // 重写基类（CarlaPublisher）中的虚函数type，返回表示当前发布者类型的字符串，这里返回"optical flow camera"，用于标识这是一个光流相机发布者

    private:
      bool InitImage();
      // 私有函数，用于初始化图像相关的资源或状态，返回初始化是否成功的布尔值
      bool InitInfo();
      // 私有函数，用于初始化相机信息相关的资源或状态，返回初始化是否成功的布尔值
      bool PublishImage();
      // 私有函数，用于执行图像数据的发布操作，返回发布是否成功的布尔值
      bool PublishInfo();
      // 私有函数，用于执行相机信息数据的发布操作，返回发布是否成功的布尔值
      void SetInfoRegionOfInterest( uint32_t x_offset, uint32_t y_offset, uint32_t height, uint32_t width, bool do_rectify);
      // 私有函数，用于设置相机信息中的感兴趣区域，传入水平和垂直方向的偏移量、高度、宽度以及是否进行矫正等参数
      void SetData(int32_t seconds, uint32_t nanoseconds, size_t height, size_t width, std::vector<uint8_t>&& data);
      // 私有函数，用于设置数据
      // 这里使用右值引用接收动态数组，可能是为了高效传递临时数据，避免不必要的拷贝
    private:
      std::shared_ptr<CarlaOpticalFlowCameraPublisherImpl> _impl;
      // 定义一个智能指针，指向CarlaOpticalFlowCameraPublisherImpl结构体类型，用于管理该类型对象的生命周期
      std::shared_ptr<CarlaCameraInfoPublisherImpl> _impl_info;
      // 类似地，定义一个智能指针，指向CarlaCameraInfoPublisherImpl结构体类型，用于管理相机信息发布相关的实现对象的生命周期
  };
  };
}
}
