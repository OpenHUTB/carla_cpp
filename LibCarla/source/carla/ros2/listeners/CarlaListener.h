// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once // 防止头文件被多次包含（即，确保此头文件在每个编译单元中只被包含一次）。  
  
// 设置 _GLIBCXX_USE_CXX11_ABI 为 0，这是为了兼容旧的 C++ ABI（应用程序二进制接口）。  
// 这通常用于确保与在 GCC 5 之前编译的二进制库的兼容性。  
#define _GLIBCXX_USE_CXX11_ABI 0  
  
#include <memory> // 包含C++标准库中的内存管理功能，如智能指针。  
  
// 定义命名空间 carla，在其内部再定义一个子命名空间 ros2。  
// 这有助于组织代码，避免命名冲突。  
namespace carla {  
namespace ros2 {  
  
  // 前向声明 CarlaListenerImpl 类。  
  // 这意味着在此处声明了一个名为 CarlaListenerImpl 的类，  
  // 但其定义将在其他地方给出。这样做通常是为了解决循环依赖问题。  
  class CarlaListenerImpl;  
  
  // CarlaListener 类定义开始。  
  class CarlaListener {  
    public:  
      // 默认构造函数。  
      CarlaListener();  
        
      // 析构函数。  
      ~CarlaListener();  
        
      // 删除拷贝构造函数，防止对象被拷贝。  
      CarlaListener(const CarlaListener&) = delete;  
        
      // 删除拷贝赋值运算符，防止对象通过赋值操作被拷贝。  
      CarlaListener& operator=(const CarlaListener&) = delete;  
        
      // 删除移动构造函数，防止对象被移动。  
      CarlaListener(CarlaListener&&) = delete;  
        
      // 删除移动赋值运算符，防止对象通过赋值操作被移动。  
      CarlaListener& operator=(CarlaListener&&) = delete;  
  
      // 使用 std::unique_ptr 管理 CarlaListenerImpl 对象的生命周期。  
      // 这确保了 CarlaListenerImpl 对象在 CarlaListener 对象被销毁时也会被正确销毁。  
      // 同时，这也提供了异常安全性和内存泄漏保护。  
      std::unique_ptr<CarlaListenerImpl> _impl;  
  };  
}  
