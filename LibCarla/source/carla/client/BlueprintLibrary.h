// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Debug.h"
#include "carla/Iterator.h"
#include "carla/Memory.h"
#include "carla/NonCopyable.h"
#include "carla/client/ActorBlueprint.h"

#include <type_traits>
#include <unordered_map>
#include <vector>

namespace carla {
namespace client {

  /// @todo 看起来像列表，但实际上是地图。我们应该评估用例并重新考虑此实现。
  class BlueprintLibrary
    : public EnableSharedFromThis<BlueprintLibrary>,
      private MovableNonCopyable {
    using map_type = std::unordered_map<std::string, ActorBlueprint>;
  public:

    // 这里我们强制使用一些 typedef 来使这个类看起来像一个列表。
    using key_type = map_type::key_type;
    using value_type = map_type::mapped_type;
    using size_type = map_type::size_type;
    using const_iterator = decltype(carla::iterator::make_map_values_const_iterator<map_type::const_iterator>(map_type::const_iterator{}));
    using const_reference = const value_type &;
    using const_pointer = const value_type *;

    explicit BlueprintLibrary(const std::vector<rpc::ActorDefinition> &blueprints);

    BlueprintLibrary(BlueprintLibrary &&) = default;
    BlueprintLibrary &operator=(BlueprintLibrary &&) = default;

    /// 过滤 id 或标签与 @a wildcard_pattern 匹配的 ActorBlueprint 列表。
    SharedPtr<BlueprintLibrary> Filter(const std::string &wildcard_pattern) const;
    SharedPtr<BlueprintLibrary> FilterByAttribute(const std::string &name, const std::string& value) const;

    const_pointer Find(const std::string &key) const;

    /// @throw 如果不存在这些元素，则抛出 std::out_of_range 异常。
    const_reference at(const std::string &key) const;

    /// @warning 线性复杂度。
    const_reference operator[](size_type pos) const {
      using diff_t = std::iterator_traits<const_iterator>::difference_type;
      return std::next(_blueprints.begin(), static_cast<diff_t>(pos))->second;
    }

    /// @warning 线性复杂度。
    /// @throw std::out_of_range if !(pos < size()).
    const_reference at(size_type pos) const;
    // 返回一个指向容器中第一个元素的常量迭代器，该迭代器只能读取值。
    const_iterator begin() const /*noexcept*/ {
      return iterator::make_map_values_const_iterator(_blueprints.begin());
    }
    // 返回一个指向容器末尾的常量迭代器，该迭代器只能读取值。
    const_iterator end() const /*noexcept*/ {
      return iterator::make_map_values_const_iterator(_blueprints.end());
    }
    // 检查容器是否为空，如果没有任何元素则返回true。
    bool empty() const /*noexcept*/ {
      return _blueprints.empty();
    }
    // 返回容器中元素的数量。
    size_type size() const /*noexcept*/ {
      return _blueprints.size();
    }

  private:

    BlueprintLibrary(map_type blueprints)
      : _blueprints(std::move(blueprints)) {}

    map_type _blueprints;
  };

} // namespace client
} // namespace carla
