// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once  // 确保头文件只被包含一次，避免重复包含导致的问题

#include "carla/Debug.h"  // 包含自定义调试功能的头文件
#include "carla/NonCopyable.h"  // 包含定义 NonCopyable 类的头文件，避免类被复制

#include <thread> // 包含线程相关的库
#include <vector>

namespace carla {

  // ThreadGroup 类用于管理一组线程，并确保它们在销毁时被正确地等待
  class ThreadGroup : private NonCopyable {  // 继承自 NonCopyable，防止复制该类
  public:

    ThreadGroup() = default; // 默认构造函数

    ~ThreadGroup() {  // 析构函数
      JoinAll();  // 在销毁时确保所有线程都已完成
    }
    
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
