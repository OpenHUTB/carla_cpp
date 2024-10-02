// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/client/WalkerAIController.h"

#include "carla/client/detail/Simulator.h"
#include "carla/client/detail/WalkerNavigation.h"

namespace carla {
namespace client {

  WalkerAIController::WalkerAIController(ActorInitializer init)
    : Actor(std::move(init)) {}

  void WalkerAIController::Start() {
    GetEpisode().Lock()->RegisterAIController(*this);

    // 在 Recast & Detour 中添加行人
    auto walker = GetParent();
    if (walker != nullptr) {
      auto nav = GetEpisode().Lock()->GetNavigation();
      if (nav != nullptr) {
        nav->AddWalker(walker->GetId(), walker->GetLocation());
        // 禁用行人参与者的物理和碰撞
        GetEpisode().Lock()->SetActorSimulatePhysics(*walker, false);
        GetEpisode().Lock()->SetActorCollisions(*walker, false);
      }
    }
  }

  void WalkerAIController::Stop() {
    GetEpisode().Lock()->UnregisterAIController(*this);

    // 从 Recast & Detour 中移除行人
    auto walker = GetParent();
    if (walker != nullptr) {
      auto nav = GetEpisode().Lock()->GetNavigation();
      if (nav != nullptr) {
        nav->RemoveWalker(walker->GetId());
      }
    }
  }

  boost::optional<geom::Location> WalkerAIController::GetRandomLocation() {
    auto nav = GetEpisode().Lock()->GetNavigation();
    if (nav != nullptr) {
      return nav->GetRandomLocation();
    }
    return {};
  }

  void WalkerAIController::GoToLocation(const carla::geom::Location &destination) {
    auto nav = GetEpisode().Lock()->GetNavigation();
    if (nav != nullptr) {
      auto walker = GetParent();
      if (walker != nullptr) {
        if (!nav->SetWalkerTarget(walker->GetId(), destination)) {
          log_warning("NAV: Failed to set request to go to ", destination.x, destination.y, destination.z);
        }
      } else {
        log_warning("NAV: Failed to set request to go to ", destination.x, destination.y, destination.z, "(parent does not exist)");
      }
    }
  }

  void WalkerAIController::SetMaxSpeed(const float max_speed) {
    auto nav = GetEpisode().Lock()->GetNavigation();
    if (nav != nullptr) {
      auto walker = GetParent();
      if (walker != nullptr) {
        if (!nav->SetWalkerMaxSpeed(walker->GetId(), max_speed)) {
          log_warning("NAV: failed to set max speed");
        }
      } else {
        log_warning("NAV: failed to set max speed (parent does not exist)");
      }
    }
  }

} // namespace client
} // namespace carla
