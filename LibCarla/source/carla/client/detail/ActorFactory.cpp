// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// 这项工作受到 MIT 许可证的条款保护。
// 请参阅<https://opensource.org/licenses/MIT>以获得副本。

#include "carla/client/detail/ActorFactory.h"

#include "carla/Logging.h"
#include "carla/StringUtil.h"
#include "carla/client/Actor.h"
#include "carla/client/LaneInvasionSensor.h"
#include "carla/client/ServerSideSensor.h"
#ifdef RSS_ENABLED
#include "carla/rss/RssSensor.h"
#endif
#include "carla/client/TrafficLight.h"
#include "carla/client/TrafficSign.h"
#include "carla/client/Vehicle.h"
#include "carla/client/Walker.h"
#include "carla/client/WalkerAIController.h"
#include "carla/client/World.h"
#include "carla/client/detail/Client.h"

#include <rpc/config.h>
#include <rpc/rpc_error.h>

#include <exception>

namespace carla {
namespace client {
namespace detail {

  // 无法对 deleter 抛出异常；与 std::unique_ptr 不同，deleter
  // (std|boost)::shared_ptr 的 deleter 即使受管指针为空也会被调用。
    // 结构体：GarbageCollector，管理 Actor 的生命周期并处理销毁逻辑。
  struct GarbageCollector {
      // 操作符重载：自定义 deleter 函数，用于在 shared_ptr 销毁时执行清理操作。
    void operator()(::carla::client::Actor *ptr) const noexcept {
      if ((ptr != nullptr) && ptr->IsAlive()) {
        try {
            // 销毁 Actor，释放资源
          ptr->Destroy();
          delete ptr;
        } catch (const ::rpc::timeout &timeout) {
            // 捕捉到超时错误时记录日志
          log_error(timeout.what());
          log_error(
              "timeout while trying to garbage collect Actor",
              ptr->GetDisplayId(),
              "actor hasn't been removed from the simulation");
        } catch (const std::exception &e) {
            // 捕捉到其他标准异常时记录日志并终止程序
          log_critical(
              "exception thrown while trying to garbage collect Actor",
              ptr->GetDisplayId(),
              e.what());
          std::terminate();// 终止程序
        } catch (...) {
            // 捕捉到未知异常时记录日志并终止程序
          log_critical(
              "unknown exception thrown while trying to garbage collect an Actor :",
              ptr->GetDisplayId());
          std::terminate();
        }
      }
    }
  };

  // 模板函数：根据传入的 Actor 初始化参数和垃圾回收策略创建 Actor 实例。
// ActorT 是 Actor 类型的模板参数，init 是 ActorInitializer，gc 是垃圾回收策略。
  template <typename ActorT>
  static auto MakeActorImpl(ActorInitializer init, GarbageCollectionPolicy gc) {
    if (gc == GarbageCollectionPolicy::Enabled) {
        // 启用垃圾回收时，创建并返回一个带有 GarbageCollector 的 shared_ptr
      return SharedPtr<ActorT>{new ActorT(std::move(init)), GarbageCollector()};
    }
    DEBUG_ASSERT(gc == GarbageCollectionPolicy::Disabled);// 确保禁用垃圾回收
    return SharedPtr<ActorT>{new ActorT(std::move(init))};// 否则直接返回普通的 shared_ptr
  }

  // ActorFactory 类的成员函数：创建不同类型的 Actor 实例。
// episode: 当前场景的 EpisodeProxy，代表仿真中的当前环境。
// description: rpc::Actor，包含 Actor 的描述信息。
// gc: 垃圾回收策略，决定是否启用垃圾回收。
  SharedPtr<Actor> ActorFactory::MakeActor(
      EpisodeProxy episode,
      rpc::Actor description,
      GarbageCollectionPolicy gc) {
      // 创建 ActorInitializer 实例，传递给具体 Actor 构造函数
    auto init = ActorInitializer{description, episode};
    // 判断传入的 Actor 描述信息，创建对应类型的 Actor 实例
    if (description.description.id == "sensor.other.lane_invasion") {
        // 创建车道入侵传感器实例
      return MakeActorImpl<LaneInvasionSensor>(std::move(init), gc);
#ifdef RSS_ENABLED
    } else if (description.description.id == "sensor.other.rss") {
      return MakeActorImpl<RssSensor>(std::move(init), gc);
#endif
    } else if (description.HasAStream()) {
        // 如果描述信息表示该 Actor 是服务端传感器，创建 ServerSideSensor 实例
      return MakeActorImpl<ServerSideSensor>(std::move(init), gc);
    } else if (StringUtil::StartsWith(description.description.id, "vehicle.")) {
      return MakeActorImpl<Vehicle>(std::move(init), gc);
    } else if (StringUtil::StartsWith(description.description.id, "walker.")) {
      return MakeActorImpl<Walker>(std::move(init), gc);
    } else if (StringUtil::StartsWith(description.description.id, "traffic.traffic_light")) {
      return MakeActorImpl<TrafficLight>(std::move(init), gc);
    } else if (StringUtil::StartsWith(description.description.id, "traffic.")) {
      return MakeActorImpl<TrafficSign>(std::move(init), gc);
    } else if (description.description.id == "controller.ai.walker") {
      return MakeActorImpl<WalkerAIController>(std::move(init), gc);
    }
    return MakeActorImpl<Actor>(std::move(init), gc);
  }

} // namespace detail
} // namespace client
} // namespace carla
