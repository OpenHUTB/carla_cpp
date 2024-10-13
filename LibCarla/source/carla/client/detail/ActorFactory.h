// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Memory.h"
#include "carla/client/GarbageCollectionPolicy.h"
#include "carla/client/detail/EpisodeProxy.h"
#include "carla/rpc/Actor.h"

namespace carla {
namespace client {

  class Actor;

namespace detail {

  class ActorFactory {
  public:

    /// 基于提供的 @a actor_description 创建一个参与者。@a episode
    /// 必须指向该参与者所在的章节（或者说区域）
    ///
    /// 不要直接调用这个类，请使用 Simulator::MakeActor。
    ///
    /// 如果 @a garbage_collection_policy 是 GarbageCollectionPolicy::Enabled，那么
    /// 返回的共享指针将提供一个自定义删除器，该删除器调用
    /// actor 上的 Destroy() 方法
    static SharedPtr<Actor> MakeActor(
        EpisodeProxy episode,
        rpc::Actor actor_description,
        GarbageCollectionPolicy garbage_collection_policy);
  };

} // namespace detail
} // namespace client
} // namespace carla
