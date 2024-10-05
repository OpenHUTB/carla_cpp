// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/NonCopyable.h" // 引入非拷贝类的头文件
#include "carla/road/RoadElementSet.h" // 引入道路元素集合的头文件
#include "carla/road/element/RoadInfo.h" // 引入道路信息元素的头文件
#include "carla/road/element/RoadInfoIterator.h" // 引入道路信息迭代器的头文件

#include <vector> // 引入向量的头文件
#include <memory> // 引入智能指针的头文件

namespace carla { // carla命名空间
namespace road { // road命名空间

  class InformationSet : private MovableNonCopyable { // 信息集合类，继承自不可拷贝类
  public:

    InformationSet() = default; // 默认构造函数

    InformationSet(std::vector<std::unique_ptr<element::RoadInfo>> &&vec) // 接受右值向量构造函数
      : _road_set(std::move(vec)) {} // 移动初始化道路元素集合

    /// 返回从道路起点给定类型的所有信息
    template <typename T>
    std::vector<const T *> GetInfos() const { // 模板函数，获取指定类型的信息
      std::vector<const T *> vec; // 创建一个存储指针的向量
      auto it = element::MakeRoadInfoIterator<T>(_road_set.GetAll()); // 创建道路信息迭代器
      for (; !it.IsAtEnd(); ++it) { // 遍历迭代器
        vec.emplace_back(&*it); // 添加迭代器当前指向的元素指针到向量
      }
      return vec; // 返回信息向量
    }

    /// 返回给定类型和距离s的单一信息
    template <typename T>
    const T *GetInfo(const double s) const { // 模板函数，获取指定距离的信息
      auto it = element::MakeRoadInfoIterator<T>(_road_set.GetReverseSubset(s)); // 创建反向迭代器
      return it.IsAtEnd() ? nullptr : &*it; // 如果迭代器到达末尾，返回nullptr，否则返回当前元素的指针
    }

    /// 返回在指定范围内给定类型的所有信息
    template <typename T>
    std::vector<const T *> GetInfos(const double min_s, const double max_s) const { // 模板函数，获取指定范围的信息
      std::vector<const T *> vec; // 创建一个存储指针的向量
      if(min_s < max_s) { // 如果最小值小于最大值
        auto it = element::MakeRoadInfoIterator<T>(
            _road_set.GetSubsetInRange(min_s, max_s)); // 创建范围内迭代器
        for (; !it.IsAtEnd(); ++it) { // 遍历迭代器
          vec.emplace_back(&*it); // 添加当前元素指针到向量
        }
      } else { // 如果最小值大于等于最大值
        auto it = element::MakeRoadInfoIterator<T>(
            _road_set.GetReverseSubsetInRange(max_s, min_s)); // 创建反向范围迭代器
        for (; !it.IsAtEnd(); ++it) { // 遍历迭代器
          vec.emplace_back(&*it); // 添加当前元素指针到向量
        }
      }
      return vec; // 返回信息向量
    }

  private:

    RoadElementSet<std::unique_ptr<element::RoadInfo>> _road_set; // 私有成员，存储道路元素集合
  };

} // road
} // carla