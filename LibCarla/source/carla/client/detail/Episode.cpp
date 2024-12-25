// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.
// 包含Carla项目中客户端相关的Episode（场景、情节相关概念，可能代表一次模拟过程等）的头文件，
// 推测其中定义了Episode类的声明等内容，用于处理模拟场景相关的操作和状态管理
#include "carla/client/detail/Episode.h"

#include "carla/Logging.h"
#include "carla/client/detail/Client.h"
#include "carla/client/detail/WalkerNavigation.h"
#include "carla/sensor/Deserializer.h"
#include "carla/trafficmanager/TrafficManager.h"

#include <exception>

namespace carla {
namespace client {
namespace detail {
// 使用命名空间中的chrono_literals，用于方便地表示时间常量
using namespace std::chrono_literals;
// 静态函数，将传感器数据强制转换为特定类型
  static auto &CastData(const sensor::SensorData &data) {
    using target_t = const sensor::data::RawEpisodeState;
    return static_cast<target_t &>(data);
  }
// 模板函数，根据给定的参与者ID范围获取演员列表
  template <typename RangeT>
  static auto GetActorsById_Impl(Client &client, CachedActorList &actors, const RangeT &actor_ids) {
  	// 获取缺失的参与者ID
    auto missing_ids = actors.GetMissingIds(actor_ids);
    if (!missing_ids.empty()) {
    	// 如果有缺失的ID，从客户端获取对应参与者并插入列表
      actors.InsertRange(client.GetActorsById(missing_ids));
    }
     // 返回指定ID的参与者列表
    return actors.GetActorsById(actor_ids);
  }
// 构造函数，通过客户端和弱引用的模拟器创建Episode对象，并使用默认的EpisodeInfo
  Episode::Episode(Client &client, std::weak_ptr<Simulator> simulator)
    : Episode(client, client.GetEpisodeInfo(), simulator) {}
// 构造函数，通过客户端、EpisodeInfo和弱引用的模拟器创建Episode对象
  Episode::Episode(Client &client, const rpc::EpisodeInfo &info, std::weak_ptr<Simulator> simulator)
    : _client(client),
      _state(std::make_shared<EpisodeState>(info.id)),
      _simulator(simulator),
      _token(info.token) {}
// 析构函数，尝试取消订阅流并处理可能的异常
  Episode::~Episode() {
    try {
      _client.UnSubscribeFromStream(_token);
    } catch (const std::exception &e) {
      log_error("exception trying to disconnect from episode:", e.what());
    }
  }
// 开始监听流数据的函数
  void Episode::Listen() {
    std::weak_ptr<Episode> weak = shared_from_this();
    _client.SubscribeToStream(_token, [weak](auto buffer) {
      auto self = weak.lock();
      if (self != nullptr) {
        // 反序列化数据
        auto data = sensor::Deserializer::Deserialize(std::move(buffer));
        auto next = std::make_shared<const EpisodeState>(CastData(*data));
        auto prev = self->GetState();

        // TODO: 更新地图变化的检测方式
        bool HasMapChanged = next->HasMapChanged();
        bool UpdateLights = next->IsLightUpdatePending();

        /// 检查待处理的异常（主要是交通管理服务器关闭）
        if(self->_pending_exceptions) {

          /// 将待处理的异常标记为 false
          self->_pending_exceptions = false;

          /// 为错误消息创建异常
          auto exception(self->_pending_exceptions_msg);
          // 通知等待线程发生异常
          self->_snapshot.SetException(std::runtime_error(exception));
        }
        /// 传感器案例：数据不一致
        else {
          bool episode_changed = (next->GetEpisodeId() != prev->GetEpisodeId());

          do {// 如果当前状态的帧编号（GetFrame函数获取）大于等于下一个状态的帧编号，并且模拟场景没有改变（episode_changed为false），
                        // 则调用_on_tick_callbacks（可能是一个用于存储和执行每帧回调函数的对象）的Call函数，传入下一个状态数据，然后直接返回，
                        // 表示这种情况下已经处理完当前数据，不需要进行后续的状态更新等操作。
            if (prev->GetFrame() >= next->GetFrame() && !episode_changed) {
              self->_on_tick_callbacks.Call(next);
              return;
            }
          } while (!self->_state.compare_exchange(&prev, next));
// 如果需要更新交通信号灯（UpdateLights为true）或者地图发生了变化（HasMapChanged为true），
                    // 则调用_on_light_update_callbacks（可能是一个用于处理交通信号灯更新相关回调函数的对象）的Call函数，传入下一个状态数据。
          if(UpdateLights || HasMapChanged) {
            self->_on_light_update_callbacks.Call(next);
          }
// 如果地图发生了变化，将_should_update_map标记为true，表示需要在后续的操作中更新地图相关的内容。
          if(HasMapChanged) {
            self->_should_update_map = true;
          }

          /// Episode 改变
          if(episode_changed) {
            self->OnEpisodeChanged();
          }

          // 通知等待的线程并执行回调。
          self->_snapshot.SetValue(next);
/ 通知等待的线程并执行回调，通过调用_snapshot的SetValue函数，传入下一个状态数据，
                    // 这样其他等待该状态数据的部分（可能是其他线程或者模块）就可以获取到最新的状态并进行相应操作。
                    // 同时调用_on_tick_callbacks的Call函数，传入下一个状态数据，执行用户注册的每帧回调函数。

          self->_on_tick_callbacks.Call(next);
        }
      }
    });
  }
// Episode类的成员函数GetActorById，用于根据给定的参与者ID获取单个参与者信息。
    // 首先尝试从缓存的参与者列表_actors中获取对应的参与者信息（通过调用GetActorById函数），如果获取不到（返回的结果没有值），
    // 则从客户端获取该ID对应的参与者信息列表（通过调用_client的GetActorsById函数传入单个ID的列表），
    // 如果获取到的列表不为空，则取出第一个参与者信息（假设ID是唯一对应的），并插入到缓存列表_actors中，最后返回获取到的参与者信息（如果有的话）
  boost::optional<rpc::Actor> Episode::GetActorById(ActorId id) {
    auto actor = _actors.GetActorById(id);
    if (!actor.has_value()) {
      auto actor_list = _client.GetActorsById({id});
      if (!actor_list.empty()) {
        actor = std::move(actor_list.front());
        _actors.Insert(*actor);
      }
    }
    return actor;
  }
// Episode类的成员函数GetActorsById，用于根据给定的参与者ID列表获取对应的参与者列表。
    // 它调用了前面定义的模板函数GetActorsById_Impl，传入客户端对象、缓存的参与者列表对象以及给定的参与者ID列表，
    // 通过模板函数内部的逻辑来获取并返回对应的参与者列表（如果有的话），可能涉及从缓存中获取或者从客户端获取缺失的参与者信息等操作。
  std::vector<rpc::Actor> Episode::GetActorsById(const std::vector<ActorId> &actor_ids) {
    return GetActorsById_Impl(_client, _actors, actor_ids);
  }
// Episode类的成员函数GetActors，用于获取所有的参与者列表。
    // 它同样调用了模板函数GetActorsById_Impl，不过传入的参与者ID范围是通过调用GetState函数获取当前模拟场景状态中的所有参与者ID列表，
    // 以此来获取并返回整个模拟场景中的所有参与者信息列表（如果有的话）。
  std::vector<rpc::Actor> Episode::GetActors() {
    return GetActorsById_Impl(_client, _actors, GetState()->GetActorIds());
  }
// Episode类的成员函数OnEpisodeStarted，用于在模拟场景（Episode）开始时执行一些初始化和清理操作。
    // 它会清空缓存的参与者列表_actors，清除每帧回调函数列表_on_tick_callbacks，重置行人导航对象_walker_navigation（如果有的话），
    // 以及调用交通管理模块（TrafficManager）的Release函数（可能用于释放之前的资源或者重置相关状态等操作）。
  void Episode::OnEpisodeStarted() {
    _actors.Clear();
    _on_tick_callbacks.Clear();
    _walker_navigation.reset();
    traffic_manager::TrafficManager::Release();
  }
// Episode类的成员函数OnEpisodeChanged，用于在模拟场景（Episode）发生改变（比如场景切换、重新初始化等情况）时执行相应的操作。
    // 这里调用了交通管理模块（TrafficManager）的Reset函数，可能用于重置交通管理相关的状态、参数等内容，以适应新的模拟场景情况。
  void Episode::OnEpisodeChanged() {
    traffic_manager::TrafficManager::Reset();
  }
// Episode类的成员函数HasMapChangedSinceLastCall，用于检查自上次调用该函数以来地图是否发生了变化。
    // 通过检查_should_update_map成员变量（如果为true表示地图需要更新，即发生了变化），如果为true则将其重置为false并返回true，
    // 表示地图发生了变化；如果为false则直接返回false，表示地图没有发生变化。
  bool Episode::HasMapChangedSinceLastCall() {
    if(_should_update_map) {
      _should_update_map = false;
      return true;
    }
    return false;
  }// Episode类的成员函数CreateNavigationIfMissing，用于创建一个WalkerNavigation对象（行人导航对象），如果该对象还不存在的话。
    // 通过一个循环和原子操作compare_exchange来确保只有一个线程能够成功创建新的WalkerNavigation对象（如果当前不存在的话），
    // 一旦创建成功或者已经存在，则返回指向该WalkerNavigation对象的共享指针。
  std::shared_ptr<WalkerNavigation> Episode::CreateNavigationIfMissing() {
    std::shared_ptr<WalkerNavigation> nav;
    do {
      nav = _walker_navigation.load();
      if (nav == nullptr) {
        auto new_nav = std::make_shared<WalkerNavigation>(_simulator);
        _walker_navigation.compare_exchange(&nav, new_nav);
      }
    } while (nav == nullptr);
    return nav;
  }

} // namespace detail
} // namespace client
} // namespace carla
