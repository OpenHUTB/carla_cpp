// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.
// 引入必要的头文件
#include "carla/client/detail/EpisodeState.h"

namespace carla {
namespace client {
namespace detail {

// EpisodeState类的构造函数，用于初始化一个EpisodeState对象
  // 参数：state - 一个const引用，指向sensor::data::RawEpisodeState类型的数据，包含了当前模拟场景的状态信息
  EpisodeState::EpisodeState(const sensor::data::RawEpisodeState &state)
 // 使用传入的RawEpisodeState对象中的数据来初始化EpisodeState对象的成员变量
    : _episode_id(state.GetEpisodeId()),// 初始化_episode_id，表示当前模拟场景的ID
      _timestamp(// 初始化_timestamp，包含帧信息、游戏时间戳、时间差、平台时间戳
          state.GetFrame(),
          state.GetGameTimeStamp(),
          state.GetDeltaSeconds(),
          state.GetPlatformTimeStamp()),
      _map_origin(state.GetMapOrigin()),// 初始化_map_origin，表示地图的原点
      _simulation_state(state.GetSimulationState()) {// 初始化_simulation_state，表示当前的模拟状态
    // 预留空间以存储所有的Actor快照
    _actors.reserve(state.size());
// 遍历RawEpisodeState中的所有Actor，并为每个Actor创建一个ActorSnapshot对象
    for (auto &&actor : state) {
 // 使用emplace方法将ActorSnapshot对象插入到_actors映射中
      // 键是Actor的ID，值是ActorSnapshot对象
      // DEBUG_ONLY(auto result = ) 这部分代码用于调试，用于检查插入操作是否成功
      DEBUG_ONLY(auto result = )
      _actors.emplace(
          actor.id,
          ActorSnapshot{// 初始化ActorSnapshot对象
              actor.id,         // Actor的ID
              actor.actor_state, // Actor的状态
              actor.transform,// Actor的变换（位置和方向）
              actor.velocity,// Actor的速度
              actor.angular_velocity,// Actor的角速度
              actor.acceleration,// Actor的加速度
              actor.state});// Actor的附加状态信息
      DEBUG_ASSERT(result.second);
 // DEBUG_ASSERT(result.second); 这部分代码用于调试，确保emplace操作成功，即没有重复键
    }
  }

} // namespace detail
} // namespace client
} // namespace carla
