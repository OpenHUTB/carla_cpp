// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/client/Timestamp.h"
#include "carla/client/ActorSnapshot.h"
#include "carla/client/detail/EpisodeState.h"

#include <boost/optional.hpp>

namespace carla {
namespace client {
    // 定义一个名为 WorldSnapshot 的类
  class WorldSnapshot {
  public:
    // 构造函数，接收一个 std::shared_ptr<const detail::EpisodeState> 类型的参数 state，并将其移动到 _state 成员变
    WorldSnapshot(std::shared_ptr<const detail::EpisodeState> state)
      : _state(std::move(state)) {}

	// 获取与这个世界相关联的剧集的 ID
    uint64_t GetId() const {
      return _state->GetEpisodeId();
    }
    // 获取这个快照的帧编号
    size_t GetFrame() const {
      return GetTimestamp().frame;
    }

	// 获取这个快照的时间戳
    const Timestamp &GetTimestamp() const {
      return _state->GetTimestamp();
    }

	// 检查一个参与者是否在这个快照中存在
    bool Contains(ActorId actor_id) const {
      return _state->ContainsActorSnapshot(actor_id);
    }

	// 通过 ID 查找一个参与者的快照，如果找到则返回 boost::optional<ActorSnapshot>，否则返回 boost::none
    boost::optional<ActorSnapshot> Find(ActorId actor_id) const {
      return _state->GetActorSnapshotIfPresent(actor_id);
    }

	// 返回这个世界快照中参与者快照的数量
    size_t size() const {
      return _state->size();
    }

	// 返回指向参与者快照列表的开始迭代器
    auto begin() const {
      return _state->begin();
    }

	// 返回指向参与者快照列表的结束迭代器
    auto end() const {
      return _state->end();
    }
    // 重载等于运算符，判断两个 WorldSnapshot 对象是否相等，仅当时间戳相等时返回 true
    bool operator==(const WorldSnapshot &rhs) const {
      return GetTimestamp() == rhs.GetTimestamp();
    }
    // 重载不等于运算符，当两个 WorldSnapshot 对象不相等时返回 true
    bool operator!=(const WorldSnapshot &rhs) const {
      return !(*this == rhs);
    }

  private:

    std::shared_ptr<const detail::EpisodeState> _state;
  };

} // namespace client
} // namespace carla
