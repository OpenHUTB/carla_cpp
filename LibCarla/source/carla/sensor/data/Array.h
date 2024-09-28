// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once // 防止头文件被多次包含

#include "carla/Debug.h" // 引入调试相关的头文件
#include "carla/Exception.h" // 引入异常处理相关的头文件
#include "carla/sensor/SensorData.h" // 引入传感器数据相关的头文件

#include <exception> // 引入异常处理标准库
#include <iterator> // 引入迭代器标准库
#include <type_traits> // 引入类型特性相关的库

namespace carla { // 定义命名空间 carla
namespace sensor { // 定义命名空间 sensor
namespace data { // 定义命名空间 data

  /// 所有传感器数据的基类，包含一个项目数组。
  template <typename T> // 模板定义，类型为 T
  class Array : public SensorData { // Array 类继承自 SensorData
  public:

    using value_type = T; // 定义 value_type 为 T
    using iterator = value_type *; // 定义迭代器类型为 value_type 指针
    using const_iterator = typename std::add_const<value_type>::type *; // 定义常量迭代器类型
    using reverse_iterator = std::reverse_iterator<iterator>; // 定义反向迭代器类型
    using const_reverse_iterator = std::reverse_iterator<const_iterator>; // 定义常量反向迭代器类型
    using size_type = size_t; // 定义 size_type 为 size_t
    using pointer = value_type *; // 定义指针类型为 value_type 指针
    using const_pointer = typename std::add_const<value_type>::type *; // 定义常量指针类型
    using reference = value_type &; // 定义引用类型为 value_type 引用
    using const_reference = typename std::add_const<value_type>::type &; // 定义常量引用类型

    // 返回迭代器的起始位置
    iterator begin() { 
      return reinterpret_cast<iterator>(_data.begin() + _offset); // 计算并返回开始迭代器
    }

    // 返回常量迭代器的起始位置
    const_iterator cbegin() const { 
      return reinterpret_cast<const_iterator>(_data.begin() + _offset); // 计算并返回常量开始迭代器
    }

    // 返回常量迭代器的起始位置（重载）
    const_iterator begin() const { 
      return cbegin(); // 调用 cbegin()
    }

    // 返回迭代器的结束位置
    iterator end() { 
      return reinterpret_cast<iterator>(_data.end()); // 返回结束迭代器
    }

    // 返回常量迭代器的结束位置
    const_iterator cend() const { 
      return reinterpret_cast<const_iterator>(_data.end()); // 返回常量结束迭代器
    }

    // 返回常量迭代器的结束位置（重载）
    const_iterator end() const { 
      return cend(); // 调用 cend()
    }

    // 返回反向迭代器的起始位置
    reverse_iterator rbegin() { 
      return reverse_iterator(begin()); // 返回反向开始迭代器
    }

    // 返回常量反向迭代器的起始位置
    const_reverse_iterator crbegin() const { 
      return const_reverse_iterator(cbegin()); // 返回常量反向开始迭代器
    }

    // 返回常量反向迭代器的起始位置（重载）
    const_reverse_iterator rbegin() const { 
      return crbegin(); // 调用 crbegin()
    }

    // 返回反向迭代器的结束位置
    reverse_iterator rend() { 
      return reverse_iterator(end()); // 返回反向结束迭代器
    }

    // 返回常量反向迭代器的结束位置
    const_reverse_iterator crend() const { 
      return const_reverse_iterator(cend()); // 返回常量反向结束迭代器
    }

    // 返回常量反向迭代器的结束位置（重载）
    const_reverse_iterator rend() const { 
      return crend(); // 调用 crend()
    }

    // 检查数组是否为空
    bool empty() const { 
      return begin() == end(); // 如果开始迭代器等于结束迭代器，则为空
    }

    // 返回数组大小
    size_type size() const { 
      DEBUG_ASSERT(std::distance(begin(), end()) >= 0); // 确保大小非负
      return static_cast<size_type>(std::distance(begin(), end())); // 返回元素个数
    }

    // 返回数据指针
    value_type *data() { 
      return begin(); // 返回开始迭代器作为数据指针
    }

    // 返回常量数据指针
    const value_type *data() const { 
      return begin(); // 返回开始迭代器作为常量数据指针
    }

    // 通过下标访问元素
    reference operator[](size_type pos) { 
      return data()[pos]; // 返回指定位置的元素
    }

    // 通过下标访问常量元素
    const_reference operator[](size_type pos) const { 
      return data()[pos]; // 返回指定位置的常量元素
    }

    // 安全地访问元素，抛出越界异常
    reference at(size_type pos) { 
      if (!(pos < size())) { // 检查索引是否越界
        throw_exception(std::out_of_range("Array index out of range")); // 抛出越界异常
      }
      return operator[](pos); // 返回元素
    }

    // 安全地访问常量元素，抛出越界异常
    const_reference at(size_type pos) const { 
      if (!(pos < size())) { // 检查索引是否越界
        throw_exception(std::out_of_range("Array index out of range")); // 抛出越界异常
      }
      return operator[](pos); // 返回常量元素
    }

  protected:

    // 构造函数，接受原始数据和偏移量获取函数
    template <typename FuncT>
    explicit Array(RawData &&data, FuncT get_offset) 
      : SensorData(data), // 初始化基类
        _data(std::move(data)), // 移动原始数据
        _offset(get_offset(_data)) { // 获取偏移量
      DEBUG_ASSERT(_data.size() >= _offset); // 确保偏移量不大于数据大小
      DEBUG_ASSERT((_data.size() - _offset) % sizeof(T) == 0u); // 确保数据对齐
      DEBUG_ASSERT(begin() <= end()); // 确保开始迭代器不大于结束迭代器
    }

    // 构造函数，接受偏移量和原始数据
    explicit Array(size_t offset, RawData &&data) 
      : Array(std::move(data), [offset](const RawData &) { return offset; }) {} // 调用上一个构造函数

    // 获取原始数据
    const RawData &GetRawData() const { 
      return _data; // 返回原始数据
    }

  private:

    RawData _data; // 存储原始数据

    const size_t _offset; // 存储偏移量
  };

} // namespace data
} // namespace sensor
} // namespace carla

