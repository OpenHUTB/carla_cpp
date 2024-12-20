// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.
// 这是一个预处理指令，用于确保头文件只被编译一次，避免重复包含导致的编译错误
#pragma once
// 定义 C++11 ABI（应用二进制接口）相关设置为0，通常用于控制库的链接和二进制兼容性等方面，
// 例如影响不同编译器版本或者不同编译选项下库的交互方式
#define _GLIBCXX_USE_CXX11_ABI 0
// 引入标准库中的智能指针头文件，用于管理动态分配的对象，如这里的CarlaMapSensorPublisherImpl对象
#include <memory>

#include "CarlaPublisher.h"

namespace carla {
namespace ros2 {
// 前置声明CarlaMapSensorPublisherImpl结构体，告诉编译器存在这样一个结构体类型，
    // 但具体的结构体定义在后面或者其他地方给出，这样可以解决编译顺序依赖的问题，
    // 例如在类中使用该结构体类型的指针时，编译器知道这是一个合法的类型即可
  struct CarlaMapSensorPublisherImpl;
// CarlaMapSensorPublisher类定义，继承自CarlaPublisher类，
    // 可能用于发布地图传感器相关的数据，实现特定的发布功能和遵循相关的发布逻辑
  class CarlaMapSensorPublisher : public CarlaPublisher {
    public:
 // 构造函数，用于创建CarlaMapSensorPublisher对象
        // 参数ros_name用于指定该发布者在ROS系统中的名称，默认为空字符串，
        // 参数parent用于指定父节点名称，默认为"", 可以用于构建层次化的名称结构等
      CarlaMapSensorPublisher(const char* ros_name = "", const char* parent = "");
// 析构函数，用于在对象销毁时进行资源清理工作，比如释放动态分配的内存等相关操作
      ~CarlaMapSensorPublisher();
// 拷贝构造函数，用于通过已有的CarlaMapSensorPublisher对象创建一个新的相同的对象，
        // 进行深拷贝或者浅拷贝相关的操作，确保新对象的状态和原对象一致（取决于具体实现）
      CarlaMapSensorPublisher(const CarlaMapSensorPublisher&);
// 拷贝构造函数，用于通过已有的CarlaMapSensorPublisher对象创建一个新的相同的对象，
        // 进行深拷贝或者浅拷贝相关的操作，确保新对象的状态和原对象一致（取决于具体实现）
      CarlaMapSensorPublisher& operator=(const CarlaMapSensorPublisher&);
// 移动构造函数，通过将已有对象的资源所有权转移给新对象来高效地创建新对象，
        // 避免不必要的资源复制开销，常用于涉及动态资源管理的情况
      CarlaMapSensorPublisher(CarlaMapSensorPublisher&&);
      CarlaMapSensorPublisher& operator=(CarlaMapSensorPublisher&&);

      bool Init();
      bool Publish();
      void SetData(const char* data);
 // 重写基类（CarlaPublisher）中的虚函数type，返回表示该发布者类型的字符串"map sensor"，
        // 可以用于在运行时识别发布者的具体类型等用途
      const char* type() const override { return "map sensor"; }

    private:
// 使用std::shared_ptr智能指针来管理CarlaMapSensorPublisherImpl类型的对象，
        // 实现自动的内存管理，避免手动释放内存导致的问题，如内存泄漏等
      std::shared_ptr<CarlaMapSensorPublisherImpl> _impl;
  };
}
}
