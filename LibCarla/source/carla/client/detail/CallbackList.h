// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>. // 这是一个预处理指令，用于确保头文件只被包含一次，避免重复定义的问题

#pragma once
// 包含CARLA相关的原子列表（AtomicList）头文件，可能用于实现线程安全的列表操作等功能
#include "carla/AtomicList.h" // 包含不可拷贝类的基类定义，用于让派生类禁止拷贝操作，保证对象的唯一性等
#include "carla/NonCopyable.h" // 引入C++标准库中的原子类型相关头文件，原子类型常用于多线程环境下保证数据操作的原子性，避免数据竞争等问题
// 包含时间相关的定义，比如时间类型、时间操作函数等，用于处理模拟器中的时间相关逻辑
#include <atomic> // 引入C++标准库中的函数对象头文件，用于支持可调用对象、函数指针等相关操作
#include <functional>
// 定义在carla命名空间下的client命名空间里的detail命名空间中，通常表示这是一个内部实现细节相关的类定义
namespace carla {
namespace client {
namespace detail {
// 定义一个模板类CallbackList，它可以接受任意数量（由...表示可变参数模板）的模板参数类型InputsT
  // 这个类继承自NonCopyable，表示该类对象不可拷贝，可能是为了避免一些由于意外拷贝导致的逻辑错误或资源管理问题
  template <typename... InputsT>
  class CallbackList : private NonCopyable {
  public:
// 定义一个类型别名CallbackType，它是一个函数对象类型，接受类型为InputsT...的参数，并且无返回值
    // 这意味着可以使用CallbackType来方便地声明符合这种函数签名的函数、函数指针、lambda表达式等作为可调用对象
    using CallbackType = std::function<void(InputsT...)>;
// 定义Call函数，它用于依次调用存储在列表中的所有回调函数（CallbackType类型的函数对象）
    // 并将可变参数args传递给每个回调函数，const表示该函数不会修改类的成员变量
    void Call(InputsT... args) const { // 通过原子加载操作获取当前的列表（_list是一个AtomicList类型，用于保证在多线程环境下对列表操作的原子性）
      auto list = _list.Load(); // 遍历列表中的每一个元素（每个元素是一个Item结构体，包含一个唯一标识id和一个回调函数callback）
      for (auto &item : *list) { // 调用当前元素对应的回调函数，并传递参数args
        item.callback(args...);
      }
    }
// 定义Push函数，用于向回调函数列表中添加一个新的回调函数
    // 它接受一个右值引用类型的回调函数（CallbackType &&callback），这样可以避免不必要的拷贝，提高效率
    // 返回一个size_t类型的唯一标识（id），用于后续可以通过这个标识来移除对应的回调函数
    size_t Push(CallbackType &&callback) { // 对计数器（_counter）进行自增操作，获取一个新的唯一标识，并且断言这个标识不等于0（确保唯一性）
      auto id = ++_counter;
      DEBUG_ASSERT(id != 0u); // 将包含新生成的唯一标识id和传入的回调函数的Item结构体添加到原子列表（_list）中
      _list.Push(Item{id, std::move(callback)});
      return id;
    }

    void Remove(size_t id) {
      _list.DeleteByValue(id);
    }

    void Clear() {
      _list.Clear();
    }

  private:

    struct Item {
      size_t id;
      CallbackType callback;

      friend bool operator==(const Item &lhs, const Item &rhs) {
        return lhs.id == rhs.id;
      }

      friend bool operator==(const Item &lhs, size_t rhs) {
        return lhs.id == rhs;
      }
// 重载==操作符，用于比较一个size_t类型的值和一个Item结构体是否相等，也是通过比较唯一标识id来判断
      friend bool operator==(size_t lhs, const Item &rhs) {
        return lhs == rhs.id;
      }
    };

    std::atomic_size_t _counter{0u};
// 定义一个原子列表（AtomicList）类型的成员变量（_list），用于存储包含回调函数及其唯一标识的Item结构体
    // 原子列表保证了在多线程环境下对列表进行添加、删除等操作的原子性，避免数据不一致等问题
    AtomicList<Item> _list;
  };

} // namespace detail
} // namespace client
} // namespace carla
