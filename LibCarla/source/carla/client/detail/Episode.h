// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once // 防止重复包含

#include "carla/AtomicSharedPtr.h" // 引入原子共享指针
#include "carla/NonCopyable.h" // 引入不可复制类
#include "carla/RecurrentSharedFuture.h" // 引入递归共享未来
#include "carla/client/Timestamp.h" // 引入时间戳
#include "carla/client/WorldSnapshot.h" // 引入世界快照
#include "carla/client/detail/CachedActorList.h" // 引入缓存参与者列表
#include "carla/client/detail/CallbackList.h" // 引入回调列表
#include "carla/client/detail/EpisodeState.h" // 引入剧集状态
#include "carla/client/detail/EpisodeProxy.h" // 引入剧集代理
#include "carla/rpc/EpisodeInfo.h" // 引入剧集信息

#include <vector> // 引入向量类

namespace carla {
namespace client {
namespace detail {

  class Client; // 前向声明 Client 类
  class WalkerNavigation; // 前向声明 WalkerNavigation 类

  /// 持有当前剧集及当前剧集状态
  ///
  /// 每当接收到世界 tick 时，剧集状态在后台变化。
  /// 如果模拟器加载了新剧集，剧集可能会随任何后台更新而变化。
  class Episode
    : public std::enable_shared_from_this<Episode>, // 支持共享指针
      private NonCopyable { // 继承不可复制类
  public:

    explicit Episode(Client &client, std::weak_ptr<Simulator> simulator); // 构造函数

    ~Episode(); // 析构函数

    void Listen(); // 监听事件

    auto GetId() const { // 获取剧集 ID
      return GetState()->GetEpisodeId();
    }

    std::shared_ptr<const EpisodeState> GetState() const { // 获取剧集状态
      return _state.load();
    }

    void RegisterParticipant(rpc::Actor actor) { // 注册参与者
      _actors.Insert(std::move(actor));
    }

    boost::optional<rpc::Actor> GetParticipantById(ActorId id); // 根据 ID 获取参与者

    std::vector<rpc::Actor> GetParticipantsById(const std::vector<ActorId> &actor_ids); // 根据 ID 列表获取参与者

    std::vector<rpc::Actor> GetParticipants(); // 获取所有参与者

    boost::optional<WorldSnapshot> WaitForState(time_duration timeout) { // 等待状态变化
      return _snapshot.WaitFor(timeout);
    }

    size_t RegisterOnTickEvent(std::function<void(WorldSnapshot)> callback) { // 注册 tick 事件回调
      return _on_tick_callbacks.Push(std::move(callback));
    }

    void RemoveOnTickEvent(size_t id) { // 移除 tick 事件回调
      _on_tick_callbacks.Remove(id);
    }

    size_t RegisterOnMapChangeEvent(std::function<void(WorldSnapshot)> callback) { // 注册地图变化事件回调
      return _on_map_change_callbacks.Push(std::move(callback));
    }

    void RemoveOnMapChangeEvent(size_t id) { // 移除地图变化事件回调
      _on_map_change_callbacks.Remove(id);
    }

    size_t RegisterLightUpdateChangeEvent(std::function<void(WorldSnapshot)> callback) { // 注册光照更新事件回调
      return _on_light_update_callbacks.Push(std::move(callback));
    }

    void RemoveLightUpdateChangeEvent(size_t id) { // 移除光照更新事件回调
      _on_light_update_callbacks.Remove(id);
    }

    void SetPedestriansCrossFactor(float percentage); // 设置行人过马路的概率

    void SetPedestriansSeed(unsigned int seed); // 设置行人种子值

    void AddPendingException(std::string e) { // 添加待处理异常
      _pending_exceptions = true;
      _pending_exceptions_msg = e;
    }

    bool HasMapChangedSinceLastCall(); // 检查地图是否自上次调用后变化

    std::shared_ptr<WalkerNavigation> CreateNavigationIfMissing(); // 如果缺失则创建导航

  private:

    Episode(Client &client, const rpc::EpisodeInfo &info, std::weak_ptr<Simulator> simulator); // 私有构造函数

    void OnEpisodeStarted(); // 处理剧集开始事件

    void OnEpisodeChanged(); // 处理剧集变化事件

    Client &_client; // 引用客户端

    AtomicSharedPtr<const EpisodeState> _state; // 原子共享指针指向剧集状态

    std::string _pending_exceptions_msg; // 待处理异常消息

    CachedActorList _actors; // 缓存的参与者列表

    CallbackList<WorldSnapshot> _on_tick_callbacks; // tick 事件回调列表

    CallbackList<WorldSnapshot> _on_map_change_callbacks; // 地图变化事件回调列表

    CallbackList<WorldSnapshot> _on_light_update_callbacks; // 光照更新事件回调列表

    RecurrentSharedFuture<WorldSnapshot> _snapshot; // 递归共享未来的世界快照

    AtomicSharedPtr<WalkerNavigation> _walker_navigation; // 原子共享指针指向 WalkerNavigation

    const streaming::Token _token; // 令牌

    bool _pending_exceptions = false; // 是否有待处理异常

    bool _should_update_map = true; // 是否应该更新地图

    std::weak_ptr<Simulator> _simulator; // 弱指针指向模拟器
  };

} // namespace detail
} // namespace client
} // namespace carla
