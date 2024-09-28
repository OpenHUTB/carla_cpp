// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/NonCopyable.h"
#include "carla/rpc/Actor.h"

#include <boost/iterator/transform_iterator.hpp>

#include <algorithm>
#include <iterator>
#include <mutex>

namespace carla {
namespace client {
namespace detail {

  // ===========================================================================
  // -- 在客户端缓冲的参与者列表 CachedActorList ---------------------------------
  // ===========================================================================

  /// 保留参与者描述列表，以避免每次都向服务器请求描述。
  ///
  /// @todo Dead actors are never removed from the list.
  class CachedActorList : private MovableNonCopyable {
  public:

    /// 将参与者插入到列表中。
    void Insert(rpc::Actor actor);

    /// 插入包含参与者的 @a 范围。
    template <typename RangeT>
    void InsertRange(RangeT range);

    /// 返回 @a 范围内尚未添加到此列表的参与者 ID。
    template <typename RangeT>
    std::vector<ActorId> GetMissingIds(const RangeT &range) const;

    /// 检索与 @a id 匹配的参与者，如果参与者未被缓存，则为空（可选）。
    boost::optional<rpc::Actor> GetActorById(ActorId id) const;

    /// 检索与 @a 范围内的 ID 匹配的参与者。
    template <typename RangeT>
    std::vector<rpc::Actor> GetActorsById(const RangeT &range) const;

    void Clear();

  private:

    mutable std::mutex _mutex;

    std::unordered_map<ActorId, rpc::Actor> _actors;
  };

  // ===========================================================================
  // -- 缓冲的参与者列表 CachedActorList implementation 实现 ---------------------
  // ===========================================================================

  inline void CachedActorList::Insert(rpc::Actor actor) {
    std::lock_guard<std::mutex> lock(_mutex);
    auto id = actor.id;
    _actors.emplace(id, std::move(actor));
  }

  template <typename RangeT>
  inline void CachedActorList::InsertRange(RangeT range) {
    auto make_a_pair = [](rpc::Actor actor) {
      auto id = actor.id;
      return std::make_pair(id, std::move(actor));
    };
    auto make_iterator = [&make_a_pair](auto it) {
      return boost::make_transform_iterator(std::make_move_iterator(it), make_a_pair);
    };
    std::lock_guard<std::mutex> lock(_mutex);
    _actors.insert(make_iterator(std::begin(range)), make_iterator(std::end(range)));
  }

  template <typename RangeT>
  inline std::vector<ActorId> CachedActorList::GetMissingIds(const RangeT &range) const {
    std::vector<ActorId> result;
    result.reserve(range.size());
    std::lock_guard<std::mutex> lock(_mutex);
    std::copy_if(std::begin(range), std::end(range), std::back_inserter(result), [this](auto id) {
      return _actors.find(id) == _actors.end();
    });
    return result;
  }

  inline boost::optional<rpc::Actor> CachedActorList::GetActorById(ActorId id) const {
    std::lock_guard<std::mutex> lock(_mutex);
    auto it = _actors.find(id);
    if (it != _actors.end()) {
      return it->second;
    }
    return boost::none;
  }

  template <typename RangeT>
  inline std::vector<rpc::Actor> CachedActorList::GetActorsById(const RangeT &range) const {
    std::vector<rpc::Actor> result;
    result.reserve(range.size());
    std::lock_guard<std::mutex> lock(_mutex);
    for (auto &&id : range) {
      auto it = _actors.find(id);
      if (it != _actors.end()) {
        result.emplace_back(it->second);
      }
    }
    return result;
  }

  inline void CachedActorList::Clear() {
    std::lock_guard<std::mutex> lock(_mutex);
    _actors.clear();
  }

} // namespace detail
} // namespace client
} // namespace carla
