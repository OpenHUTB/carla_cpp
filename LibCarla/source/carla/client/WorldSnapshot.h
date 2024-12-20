// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once


#include "carla/client/Timestamp.h" // 引入时间戳相关的头文件
#include "carla/client/ActorSnapshot.h" // 引入参与者快照相关的头文件
#include "carla/client/detail/EpisodeState.h" // 引入剧集状态相关的头文件

#include <boost/optional.hpp> // 引入 Boost 库中的 optional，用于表示可能为空的值

namespace carla {
namespace client {
  // WorldSnapshot 类表示一个世界快照，它包含了与世界相关的所有状态信息
  class WorldSnapshot {
  public:
    // 构造函数，接收一个指向 EpisodeState 的共享指针，并将其存储在 _state 成员变量中
    WorldSnapshot(std::shared_ptr<const detail::EpisodeState> state)
      : _state(std::move(state)) {}

    // 获取与该世界快照关联的剧集 ID
    uint64_t GetId() const {
      return _state->GetEpisodeId();
    }
    // 获取当前世界快照的帧编号
    size_t GetFrame() const {
      return GetTimestamp().frame;
    }

    // 获取世界快照的时间戳
    const Timestamp &GetTimestamp() const {
      return _state->GetTimestamp();
    }

    // 检查指定的 Actor 是否在当前世界快照中
    bool Contains(ActorId actor_id) const {
      return _state->ContainsActorSnapshot(actor_id);
    }

    // 根据 ActorId 查找相应的 Actor 快照，如果找到了则返回 ActorSnapshot，否则返回 boost::none
    boost::optional<ActorSnapshot> Find(ActorId actor_id) const {
      return _state->GetActorSnapshotIfPresent(actor_id);
    }

    // 获取当前世界快照中参与者快照的数量
    size_t size() const {
      return _state->size();
    }

    // 获取指向世界快照中所有参与者快照列表的开始迭代器
    auto begin() const {
      return _state->begin();
    }

    // 获取指向世界快照中所有参与者快照列表的结束迭代器
    auto end() const {
      return _state->end();
    }

    // 重载等于运算符，比较两个 WorldSnapshot 对象是否相等
    // 只有在时间戳相同的情况下，两个快照才视为相等
    bool operator==(const WorldSnapshot &rhs) const {
      return GetTimestamp() == rhs.GetTimestamp();
    }
    // 重载不等于运算符，判断两个 WorldSnapshot 对象是否不相等
    bool operator!=(const WorldSnapshot &rhs) const {
      return !(*this == rhs);
    }

  private:
<<<<<<< HEAD

    std::shared_ptr<const detail::EpisodeState> _state;
=======
    // 存储与该世界快照相关的剧集状态
    std::shared_ptr<const detail::EpisodeState> _state;  
>>>>>>> f89b365fd9915e0aff4920d1d781eebc897dad7d
  };

} // namespace client
} // namespace carla
