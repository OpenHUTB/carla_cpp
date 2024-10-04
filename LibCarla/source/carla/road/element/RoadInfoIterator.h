// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once // 确保该头文件只被包含一次

#include "carla/Debug.h" // 包含调试相关的头文件
#include "carla/road/element/RoadInfoVisitor.h" // 包含道路信息访问者的头文件

#include <iterator> // 包含迭代器相关的头文件
#include <memory> // 包含智能指针相关的头文件

namespace carla { // 定义 carla 命名空间
namespace road { // 定义 road 命名空间
namespace element { // 定义 element 命名空间

  template <typename T, typename IT> // 模板类，T 是值类型，IT 是迭代器类型
  class RoadInfoIterator : private RoadInfoVisitor { // 道路信息迭代器类，继承自 RoadInfoVisitor
  public:

    static_assert(std::is_same<std::unique_ptr<RoadInfo>, typename IT::value_type>::value, "Not compatible."); // 静态断言，确保迭代器值类型与 unique_ptr<RoadInfo> 兼容

    using value_type = T; // 定义值类型
    using difference_type = typename IT::difference_type; // 定义差值类型
    using pointer = T *; // 定义指针类型
    using reference = T &; // 定义引用类型

    RoadInfoIterator(IT begin, IT end) // 构造函数，接受迭代器的开始和结束
      : _it(begin), // 初始化当前迭代器
        _end(end) { // 初始化结束迭代器
      _success = false; // 初始化成功标志为 false
      for (; !IsAtEnd(); ++_it) { // 遍历迭代器直到结束
        DEBUG_ASSERT((*_it) != nullptr); // 断言当前迭代器指向的对象不为空
        (*_it)->AcceptVisitor(*this); // 调用接受访问者的方法
        if (_success) { // 如果成功，则退出循环
          break;
        }
      }
    }

    RoadInfoIterator &operator++() { // 重载前缀递增运算符
      _success = false; // 重置成功标志为 false
      while (!_success) { // 循环直到成功
        ++_it; // 移动到下一个迭代器元素
        if (IsAtEnd()) { // 检查是否到达结束
          break; // 到达结束则退出
        }
        DEBUG_ASSERT((*_it) != nullptr); // 断言当前迭代器指向的对象不为空
        (*_it)->AcceptVisitor(*this); // 调用接受访问者的方法
      }
      return *this; // 返回自身
    }

    reference operator*() const { // 重载解引用运算符
      DEBUG_ASSERT((*_it) != nullptr); // 断言当前迭代器指向的对象不为空
      return static_cast<T &>(**_it); // 返回当前迭代器指向的对象的引用
    }

    pointer operator->() const { // 重载箭头运算符
      DEBUG_ASSERT((*_it) != nullptr); // 断言当前迭代器指向的对象不为空
      return static_cast<T *>(_it->get()); // 返回当前迭代器指向的对象的指针
    }

    bool operator!=(const RoadInfoIterator &rhs) const { // 重载不等于运算符
      return _it != rhs._it; // 判断当前迭代器与右侧迭代器是否不相等
    }

    bool operator==(const RoadInfoIterator &rhs) const { // 重载等于运算符
      return !((*this) != rhs); // 判断当前迭代器与右侧迭代器是否相等
    }

    bool IsAtEnd() const { // 检查是否到达结束
      return _it == _end; // 返回当前迭代器是否等于结束迭代器
    }

  private:

    void Visit(T &) { // 访问方法，接受一个类型为 T 的对象
      _success = true; // 设置成功标志为 true
    }

    IT _it; // 当前迭代器
    IT _end; // 结束迭代器
    bool _success; // 成功标志
  };

  template <typename T, typename Container> // 模板函数，接受值类型和容器类型
  static auto MakeRoadInfoIterator(const Container &c) { // 创建道路信息迭代器的函数
    auto begin = std::begin(c); // 获取容器的开始迭代器
    auto end = std::end(c); // 获取容器的结束迭代器
    return RoadInfoIterator<T, decltype(begin)>(begin, end); // 返回新的道路信息迭代器
  }

  template <typename T, typename IT> // 模板函数，接受值类型和迭代器类型
  static auto MakeRoadInfoIterator(IT begin, IT end) { // 创建道路信息迭代器的函数
    return RoadInfoIterator<T, decltype(begin)>(begin, end); // 返回新的道路信息迭代器
  }

} // namespace element
} // namespace road
} // namespace carla
