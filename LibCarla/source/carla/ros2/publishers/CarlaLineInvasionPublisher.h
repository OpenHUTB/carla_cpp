// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
#define _GLIBCXX_USE_CXX11_ABI 0

#include <memory>

#include "CarlaPublisher.h"

namespace carla {
namespace ros2 {
 // 前向声明一个名为 CarlaLineInvasionPublisherImpl 的结构体，
        // 通常这样做是因为在当前编译单元中暂时不需要知道该结构体的完整定义（可能其定义在其他源文件中），
        // 但在类中又需要以指针等形式使用它，先声明一下告诉编译器它是存在的结构体类型即可
  struct CarlaLineInvasionPublisherImpl;
// 定义 CarlaLineInvasionPublisher 类，它继承自 CarlaPublisher 类，意味着它会继承 CarlaPublisher 的公有成员和接口等，
        // 并在此基础上可能扩展自己特有的功能，这个类大概率是用于在 ROS2 环境下发布某种和 "line invasion"（线入侵，推测是和自动驾驶等领域相关概念）相关的消息
  class CarlaLineInvasionPublisher : public CarlaPublisher {
    public
 // 构造函数，用于创建 CarlaLineInvasionPublisher 类的对象，
                // 接收两个可选的参数，ros_name 默认值为空字符串，parent 默认值为 ""，可用于初始化对象相关属性，比如设置发布者在 ROS2 中的名称等
      CarlaLineInvasionPublisher(const char* ros_name = "", const char* parent = "");
 // 析构函数，用于在对象生命周期结束时进行资源清理等操作，比如释放内存、关闭文件句柄等相关资源，
                // 这里会释放该类对象所占用的相关资源（具体取决于类内部的实现以及其包含的成员等情况）
      ~CarlaLineInvasionPublisher();
 // 析构函数，用于在对象生命周期结束时进行资源清理等操作，比如释放内存、关闭文件句柄等相关资源，
                // 这里会释放该类对象所占用的相关资源（具体取决于类内部的实现以及其包含的成员等情况）
      CarlaLineInvasionPublisher(const CarlaLineInvasionPublisher&);
// 拷贝赋值运算符重载，用于将一个同类型的对象赋值给当前对象，
                // 返回当前对象的引用，以便可以进行连续赋值操作（如 a = b = c; 这样的语法形式），同样如果没有显式定义，编译器会生成默认版本
      CarlaLineInvasionPublisher& operator=(const CarlaLineInvasionPublisher&);
  // 移动构造函数，用于通过 "窃取" 其他同类型对象的资源来初始化新对象，而不是进行拷贝操作，
                // 这样可以提高效率，特别是对于包含大量资源（如动态分配内存等）的对象，参数是右值引用类型，表示可以接收临时对象等即将销毁的资源
      CarlaLineInvasionPublisher(CarlaLineInvasionPublisher&&);
// 移动赋值运算符重载，类似移动构造函数的作用，将一个右值引用类型的同类型对象的资源转移给当前对象，
                // 并返回当前对象的引用，实现高效的资源转移赋值操作，同样是为了优化性能，避免不必要的拷贝
      CarlaLineInvasionPublisher& operator=(CarlaLineInvasionPublisher&&);
// 用于进行一些初始化相关的操作，比如初始化 ROS2 发布者、连接到相关的 ROS2 节点等，
                // 返回一个 bool 值，用于指示初始化是否成功，如果返回 true 表示初始化操作顺利完成，否则表示出现问题
      bool Init();
// 用于进行一些初始化相关的操作，比如初始化 ROS2 发布者、连接到相关的 ROS2 节点等，
                // 返回一个 bool 值，用于指示初始化是否成功，如果返回 true 表示初始化操作顺利完成，否则表示出现问题
      bool Publish();
// 用于设置要发布的数据，接收三个参数，分别是表示时间的秒数（int32_t 类型）、纳秒数（uint32_t 类型）以及数据指针（指向 int32_t 类型数组，推测是具体的 "line invasion" 相关数据内容），
                // 这个函数的作用是准备好要发布的数据，以便后续调用 Publish 函数进行发布操作
      void SetData(int32_t seconds, uint32_t nanoseconds, const int32_t* data);
// 重写了基类（CarlaPublisher）中的虚函数 type，用于返回该发布者所发布消息的类型名称，
                // 在这个类中固定返回 "line invasion"，表明发布的是和 "线入侵" 相关的消息类型
      const char* type() const override { return "line invasion"; }

    private:
 // 使用 std::shared_ptr 智能指针来管理 CarlaLineInvasionPublisherImpl 类型的对象，
                // 这样可以方便地进行内存管理，避免手动释放内存导致的内存泄漏等问题，同时可以实现多个对象共享同一份资源（如果需要的话）
      std::shared_ptr<CarlaLineInvasionPublisherImpl> _impl;
  };
}
}
