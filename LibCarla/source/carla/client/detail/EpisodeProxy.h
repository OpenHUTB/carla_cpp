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

  class Simulator;

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

    using SharedPtrType = EpisodeProxyPointerType::Shared;

    EpisodeProxyImpl() = default;

    EpisodeProxyImpl(SharedPtrType simulator);

    template <typename T>
    EpisodeProxyImpl(EpisodeProxyImpl<T> other)
      : _episode_id(other._episode_id),
        _simulator(other._simulator) {}

    auto GetId() const noexcept {
      return _episode_id;
    }

    SharedPtrType TryLock() const noexcept;

    /// 与 TryLock 相同，但永远不会返回 nullptr。
    ///
    /// @throw 如果剧集结束，则发生 std::runtime_error。
    SharedPtrType Lock() const;

    bool IsValid() const noexcept {
      return TryLock() != nullptr;
    }

    void Clear() noexcept;

  private:

    template <typename T>
    friend class EpisodeProxyImpl;

    uint64_t _episode_id;

    PointerT _simulator;
  };

  using EpisodeProxy = EpisodeProxyImpl<EpisodeProxyPointerType::Strong>;

  using WeakEpisodeProxy = EpisodeProxyImpl<EpisodeProxyPointerType::Weak>;

} // namespace detail
} // namespace client
} // namespace carla
