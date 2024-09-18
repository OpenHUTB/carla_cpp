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

    explicit ListView(iterator begin, iterator end)
      : _begin(begin), _end(end) {
      DEBUG_ASSERT(std::distance(_begin, _end) >= 0);
    }

    ListView(const ListView &) = default;
    ListView &operator=(const ListView &) = delete;

    iterator begin() {
      return _begin;
    }

    const_iterator begin() const {
      return _begin;
    }

    const_iterator cbegin() const {
      return _begin;
    }

    iterator end() {
      return _end;
    }

    const_iterator end() const {
      return _end;
    }

    const_iterator cend() const {
      return _end;
    }

    bool empty() const {
      return _begin == _end;
    }

    size_type size() const {
      return static_cast<size_t>(std::distance(begin(), end()));
    }

  private:

    const iterator _begin;

    const iterator _end;
  };

  template <typename Iterator>
  static inline auto MakeListView(Iterator begin, Iterator end) {
    return ListView<Iterator>(begin, end);
  }

  template <typename Container>
  static inline auto MakeListView(Container &c) {
    return MakeListView(std::begin(c), std::end(c));
  }

} // namespace carla
