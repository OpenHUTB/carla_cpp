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
// 前置声明结构体CarlaRGBCameraPublisherImpl，通常用于隐藏具体实现细节，可能在后续代码中有实际的结构体定义与之对应
  struct CarlaRGBCameraPublisherImpl;
  // 前置声明结构体CarlaCameraInfoPublisherImpl，作用与上面类似，也是可能用于后续具体实现相关功能的结构体声明
  struct CarlaCameraInfoPublisherImpl;
 // 定义CarlaRGBCameraPublisher类，它继承自CarlaPublisher类，意味着它可以使用CarlaPublisher类中的公有成员以及重写（如果需要的话）虚函数等
  class CarlaRGBCameraPublisher : public CarlaPublisher {
    public: // 定义CarlaRGBCameraPublisher类，它继承自CarlaPublisher类，意味着它可以使用CarlaPublisher类中的公有成员以及重写（如果需要的话）虚函数等
      CarlaRGBCameraPublisher(const char* ros_name = "", const char* parent = "");
// 构造函数，用于创建CarlaRGBCameraPublisher类的对象。可以传入两个字符串参数，分别用于指定ROS中的名称（ros_name）和父节点名称（parent），如果不传参数则使用默认值（空字符串）
      ~CarlaRGBCameraPublisher(); // 拷贝构造函数，用于通过已有的CarlaRGBCameraPublisher对象来创建一个新的、一模一样的对象（进行深拷贝或者浅拷贝，取决于具体实现）
      CarlaRGBCameraPublisher(const CarlaRGBCameraPublisher&);// 移动构造函数，用于通过“窃取”资源的方式，高效地创建一个新的CarlaRGBCameraPublisher对象，常用于优化对象传递时的性能，避免不必要的拷贝
      CarlaRGBCameraPublisher(CarlaRGBCameraPublisher&&);   / /移动赋值运算符重载函数，类似移动构造函数的作用，不过是用于赋值操作时高效地转移资源所有权     
      CarlaRGBCameraPublisher& operator=(CarlaRGBCameraPublisher&&);        // 用于初始化相关资源或者执行一些初始化操作，返回一个布尔值表示初始化是否成功

      bool Init(); // 用于初始化信息数据，传入图像在水平和垂直方向上的偏移量（x_offset、y_offset）、图像的高度（height）、宽度（width）、视场角（fov）以及是否进行矫正（do_rectify）等参数
      void InitInfoData(uint32_t x_offset, uint32_t y_offset, uint32_t height, uint32_t width, float fov, bool do_rectify); // 用于发布数据，返回布尔值表示发布操作是否成功
      bool Publish();
 // 用于检查该对象是否已经完成初始化，返回布尔值，const表示这个函数不会修改对象的状态
      bool HasBeenInitialized() const;// 用于设置图像数据，传入时间相关的秒数（seconds）、纳秒数（nanoseconds）、图像的高度（height）、宽度（width）以及图像数据指针（data）
      void SetImageData(int32_t seconds, uint32_t nanoseconds, uint32_t height, uint32_t width, const uint8_t* data); // 用于设置相机信息数据，传入时间相关的秒数（seconds）和纳秒数（nanoseconds）
      void SetCameraInfoData(int32_t seconds, uint32_t nanoseconds);// 重写基类CarlaPublisher中的虚函数type，返回表示类型的字符串"rgb camera"，用于标识该发布者对应的是RGB相机类型
      const char* type() const override { return "rgb camera"; }

    private:
 // 用于初始化图像相关的内部资源等操作，返回布尔值表示是否初始化成功
      bool InitImage(); // 用于初始化信息相关的内部资源等操作，返回布尔值表示是否初始化成功
      bool InitInfo();// 用于发布图像相关的数据，返回布尔值表示发布是否成功
      bool PublishImage(); // 用于发布信息相关的数据，返回布尔值表示发布是否成功
      bool PublishInfo();

      void SetImageData(int32_t seconds, uint32_t nanoseconds, uint32_t height, uint32_t width, std::vector<uint8_t>&& data);
      void SetInfoRegionOfInterest(uint32_t x_offset, uint32_t y_offset, uint32_t height, uint32_t width, bool do_rectify);

    private:
      std::shared_ptr<CarlaRGBCameraPublisherImpl> _impl;
      std::shared_ptr<CarlaCameraInfoPublisherImpl> _impl_info;
  };
}
}
