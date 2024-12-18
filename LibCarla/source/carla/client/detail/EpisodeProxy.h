// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/AtomicSharedPtr.h"

#include <cstdint>

namespace carla {
namespace client {
namespace detail {

  class Simulator;// 前向声明 Simulator 类，表示仿真器类，用于与仿真环境进行交互

  // 定义 EpisodeProxyPointerType 结构体，封装了不同类型的智能指针，用于管理对 Simulator 的引用
  //定义了三种不同类型的智能指针，用于引用 Simulator 对象。每种指针类型具有不同的生命周期管理策略
  struct EpisodeProxyPointerType {
    using Shared = std::shared_ptr<Simulator>;
    using Strong = AtomicSharedPtr<Simulator>;
    using Weak = std::weak_ptr<Simulator>;
  };

  /// 在给定情节期间提供对模拟器的访问。
  /// 情节结束后，对模拟器的任何访问都会引发 std::runtime_error。
  template <typename PointerT>
  class EpisodeProxyImpl {
  public:

      // 构造函数，初始化时传入一个 SharedPtrType（SharedPtr 类型的 Simulator 智能指针）
    using SharedPtrType = EpisodeProxyPointerType::Shared;

    EpisodeProxyImpl() = default;

    // 构造函数，初始化时传入一个 SharedPtrType
    EpisodeProxyImpl(SharedPtrType simulator);

    // 复制构造函数：将另一个 EpisodeProxyImpl<T> 对象的状态复制过来
    template <typename T>
    EpisodeProxyImpl(EpisodeProxyImpl<T> other)
      : _episode_id(other._episode_id), // 复制 episode_id
        _simulator(other._simulator) {}// 复制 simulator 智能指针

    // 返回 episode 的唯一 ID
    auto GetId() const noexcept {
      return _episode_id;
    }
    // 尝试获取锁，返回一个 SharedPtr（可能为空）
    SharedPtrType TryLock() const noexcept;

    /// 与 TryLock 相同，但永远不会返回 nullptr。
    ///
    /// @throw 如果剧集结束，则发生 std::runtime_error。
    SharedPtrType Lock() const;

    // 检查当前 episode 是否有效，即 TryLock 是否返回非 nullptr
    bool IsValid() const noexcept {
      return TryLock() != nullptr;
    }

    // 清空当前 EpisodeProxyImpl 的状态
    void Clear() noexcept;

  private:

      // 友元声明：允许其他模板实例访问该类的私有成员
    template <typename T>
    friend class EpisodeProxyImpl;

    uint64_t _episode_id;// 存储当前 episode 的唯一 ID

    PointerT _simulator; // 存储 Simulator 类型的智能指针
  };
  // 定义 EpisodeProxy 类型，使用 Strong 类型的智能指针来管理 Simulator
  using EpisodeProxy = EpisodeProxyImpl<EpisodeProxyPointerType::Strong>;
  // 定义 WeakEpisodeProxy 类型，使用 Weak 类型的智能指针来管理 Simulator
  using WeakEpisodeProxy = EpisodeProxyImpl<EpisodeProxyPointerType::Weak>;

} // namespace detail
} // namespace client
} // namespace carla
