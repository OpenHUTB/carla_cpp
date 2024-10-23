// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once // 确保该头文件只会被编译一次

namespace carla { // 定义命名空间 carla，以避免命名冲突

  /// 这个类用于禁止拷贝和移动构造函数及赋值操作
  class NonCopyable {
  public:

    NonCopyable() = default; // 默认构造函数

    // 在C++11中，如果想要禁止类的拷贝行为只需要把相应的函数设为delete即可，参见标准库的std::unique_ptr
    // 而在之前的标准中是把相应的函数作为private函数。
    NonCopyable(const NonCopyable &) = delete; // 禁用拷贝构造函数
    NonCopyable &operator=(const NonCopyable &) = delete; // 禁用拷贝赋值操作符

    NonCopyable(NonCopyable &&) = delete; // 禁用移动构造函数
    NonCopyable &operator=(NonCopyable &&) = delete; // 禁用移动赋值操作符
  };

  /// 这个类用于禁止拷贝构造函数和赋值操作，但允许移动构造函数和赋值操作
  class MovableNonCopyable {
  public:

    MovableNonCopyable() = default; // 默认构造函数

    MovableNonCopyable(const MovableNonCopyable &) = delete; // 禁用拷贝构造函数
    MovableNonCopyable &operator=(const MovableNonCopyable &) = delete; // 禁用拷贝赋值操作符

    MovableNonCopyable(MovableNonCopyable &&) = default; // 允许默认的移动构造函数
    MovableNonCopyable &operator=(MovableNonCopyable &&) = default; // 允许默认的移动赋值操作符
  };

} // 结束命名空间 carla
