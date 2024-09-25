// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once // 防止头文件被重复包含

#include "carla/Iterator.h" // 引入迭代器头文件
#include "carla/ListView.h" // 引入列表视图头文件
#include "carla/NonCopyable.h" // 引入不可复制类的头文件
#include "carla/client/ActorSnapshot.h" // 引入参与者快照头文件
#include "carla/client/Timestamp.h" // 引入时间戳头文件
#include "carla/geom/Vector3DInt.h" // 引入三维整数向量头文件
#include "carla/sensor/data/RawEpisodeState.h" // 引入原始剧集状态数据头文件

#include <boost/optional.hpp> // 引入Boost可选类型头文件

#include <memory> // 引入智能指针头文件
#include <unordered_map> // 引入无序映射头文件

namespace carla { // 定义carla命名空间
namespace client { // 定义client子命名空间
namespace detail { // 定义detail子命名空间

  /// 表示某一帧的所有参与者的状态
  class EpisodeState
    : public std::enable_shared_from_this<EpisodeState>, // 允许共享自身指针
      private NonCopyable { // 禁止复制

      using SimulationState = sensor::s11n::EpisodeStateSerializer::SimulationState; // 定义模拟状态类型

  public:

    // 构造函数，接受剧集ID
    explicit EpisodeState(uint64_t episode_id) : _episode_id(episode_id) {}

    // 构造函数，接受原始剧集状态
    explicit EpisodeState(const sensor::data::RawEpisodeState &state);

    // 获取剧集ID
    auto GetEpisodeId() const {
      return _episode_id;
    }

    // 获取当前帧数
    auto GetFrame() const {
      return _timestamp.frame;
    }

    // 获取时间戳
    const auto &GetTimestamp() const {
      return _timestamp;
    }

    // 获取模拟状态
    SimulationState GetsimulationState() const {
      return _simulation_state;
    }

    // 检查地图是否发生变化
    bool HasMapChanged() const {
      return (_simulation_state & SimulationState::MapChange) != SimulationState::None;
    }

    // 检查光照更新是否待处理
    bool IsLightUpdatePending() const {
      return (_simulation_state & SimulationState::PendingLightUpdate)  != 0;
    }

    // 检查是否包含指定的参与者快照
    bool ContainsActorSnapshot(ActorId actor_id) const {
      return _actors.find(actor_id) != _actors.end();
    }

    // 获取指定参与者的快照
    ActorSnapshot GetActorSnapshot(ActorId id) const {
      ActorSnapshot state; // 创建参与者快照对象
      CopyActorSnapshotIfPresent(id, state); // 复制快照（如果存在）
      return state; // 返回快照
    }

    // 获取指定参与者的快照（如果存在）
    boost::optional<ActorSnapshot> GetActorSnapshotIfPresent(ActorId id) const {
      boost::optional<ActorSnapshot> state; // 创建可选快照
      CopyActorSnapshotIfPresent(id, state); // 复制快照（如果存在）
      return state; // 返回可选快照
    }

    // 获取所有参与者ID
    auto GetActorIds() const {
      return MakeListView( // 创建列表视图
          iterator::make_map_keys_const_iterator(_actors.begin()), // 获取参与者ID迭代器
          iterator::make_map_keys_const_iterator(_actors.end())); // 获取参与者ID迭代器
    }

    // 获取参与者数量
    size_t size() const {
      return _actors.size(); // 返回参与者数量
    }

    // 返回参与者快照的开始迭代器
    auto begin() const {
      return iterator::make_map_values_const_iterator(_actors.begin()); // 返回参与者快照值的开始迭代器
    }

    // 返回参与者快照的结束迭代器
    auto end() const {
      return iterator::make_map_values_const_iterator(_actors.end()); // 返回参与者快照值的结束迭代器
    }

  private:

    // 复制指定参与者的快照（如果存在）
    template <typename T>
    void CopyActorSnapshotIfPresent(ActorId id, T &value) const {
      auto it = _actors.find(id); // 查找参与者
      if (it != _actors.end()) { // 如果找到了
        value = it->second; // 复制快照
      }
    }

    const uint64_t _episode_id; // 存储剧集ID

    const Timestamp _timestamp; // 存储时间戳

    geom::Vector3DInt _map_origin; // 存储地图原点

    SimulationState _simulation_state; // 存储模拟状态

    std::unordered_map<ActorId, ActorSnapshot> _actors; // 存储参与者快照的无序映射
  };

} // namespace detail
} // namespace client
} // namespace carla

