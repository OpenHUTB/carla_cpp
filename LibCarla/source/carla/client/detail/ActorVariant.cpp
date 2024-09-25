// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/client/detail/ActorVariant.h" // 引入ActorVariant头文件

#include "carla/client/detail/ActorFactory.h" // 引入ActorFactory头文件
#include "carla/client/ActorList.h" // 引入ActorList头文件

namespace carla { // 定义carla命名空间
namespace client { // 定义client子命名空间
namespace detail { // 定义detail子命名空间

  void ActorVariant::MakeActor(EpisodeProxy episode) const { // 定义MakeActor方法，接受一个EpisodeProxy参数
    _value = detail::ActorFactory::MakeActor( // 调用ActorFactory的MakeActor方法创建一个演员
        episode, // 传入当前的episode
        boost::variant2::get<rpc::Actor>(std::move(_value)), // 从_variant中获取rpc::Actor对象并移动
        GarbageCollectionPolicy::Disabled); // 设置垃圾回收策略为禁用
  }

} // namespace detail
} // namespace client
} // namespace carla


