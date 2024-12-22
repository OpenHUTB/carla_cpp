// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once  // 使用预处理指令确保该头文件在单个编译单元中只被包含一次，防止重复包含导致的问题。
 
#include "carla/Debug.h"  // 包含CARLA项目中用于调试功能的头文件，可能提供了日志记录、断言等调试工具。
#include "carla/NonCopyable.h"  // 包含定义了`NonCopyable`类的头文件，`NonCopyable`类通过删除复制构造函数和赋值操作符来防止其派生类被复制。
 
#include <thread> // 包含C++标准库中的线程相关头文件，提供了线程的创建、管理、同步等功能。
#include <vector> // 包含C++标准库中的动态数组（向量）头文件，提供了动态数组的数据结构及相关操作。
// 注意：原注释中提到<vector>包含了迭代器相关的头文件，并定义了与迭代器操作相关的功能，这是不准确的。<vector>确实提供了迭代器，但迭代器相关的定义和功能主要在<iterator>头文件中，不过<vector>会间接包含所需的迭代器支持。
 
namespace carla {
 
  // ThreadGroup类用于管理一组线程，并确保在对象销毁时所有线程都已正确终止或等待完成。
  class ThreadGroup : private NonCopyable {  // 继承自NonCopyable类，通过私有继承防止ThreadGroup类被复制。
  public:
 
    // 默认构造函数，不执行任何特殊操作，仅用于创建ThreadGroup对象。
    ThreadGroup() = default;
 
    // 析构函数，在ThreadGroup对象销毁时调用。
    // 它调用JoinAll成员函数来确保所有管理的线程都已正确终止或等待完成，从而避免资源泄漏或未定义行为。
    ~ThreadGroup() {
      JoinAll();  // 销毁对象时确保所有线程都已完成执行。
    }
 
    // 注意：ThreadGroup类的其他成员函数（如添加线程、启动线程、等待线程等）的定义没有在这个代码片段中给出。
    // 这些函数可能用于管理线程组的生命周期，包括线程的创建、启动、等待和终止等操作。
 
    // void JoinAll();  // 这个声明是假设的，实际实现应该在类定义中或相应的源文件中给出。
  };
    
    // 创建一个新线程并执行给定的可调用对象。
    // 模板参数 F 是一个可调用对象类型。

    template <typename F>
    void CreateThread(F &&functor) {
      _threads.emplace_back(std::forward<F>(functor)); // 将线程添加到容器中
    }
    // 创建多个线程，每个线程执行相同的可调用对象
    template <typename F>
    void CreateThreads(size_t count, F functor) {
      _threads.reserve(_threads.size() + count); // 预留足够的空间以避免频繁的内存分配
      for (size_t i = 0u; i < count; ++i) {
        CreateThread(functor); // 创建线程并执行 functor
      }
    }

    // 等待所有线程完成
    void JoinAll() {
      for (auto &thread : _threads) {
        DEBUG_ASSERT_NE(thread.get_id(), std::this_thread::get_id());
        if (thread.joinable()) { // 检查线程是否可加入
          thread.join(); // 等待线程完成
        }
      }
      _threads.clear();  // 清除线程容器
    }

  private:

    std::vector<std::thread> _threads; // 存储线程的容器
  };

} 
