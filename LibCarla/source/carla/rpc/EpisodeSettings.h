// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/MsgPack.h"
#include "carla/MsgPackAdaptors.h"

#ifdef LIBCARLA_INCLUDED_FROM_UE4
#include <compiler/enable-ue4-macros.h>
#include "Carla/Settings/EpisodeSettings.h"
#include <compiler/disable-ue4-macros.h>
#endif // LIBCARLA_INCLUDED_FROM_UE4

#include <boost/optional.hpp>

namespace carla {
namespace rpc {

  class EpisodeSettings {
  public:
    // =========================================================================
    // -- 公开的数据成员 --------------------------------------------------------
    // =========================================================================

    // 是否启用同步模式
    bool synchronous_mode = false;

    // 是否启用无渲染模式
    bool no_rendering_mode = false;

    // 可选的固定时间间隔，使用 boost::optional 来表示该值可能存在也可能不存在
    boost::optional<double> fixed_delta_seconds;

    // 是否启用子步，默认为 true
    bool substepping = true; 

    // 两个物理子步之间的最大时间
    double max_substep_delta_time = 0.01;  

    // 一个仿真帧包含的最多物理子步数
    int max_substeps = 10;  

    // 距离剔除：当对象尺寸足够小，距离镜头足够远时，可以不进行渲染，以提高性能
    // https://www.jianshu.com/p/537d297bedfa
    float max_culling_distance = 0.0f;

    // 确定性布娃娃系统，默认为 true
    bool deterministic_ragdolls = true;

    // 该关卡中 距离玩家 3km 内的地图瓦片会被加载
    float tile_stream_distance = 3000.f; // 3km

    // 距离玩家 2km 内的参与者会被激活
    float actor_active_distance = 2000.f; // 2km

    // 是否将观众视为自我，默认为 true
    bool spectator_as_ego = true;

    // 使用 MSGPACK_DEFINE_ARRAY 宏将类的成员变量按顺序打包到 msgpack 中，用于序列化
    MSGPACK_DEFINE_ARRAY(synchronous_mode, no_rendering_mode, fixed_delta_seconds, substepping,
        max_substep_delta_time, max_substeps, max_culling_distance, deterministic_ragdolls,
        tile_stream_distance, actor_active_distance, spectator_as_ego);

    // =========================================================================
    // -- 构造函数 --------------------------------------------------------------
    // =========================================================================

    // 默认构造函数，使用编译器生成的默认实现
    EpisodeSettings() = default;

    // 带有参数的构造函数，可以指定各项设置
    EpisodeSettings(
        bool synchronous_mode,
        bool no_rendering_mode,
        double fixed_delta_seconds = 0.0,
        bool substepping = true,
        double max_substep_delta_time = 0.01,
        int max_substeps = 10,
        float max_culling_distance = 0.0f,
        bool deterministic_ragdolls = true,
        float tile_stream_distance = 3000.f,
        float actor_active_distance = 2000.f,
        bool spectator_as_ego = true)
      : synchronous_mode(synchronous_mode),
        no_rendering_mode(no_rendering_mode),
        fixed_delta_seconds(
            fixed_delta_seconds > 0.0? fixed_delta_seconds : boost::optional<double>{}),
        substepping(substepping),
        max_substep_delta_time(max_substep_delta_time),
        max_substeps(max_substeps),
        max_culling_distance(max_culling_distance),
        deterministic_ragdolls(deterministic_ragdolls),
        tile_stream_distance(tile_stream_distance),
        actor_active_distance(actor_active_distance),
        spectator_as_ego(spectator_as_ego) {}

    // =========================================================================
    // -- 比较操作符 ------------------------------------------------------------
    // =========================================================================

    // 重载 == 操作符，用于比较两个 EpisodeSettings 对象是否相等
    bool operator==(const EpisodeSettings &rhs) const {
      return
          (synchronous_mode == rhs.synchronous_mode) &&
          (no_rendering_mode == rhs.no_rendering_mode) &&
          (substepping == rhs.substepping) &&
          (fixed_delta_seconds == rhs.fixed_delta_seconds) &&
          (max_substep_delta_time == rhs.max_substep_delta_time) &&
          (max_substeps == rhs.max_substeps) &&
          (max_culling_distance == rhs.max_culling_distance) &&
          (deterministic_ragdolls == rhs.deterministic_ragdolls) &&
          (tile_stream_distance == rhs.tile_stream_distance) &&
          (actor_active_distance == rhs.actor_active_distance) &&
          (spectator_as_ego == rhs.spectator_as_ego);
    }

    // 重载!= 操作符，使用 == 操作符的结果取反
    bool operator!=(const EpisodeSettings &rhs) const {
      return!(*this == rhs);
    }

    // =========================================================================
    // -- 转换成虚幻4类型 -------------------------------------------------------
    // =========================================================================

#ifdef LIBCARLA_INCLUDED_FROM_UE4
    // 从虚幻4的 FEpisodeSettings 类型构造 EpisodeSettings
    EpisodeSettings(const FEpisodeSettings &Settings)
      : EpisodeSettings(
            Settings.bSynchronousMode,
            Settings.bNoRenderingMode,
            Settings.FixedDeltaSeconds.Get(0.0),
            Settings.bSubstepping,
            Settings.MaxSubstepDeltaTime,
            Settings.MaxSubsteps,
            Settings.MaxCullingDistance,
            Settings.bDeterministicRagdolls,
            Settings.TileStreamingDistance,
            Settings.ActorActiveDistance,
            Settings.SpectatorAsEgo) {
      constexpr float CMTOM = 1.f/100.f;
      tile_stream_distance = CMTOM * Settings.TileStreamingDistance;
      actor_active_distance = CMTOM * Settings.ActorActiveDistance;
    }

    // 类型转换运算符，将 EpisodeSettings 转换为虚幻4的 FEpisodeSettings 类型
    operator FEpisodeSettings() const {
      constexpr float MTOCM = 100.f;
      FEpisodeSettings Settings;
      Settings.bSynchronousMode = synchronous_mode;
      Settings.bNoRenderingMode = no_rendering_mode;
      if (fixed_delta_seconds.has_value()) {
        Settings.FixedDeltaSeconds = *fixed_delta_seconds;
      }
      Settings.bSubstepping = substepping;
      Settings.MaxSubstepDeltaTime = max_substep_delta_time;
      Settings.MaxSubsteps = max_substeps;
      Settings.MaxCullingDistance = max_culling_distance;
      Settings.bDeterministicRagdolls = deterministic_ragdolls;
      Settings.TileStreamingDistance = MTOCM * tile_stream_distance;
      Settings.ActorActiveDistance = MTOCM * actor_active_distance;
      Settings.SpectatorAsEgo = spectator_as_ego;

      return Settings;
    }
#endif // LIBCARLA_INCLUDED_FROM_UE4
  };

} // namespace rpc
} // namespace carla
