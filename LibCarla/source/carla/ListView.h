// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once  // 确保此头文件仅被包含一次

#include "carla/Debug.h"  // 包含调试相关的头文件

#include <type_traits>  // 包含类型特征相关的头文件
#include <iterator>  // 包含迭代器相关的头文件


namespace carla {

   /// 代表容器中一段元素的视图，基本上是一对起始和结束迭代器。
  template<typename IT>
  class ListView {
  public:

    using iterator = IT;  // 定义迭代器类型
    using const_iterator = typename std::add_const<IT>::type;   // 定义常量迭代器类型
    using size_type = size_t;   // 定义大小类型
    using difference_type = typename std::iterator_traits<iterator>::difference_type;  // 定义差异类型
    using value_type = typename std::iterator_traits<iterator>::value_type;   // 定义值类型
    using pointer = typename std::iterator_traits<iterator>::pointer;  // 定义指针类型
    using reference = typename std::iterator_traits<iterator>::reference;  // 定义引用类型

    // 构造函数，接受开始和结束迭代器
    explicit ListView(iterator begin, iterator end)
      : _begin(begin), _end(end) {  // 初始化成员变量
      DEBUG_ASSERT(std::distance(_begin, _end) >= 0);  // 确保范围有效
    }

    ListView(const ListView &) = default;   // 默认复制构造函数
    ListView &operator=(const ListView &) = delete;   // 删除赋值运算符

    iterator begin() {  // 返回迭代器的开始位置
      return _begin;
    }

    const_iterator begin() const {  // 返回常量迭代器的开始位置
      return _begin;
    }

    const_iterator cbegin() const {   // 返回常量迭代器的开始位置（常量版本）
      return _begin;
    }

    iterator end() {  // 返回迭代器的结束位置
      return _end;
    }

    const_iterator end() const {  // 返回常量迭代器的结束位置

      return _end;
    }

    const_iterator cend() const {  // 返回常量迭代器的结束位置（常量版本）
      return _end;
    }

    bool empty() const {  // 检查视图是否为空
      return _begin == _end;  // 如果开始和结束迭代器相等则为空
    }

    size_type size() const {   // 返回视图中的元素数量
      return static_cast<size_t>(std::distance(begin(), end()));  // 计算开始和结束之间的距离
    }

  private:

    const iterator _begin;  // 成员变量，表示开始迭代器

    const iterator _end;  // 成员变量，表示结束迭代器
  };

  template <typename Iterator>  // 模板函数，接受迭代器类型
  static inline auto MakeListView(Iterator begin, Iterator end) {   // 创建 ListView 实例
    return ListView<Iterator>(begin, end);  // 返回新的 ListView
  }

  template <typename Container> // 模板函数，接受容器类型 
  static inline auto MakeListView(Container &c) {  // 创建 ListView 实例
    return MakeListView(std::begin(c), std::end(c));   // 使用容器的 begin 和 end 创建 ListView
  }

} // namespace carla
