// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once// 确保头文件只被包含一次

#include "carla/Debug.h"// 包含CARLA调试工具
#include "carla/Iterator.h"// 包含CARLA迭代器工具
#include "carla/client/ActorAttribute.h"// 包含CARLA客户端参与者属性
#include "carla/rpc/ActorDefinition.h"// 包含CARLA远程过程调用参与者定义
#include "carla/rpc/ActorDescription.h"// 包含CARLA远程过程调用参与者描述

#include <exception>// 包含C++标准异常库
#include <unordered_map>// 包含C++标准无序映射库
#include <unordered_set>// 包含C++标准无序集合库

namespace carla {
namespace client {

  /// 包含生成参与者所需的所有必要信息。
  class ActorBlueprint {
  public:

    // =========================================================================
    /// @name 构造函数
    // =========================================================================
    /// @{

    explicit ActorBlueprint(rpc::ActorDefinition actor_definition); // 构造函数，接受参与者定义

    /// @}
    // =========================================================================
    /// @name Id
    // =========================================================================
    /// @{

  public:

    const std::string &GetId() const {// 获取参与者ID
      return _id;
    }

    /// @}
    // =========================================================================
    /// @name 标签
    // =========================================================================
    /// @{

  public:

    bool ContainsTag(const std::string &tag) const {// 检查是否包含特定标签
      return _tags.find(tag) != _tags.end();
    }

    /// 测试是否有标签满足 @a wildcard_pattern。
    ///
    /// @a wildcard_pattern 遵循 Unix shell 风格的通配符。
    bool MatchTags(const std::string &wildcard_pattern) const;// 检查是否有标签匹配通配符模式

    std::vector<std::string> GetTags() const {
      return {_tags.begin(), _tags.end()};
    }

    /// @}
    // =========================================================================
    /// @name 属性
    // =========================================================================
    /// @{

  public:

    bool ContainsAttribute(const std::string &id) const {// 检查是否包含特定属性
      return _attributes.find(id) != _attributes.end();
    }

    /// @throw 如果不存在这种元素，则抛出 std::out_of_range 异常。
    const ActorAttribute &GetAttribute(const std::string &id) const;// 获取参与者属性

    /// Set the value of the attribute given by @a id.
    ///
    /// @throw 如果不存在这样的元素，则抛出 std::out_of_range 异常。
    /// @throw 如果属性不可修改，则抛出 InvalidAttributeValue 异常。
    /// @throw 如果格式不匹配属性类型，则抛出 InvalidAttributeValue 异常。
    void SetAttribute(const std::string &id, std::string value);// 设置参与者属性值

    size_t size() const {// 获取属性数量
      return _attributes.size();
    }

    auto begin() const {// 获取属性迭代器开始
      return iterator::make_map_values_const_iterator(_attributes.begin());
    }

    auto end() const {// 获取属性迭代器结束
      return iterator::make_map_values_const_iterator(_attributes.end());
    }

    /// @}
    // =========================================================================
    /// @name 参与者描述 ActorDescription
    // =========================================================================
    /// @{

  public:

    rpc::ActorDescription MakeActorDescription() const;// 创建参与者描述

    /// @}

  private:

    uint32_t _uid = 0u;// 参与者唯一ID

    std::string _id;// 参与者ID

    std::unordered_set<std::string> _tags;// 参与者标签集合

    std::unordered_map<std::string, ActorAttribute> _attributes;// 参与者属性映射
  };

} // namespace client
} // namespace carla
