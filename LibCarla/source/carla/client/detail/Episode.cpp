// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

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
// 模板函数，根据给定的演员ID范围获取演员列表
  template <typename RangeT>
  static auto GetActorsById_Impl(Client &client, CachedActorList &actors, const RangeT &actor_ids) {
  	// 获取缺失的演员ID
    auto missing_ids = actors.GetMissingIds(actor_ids);
    if (!missing_ids.empty()) {
    	// 如果有缺失的ID，从客户端获取对应演员并插入列表
      actors.InsertRange(client.GetActorsById(missing_ids));
    }
     // 返回指定ID的演员列表
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

          do {
            if (prev->GetFrame() >= next->GetFrame() && !episode_changed) {
              self->_on_tick_callbacks.Call(next);
              return;
            }
          } while (!self->_state.compare_exchange(&prev, next));

          if(UpdateLights || HasMapChanged) {
            self->_on_light_update_callbacks.Call(next);
          }

          if(HasMapChanged) {
            self->_should_update_map = true;
          }

          /// Episode 改变
          if(episode_changed) {
            self->OnEpisodeChanged();
          }

          // 通知等待的线程并执行回调。
          self->_snapshot.SetValue(next);

          // 调用用户回调函数
          self->_on_tick_callbacks.Call(next);
        }
      }
    });
  }
// 根据演员ID获取单个演员，如果不存在则从客户端获取并插入到缓存中
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
// 根据演员ID列表获取演员列表，使用模板函数实现
  std::vector<rpc::Actor> Episode::GetActorsById(const std::vector<ActorId> &actor_ids) {
    return GetActorsById_Impl(_client, _actors, actor_ids);
  }
// 获取所有演员列表，使用模板函数实现
  std::vector<rpc::Actor> Episode::GetActors() {
    return GetActorsById_Impl(_client, _actors, GetState()->GetActorIds());
  }
// 当Episode开始时的处理函数
  void Episode::OnEpisodeStarted() {
    _actors.Clear();
    _on_tick_callbacks.Clear();
    _walker_navigation.reset();
    traffic_manager::TrafficManager::Release();
  }
// 当Episode改变时的处理函数
  void Episode::OnEpisodeChanged() {
    traffic_manager::TrafficManager::Reset();
  }
// 检查自上次调用以来地图是否发生变化
  bool Episode::HasMapChangedSinceLastCall() {
    if(_should_update_map) {
      _should_update_map = false;
      return true;
    }
    return false;
  }
// 创建WalkerNavigation对象，如果不存在则创建并返回
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
