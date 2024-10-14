// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/client/detail/EpisodeProxy.h"

#include "carla/Exception.h"
#include "carla/client/detail/Simulator.h"

#include <exception>

namespace carla {
namespace client {
namespace detail {
// 静态函数，用于加载强指针类型的 EpisodeProxyPointerType
  static EpisodeProxyPointerType::Shared Load(EpisodeProxyPointerType::Strong ptr) {
    return ptr.load();
  }
// 静态函数，用于加载弱指针类型的 EpisodeProxyPointerType
  static EpisodeProxyPointerType::Shared Load(EpisodeProxyPointerType::Weak ptr) {
    return ptr.lock();
  }
// 模板类 EpisodeProxyImpl 的实现，用于处理强指针类型和弱指针类型的 EpisodeProxyPointerType
  template <typename T>
  EpisodeProxyImpl<T>::EpisodeProxyImpl(SharedPtrType simulator)
    : _episode_id(simulator != nullptr ? simulator->GetCurrentEpisodeId() : 0u),
      _simulator(std::move(simulator)) {}
// 尝试锁定 EpisodeProxyImpl，返回共享指针，如果当前指针有效则返回非空指针，否则返回空指针
  template <typename T>
  typename EpisodeProxyImpl<T>::SharedPtrType EpisodeProxyImpl<T>::TryLock() const noexcept {
    auto ptr = Load(_simulator);
    const bool is_valid = (ptr != nullptr) && (_episode_id == ptr->GetCurrentEpisodeId());
    return is_valid ? ptr : nullptr;
  }
// 锁定 EpisodeProxyImpl，如果模拟器指针为空则抛出异常，否则返回共享指针
  template <typename T>
  typename EpisodeProxyImpl<T>::SharedPtrType EpisodeProxyImpl<T>::Lock() const {
    auto ptr = Load(_simulator);
    if (ptr == nullptr) {
      throw_exception(std::runtime_error(
          "trying to operate on a destroyed actor; an actor's function "
          "was called, but the actor is already destroyed."));
    }
    return ptr;
  }
// 清除 EpisodeProxyImpl 的模拟器指针
  template <typename T>
  void EpisodeProxyImpl<T>::Clear() noexcept {
    _simulator.reset();
  }
// 实例化模板类 EpisodeProxyImpl 分别用于强指针类型和弱指针类型
  template class EpisodeProxyImpl<EpisodeProxyPointerType::Strong>;

  template class EpisodeProxyImpl<EpisodeProxyPointerType::Weak>;

} // namespace detail
} // namespace client
} // namespace carla
