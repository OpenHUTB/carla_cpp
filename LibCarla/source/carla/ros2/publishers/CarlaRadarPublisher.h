// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once    // 定义全局宏，设置 C++11 ABI 为 0，可能影响 C++ 标准库的一些行为，如动态链接库的符号解析等
#define _GLIBCXX_USE_CXX11_ABI 0   // 引入内存管理头文件，用于处理智能指针等内存相关操作
#define _GLIBCXX_USE_CXX11_ABI 0   // 引入向量头文件，用于处理动态大小数组，可能用于存储雷达数据等

#include <memory>     // 引入向量头文件，用于处理动态大小数组，可能用于存储雷达数据等
#include <vector>   // 引入自定义的 CarlaPublisher.h 头文件，CarlaRadarPublisher 类可能继承自 CarlaPublisher 或依赖其定义

#include "CarlaPublisher.h"

namespace carla {   
namespace ros2 {           // 前置声明 CarlaRadarPublisherImpl 结构体，其完整定义可能在其他文件中，通过智能指针在本类中使用

  struct CarlaRadarPublisherImpl;  // CarlaRadarPublisher 类定义，继承自 CarlaPublisher

  class CarlaRadarPublisher : public CarlaPublisher {
    public:    // 构造函数，可传入 ROS 名称和父名称，用于初始化对象，默认参数为空字符串
      CarlaRadarPublisher(const char* ros_name = "", const char* parent = "");  // 析构函数，用于释放对象占用的资源
      ~CarlaRadarPublisher();     // 拷贝构造函数，用于创建当前对象的副本，进行深拷贝或浅拷贝（取决于实现）
      CarlaRadarPublisher(const CarlaRadarPublisher&);  // 拷贝赋值运算符重载，用于将一个对象的值赋给当前对象，处理资源和数据复制
      CarlaRadarPublisher& operator=(const CarlaRadarPublisher&);  // 移动构造函数，通过移动语义高效转移资源所有权，避免不必要拷贝开销
      CarlaRadarPublisher(CarlaRadarPublisher&&);   // 移动赋值运算符重载，实现移动语义的赋值操作
      CarlaRadarPublisher& operator=(CarlaRadarPublisher&&);  // 初始化函数，可能用于初始化发布雷达数据相关的内部状态、资源或与外部系统连接，返回初始化成功与否

      bool Init();// 发布函数，将准备好的雷达数据发布出去，返回发布操作是否成功
      bool Publish();// 设置雷达数据函数，传入时间戳（秒和纳秒）、数据维度（高度、宽度、元素个数）及数据指针
      void SetData(int32_t seconds, uint32_t nanoseconds, size_t height, size_t width, size_t elements, const uint8_t* data);
      // 重写的虚函数，返回 "radar" 标识此发布者发布的数据类型为雷达数据
      const char* type() const override { return "radar"; }

    private:  // 重载的 SetData 函数，接收右值引用的向量数据，利用移动语义高效设置数据，避免拷贝
      void SetData(int32_t seconds, uint32_t nanoseconds, size_t height, size_t width, size_t elements, std::vector<uint8_t>&& data);

    private:   // 智能指针指向 CarlaRadarPublisherImpl 结构体，用于解耦接口与内部实现，处理内部逻辑
      std::shared_ptr<CarlaRadarPublisherImpl> _impl;
  };
}
}
