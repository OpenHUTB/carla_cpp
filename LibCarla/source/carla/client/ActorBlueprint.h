// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Debug.h"
#include "carla/Iterator.h"
#include "carla/client/ActorAttribute.h"
#include "carla/rpc/ActorDefinition.h"
#include "carla/rpc/ActorDescription.h"

#include <exception>
#include <unordered_map>
#include <unordered_set>

namespace carla {
namespace client {

  /// 包含生成参与者所需的所有必要信息。
  class ActorBlueprint {
  public:

    // =========================================================================
    /// @name 构造函数
    // =========================================================================
    /// @{

    explicit ActorBlueprint(rpc::ActorDefinition actor_definition);

    /// @}
    // =========================================================================
    /// @name Id
    // =========================================================================
    /// @{

  public:

    const std::string &GetId() const {
      return _id;
    }

    /// @}
    // =========================================================================
    /// @name 标签
    // =========================================================================
    /// @{

  public:

    bool ContainsTag(const std::string &tag) const {
      return _tags.find(tag) != _tags.end();
    }

    /// 测试是否有标签满足 @a wildcard_pattern。
    ///
    /// @a wildcard_pattern 遵循 Unix shell 风格的通配符。
    bool MatchTags(const std::string &wildcard_pattern) const;

    std::vector<std::string> GetTags() const {
      return {_tags.begin(), _tags.end()};
    }

    /// @}
    // =========================================================================
    /// @name 属性
    // =========================================================================
    /// @{

  public:

    bool ContainsAttribute(const std::string &id) const {
      return _attributes.find(id) != _attributes.end();
    }

    /// @throw 如果不存在这种元素，则抛出 std::out_of_range 异常。
    const ActorAttribute &GetAttribute(const std::string &id) const;

    /// Set the value of the attribute given by @a id.
    ///
    /// @throw 如果不存在这样的元素，则抛出 std::out_of_range 异常。
    /// @throw 如果属性不可修改，则抛出 InvalidAttributeValue 异常。
    /// @throw 如果格式不匹配属性类型，则抛出 InvalidAttributeValue 异常。
    void SetAttribute(const std::string &id, std::string value);

    size_t size() const {
      return _attributes.size();
    }

    auto begin() const {
      return iterator::make_map_values_const_iterator(_attributes.begin());
    }

    auto end() const {
      return iterator::make_map_values_const_iterator(_attributes.end());
    }

    /// @}
    // =========================================================================
    /// @name 参与者描述 ActorDescription
    // =========================================================================
    /// @{

  public:

    rpc::ActorDescription MakeActorDescription() const;

    /// @}

  private:

    uint32_t _uid = 0u;

    std::string _id;

    std::unordered_set<std::string> _tags;

    std::unordered_map<std::string, ActorAttribute> _attributes;
  };

} // namespace client
} // namespace carla
