// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <mutex>
#include <unordered_map>
/**
 * @namespace carla::traffic_manager
 *
 * @brief Carla项目的交通管理命名空间。
 */
namespace carla {
namespace traffic_manager {
    /**
      * @class AtomicMap
      * @brief 一个线程安全的、基于unordered_map的原子映射类。
      *
      * 这个模板类提供了一个线程安全的映射，使用mutex来确保对内部unordered_map的并发访问是安全的。
      *
      * @tparam Key 映射的键类型。
      * @tparam Value 映射的值类型。
      */
  template <typename Key, typename Value>
  class AtomicMap {

    private:
        /**
       * @brief 用于同步对map的访问的可变互斥锁。
       *
       * 这个互斥锁用于保护对内部unordered_map的并发访问，确保线程安全。
       */
    mutable std::mutex map_mutex;
    /**
       * @brief 存储键值对的无序映射。
       */
    std::unordered_map<Key, Value> map;

    public:
        /**
          * @brief 默认构造函数。
          */
    AtomicMap() {}
    /**
       * @brief 添加或更新键值对。
       *
       * 如果键已存在，则更新其对应的值；如果键不存在，则插入新的键值对。
       *
       * @param entry 要添加或更新的键值对。
       */
    void AddEntry(const std::pair<Key, Value> &entry) {

      std::lock_guard<std::mutex> lock(map_mutex);// 加锁以保护对map的访问 
      const Key& key = entry.first;
      if (map.find(key) != map.end()) {
        map.at(key) = entry.second;// 更新已存在的键的值
      } else {
        map.insert(entry);// 插入新的键值对
      }
    }
    /**
       * @brief 检查映射是否包含指定的键。
       *
       * @param key 要检查的键。
       * @return 如果映射包含指定的键，则返回true；否则返回false。
       */
    bool Contains(const Key &key) const {

      std::lock_guard<std::mutex> lock(map_mutex);// 加锁以保护对map的访问
      return map.find(key) != map.end();// 检查键是否存在  
    }
    /**
       * @brief 获取指定键的值。
       *
       * @param key 要获取值的键。
       * @return 指定键对应的值。如果键不存在，则行为未定义（通常会导致抛出异常）。
       */
    const Value &GetValue(const Key &key) const {

      std::lock_guard<std::mutex> lock(map_mutex);// 加锁以保护对map的访问
      return map.at(key);// 返回指定键的值
    }
    /**
       * @brief 从映射中移除指定的键及其对应的值。
       *
       * @param key 要移除的键。
       */
    void RemoveEntry(const Key &key) {

      std::lock_guard<std::mutex> lock(map_mutex);// 加锁以保护对map的访问 
      map.erase(key);// 移除指定的键及其对应的值 
    }

  };

} // namespace traffic_manager
} // namespace carla
