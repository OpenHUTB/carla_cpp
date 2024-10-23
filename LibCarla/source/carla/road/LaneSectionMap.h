// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/NonCopyable.h" // 引入不可复制的基类
#include "carla/road/LaneSection.h" // 引入车道段的头文件

#include <map>
#include <unordered_map>

namespace carla {
namespace road {

  // LaneSectionMap 类用于管理车道段的映射
  class LaneSectionMap
    : private std::multimap<double, LaneSection>, // 继承自多重映射，键为double类型（位置），值为LaneSection对象
      private MovableNonCopyable { // 继承自不可复制的可移动类
    using Super = std::multimap<double, LaneSection>; // 定义父类类型别名
  public:

    // 添加新的LaneSection并返回其引用
    LaneSection &Emplace(SectionId id, double s) {
      // 在多重映射中插入LaneSection，并获取其引用
      LaneSection &result = Super::emplace(s, LaneSection{id, s})->second;
      // 同时在_id映射中记录该LaneSection的指针
      _by_id.emplace(result.GetId(), &result);
      return result; // 返回LaneSection的引用
    }

    // 根据ID获取LaneSection的引用
    LaneSection &GetById(SectionId id) {
      return *_by_id.at(id); // 返回对应ID的LaneSection的引用
    }

    // 常量版本，根据ID获取LaneSection的引用
    const LaneSection &GetById(SectionId id) const {
      return *_by_id.at(id); // 返回对应ID的LaneSection的引用
    }

    // 公开父类中的一些方法
    using Super::find; // 查找功能
    using Super::upper_bound; // 获取大于给定值的第一个元素
    using Super::lower_bound; // 获取大于等于给定值的第一个元素

    // 迭代器相关功能
    using Super::begin; // 返回迭代器到首元素
    using Super::rbegin; // 返回反向迭代器到末尾元素
    using Super::end; // 返回迭代器到尾后元素
    using Super::rend; // 返回反向迭代器到首前元素

  private:
    // 通过ID快速查找LaneSection的哈希映射
    std::unordered_map<SectionId, LaneSection *> _by_id;
  };

} // namespace road
} // namespace carla