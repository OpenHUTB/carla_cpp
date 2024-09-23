// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.
 
#pragma once
 
#include "carla/AtomicSharedPtr.h"
#include "carla/NonCopyable.h"
 
#include <algorithm>
#include <mutex>
#include <vector>
 
namespace carla {
namespace client {
namespace detail {
 
  /// 持有一个指向列表的原子指针。
  ///
  /// @warning 仅 Load 方法是原子的，对列表的修改由互斥量锁定。
  template <typename T>
  class AtomicList : private NonCopyable {
    using ListT = std::vector<T>;// 列表类型定义为 std::vector<T>，存储元素 T
  public:
 
    AtomicList() : _list(std::make_shared<ListT>()) {}// 初始化列表为一个空的共享指针
 
    template <typename ValueT>
    void Push(ValueT &&value) {
      std::lock_guard<std::mutex> lock(_mutex);// 锁定互斥量以保证线程安全
      auto new_list = std::make_shared<ListT>(*Load());// 复制当前列表并创建一个新列表
      new_list->emplace_back(std::forward<ValueT>(value)); // 将新值添加到新列表的末尾
      _list = new_list;// 更新原子指针指向新列表
    }
 
    void DeleteByIndex(size_t index) {
      std::lock_guard<std::mutex> lock(_mutex);// 锁定互斥量以保证线程安全
      auto new_list = std::make_shared<ListT>(*Load());// 复制当前列表并创建一个新列表
      auto begin = new_list->begin();// 获取列表的起始迭代器
      std::advance(begin, index);
      new_list->erase(begin);
      _list = new_list;// 更新原子指针指向新列表
    }
 
    template <typename ValueT>
    void DeleteByValue(const ValueT &value) {
      std::lock_guard<std::mutex> lock(_mutex);
      auto new_list = std::make_shared<ListT>(*Load());  // 使用 std::remove 移动所有等于 value 的元素到列表末尾，然后调用 erase 删除这些元素
      new_list->erase(std::remove(new_list->begin(), new_list->end(), value), new_list->end());
      _list = new_list;
    }
 
    void Clear() {
      std::lock_guard<std::mutex> lock(_mutex);
      _list = std::make_shared<ListT>();
    }
 
      /// 返回指向列表的指针。
    std::shared_ptr<const ListT> Load() const {
      return _list.load();
    }
 
  private:
 
    std::mutex _mutex;// 互斥量，用于保护对列表的修改
 
    AtomicSharedPtr<const ListT> _list;// 原子共享指针，指向当前列表
  };
 
} // namespace detail
} // namespace client
} // namespace carla
