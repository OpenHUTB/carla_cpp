// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
#include "carla/ListView.h"  // 包含 ListView 头文件，可能用于处理列表视图。
#include "carla/NonCopyable.h" // 包含不可拷贝类的定义。

#include <iterator>  // 包含迭代器相关的标准库。
#include <memory>    // 包含智能指针相关的标准库。
#include <vector>    // 包含动态数组（向量）的标准库。
#include <algorithm> // 包含算法（如排序、查找等）的标准库。
#include <type_traits> // 包含类型特征的标准库。

namespace carla { // 定义命名空间 carla
namespace road {  // 定义命名空间 road

  /// 表示按道路位置排序的元素集合。
  template <typename T>
  class RoadElementSet : private MovableNonCopyable { // 模板类 RoadElementSet，继承不可拷贝的类
  public:

    using mapped_type = T; // 映射类型为模板参数 T

    using key_type = double; // 键类型为 double

    RoadElementSet() = default; // 默认构造函数

    /// 显式移动构造函数。
    template <typename InputTypeT>
    RoadElementSet(std::vector<InputTypeT> &&range)
      : _vec([](auto &&input) { // 初始化 _vec，使用 lambda 表达式
          static_assert(!std::is_const<InputTypeT>::value, "Input type cannot be const"); // 检查输入类型不能是常量
          std::sort(std::begin(input), std::end(input), LessComp()); // 按照 LessComp 排序
          return decltype(_vec){ // 返回 _vec 的移动迭代器
              std::make_move_iterator(std::begin(input)),
              std::make_move_iterator(std::end(input))};
        }(std::move(range))) {} // 移动输入范围并初始化

    /// 返回集合中的所有值。
    const std::vector<mapped_type> &GetAll() const { // 返回 _vec 的常量引用
      return _vec; // 返回所有元素
    }

    /// 返回键值小于等于 s 的元素的逆序列表。
    auto GetReverseSubset(const key_type k) const { // 以 k 为界获取逆序子集
      return MakeListView(
          std::make_reverse_iterator(std::upper_bound(_vec.begin(), _vec.end(), k, LessComp())), // 找到大于 k 的第一个元素并反转
          _vec.rend()); // 返回逆序迭代器的范围
    }

    /// 返回键值在 [min_k, max_k] 范围内的元素列表。
    auto GetSubsetInRange(const key_type min_k, const key_type max_k) const { // 获取范围内的子集
      auto low_bound = (std::lower_bound(_vec.begin(), _vec.end(), min_k, LessComp())); // 找到大于等于 min_k 的下界
      auto up_bound = (std::upper_bound(_vec.begin(), _vec.end(), max_k, LessComp())); // 找到大于 max_k 的上界
      return MakeListView((low_bound), (up_bound)); // 返回该范围内的视图
    }

    /// 返回键值在 [min_k, max_k] 范围内的元素的逆序列表。
    auto GetReverseSubsetInRange(const key_type min_k, const key_type max_k) const { // 获取范围内的逆序子集
      auto low_bound = (std::lower_bound(_vec.begin(), _vec.end(), min_k, LessComp())); // 找到大于等于 min_k 的下界
      auto up_bound = (std::upper_bound(low_bound, _vec.end(), max_k, LessComp())); // 找到大于 max_k 的上界
      return MakeListView(std::make_reverse_iterator(up_bound), std::make_reverse_iterator(low_bound)); // 返回逆序视图
    }

    bool empty() const { // 检查集合是否为空
      return _vec.empty(); // 返回 _vec 是否为空
    }

    size_t size() const { // 获取集合大小
      return _vec.size(); // 返回 _vec 的大小
    }

    auto begin() const { // 返回开始迭代器
      return _vec.begin(); // 返回 _vec 的开始迭代器
    }

    auto end() const { // 返回结束迭代器
      return _vec.end(); // 返回 _vec 的结束迭代器
    }

  private:

    static key_type GetDistance(const key_type key) { // 获取键值的距离
      return key; // 直接返回键值
    }

    template <typename ValueT>
    static key_type GetDistance(const ValueT &value) { // 获取元素的距离（值类型）
      return value.GetDistance(); // 调用元素的 GetDistance 方法
    }

    template <typename ValueT>
    static key_type GetDistance(const ValueT *value) { // 获取元素的距离（指针类型）
      return value->GetDistance(); // 调用指针指向元素的 GetDistance 方法
    }

    template <typename ValueT>
    static key_type GetDistance(const std::unique_ptr<ValueT> &value) { // 获取元素的距离（智能指针类型）
      return value->GetDistance(); // 调用智能指针指向元素的 GetDistance 方法
    }

    struct LessComp { // 定义一个比较结构体
      using is_transparent = void; // 使比较器支持透明比较
      template <typename LhsT, typename RhsT>
      bool operator()( // 重载比较运算符
          const LhsT &a,
          const RhsT &b) const {
        return GetDistance(a) < GetDistance(b); // 使用 GetDistance 比较两个值
      }
    };

    std::vector<mapped_type> _vec; // 存储元素的向量
  };

} // namespace road
} // namespace carla