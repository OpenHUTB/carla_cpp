// 版权所有 (c) 2017 巴塞罗那自治大学 (UAB) 计算机视觉中心 (CVC)。
//
// 本作品依据 MIT 许可证的条款进行授权。
// 如需获取副本，请访问 https://opensource.org/licenses/MIT。
 
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
  /// 这个类管理一个线程安全的列表，其中只有加载操作是原子的。
  /// 对列表的修改操作（如添加、删除）则通过互斥锁来保证线程安全。
  ///
  /// @warning 仅 Load 方法是原子的，对列表的修改由互斥量锁定。
  template <typename T>
  class AtomicList : private NonCopyable {
    using ListT = std::vector<T>;// 列表类型定义为 std::vector<T>，存储元素 T
  public:
    /// 构造函数，初始化列表为一个空的共享指针。
    AtomicList() : _list(std::make_shared<ListT>()) {}

    /// 向列表中添加一个新元素。
    ///
    /// @param value 要添加的新元素的值。
    template <typename ValueT>
    void Push(ValueT &&value) {
      std::lock_guard<std::mutex> lock(_mutex);// 锁定互斥量以保证线程安全
      auto new_list = std::make_shared<ListT>(*Load());// 复制当前列表并创建一个新列表
      new_list->emplace_back(std::forward<ValueT>(value)); // 将新值添加到新列表的末尾
      _list = new_list;// 更新原子指针指向新列表
    }

    /// 根据索引删除列表中的元素。
    ///
    /// @param index 要删除的元素的索引。
    void DeleteByIndex(size_t index) {
      std::lock_guard<std::mutex> lock(_mutex);// 锁定互斥量以保证线程安全
      auto new_list = std::make_shared<ListT>(*Load());// 复制当前列表并创建一个新列表
      auto begin = new_list->begin();// 获取列表的起始迭代器
      std::advance(begin, index);// 将迭代器移动到要删除的元素的位置
      new_list->erase(begin);// 删除指定位置的元素
      _list = new_list;// 更新原子指针指向新列表
    }
   
    /// 根据值删除列表中的元素。
    ///
    /// @param value 要删除的元素的值。
    /// @note 使用 std::remove 将所有等于 value 的元素移动到列表末尾，然后调用 erase 删除这些元素。
    template <typename ValueT>
    void DeleteByValue(const ValueT &value) {
      // 锁定互斥量以保证线程安全
      std::lock_guard<std::mutex> lock(_mutex);
      // 复制当前列表并创建一个新列表
      auto new_list = std::make_shared<ListT>(*Load());  // 使用 std::remove 移动所有等于 value 的元素到列表末尾，然后调用 erase 删除这些元素
      new_list->erase(std::remove(new_list->begin(), new_list->end(), value), new_list->end()); // 删除从 new_end 到列表末尾的所有元素
      // 更新原子指针指向新列表
      _list = new_list;
    }
 
    void Clear() {
      std::lock_guard<std::mutex> lock(_mutex);// 使用std::lock_guard自动管理互斥锁的锁定和解锁，确保在多线程环境下对_list的访问是线程安全的。
      _list = std::make_shared<ListT>(); // 将_list重置为一个新的共享列表，清空所有之前的内容。这样做会清空所有之前存储在_list中的内容。
    }
 
    /// 返回指向列表的指针。
    std::shared_ptr<const ListT> Load() const {
      return _list.load();
    }
 
  private:
 
    std::mutex _mutex;// 互斥量，用于在多线程环境下保护对_list的访问，确保同时只有一个线程可以修改_list指向的列表内容。
 
    AtomicSharedPtr<const ListT> _list;// 原子共享指针，指向当前列表，避免数据竞争和不一致的问题。
  };
 
} // namespace detail
} // namespace client
} // namespace carla
