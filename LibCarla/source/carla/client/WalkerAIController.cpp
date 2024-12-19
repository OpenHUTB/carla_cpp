// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.
//文件头部注释,版权声明和许可信息.

//包含头文件
#include "carla/client/WalkerAIController.h"

#include "carla/client/detail/Simulator.h"
#include "carla/client/detail/WalkerNavigation.h"

namespace carla {
namespace client {

  //构造函数,初始化WalkerAIController并传递初始化参数给父类Actor
  WalkerAIController::WalkerAIController(ActorInitializer init)
    : Actor(std::move(init)) {} 

  //方法Start,Start()方法在控制器启动时调用，注册AI控制器并让行人加入导航系统。
  void WalkerAIController::Start() {
    GetEpisode().Lock()->RegisterAIController(*this);

    // 在 Recast & Detour 中添加行人
    auto walker = GetParent();   
    //GetParent()获取控制器控制的行人对象
    if (walker != nullptr) {
      auto nav = GetEpisode().Lock()->GetNavigation();
      //GetEpisode().Lock()获取当前模拟环境（Episode）的锁，确保线程安全。
      if (nav != nullptr) {
        nav->AddWalker(walker->GetId(), walker->GetLocation());
        // 禁用行人参与者的物理和碰撞
        //AddWalker(walker->GetId(), walker->GetLocation())将行人添加到导航系统中，传递行人的ID和位置。
        GetEpisode().Lock()->SetActorSimulatePhysics(*walker, false);
        GetEpisode().Lock()->SetActorCollisions(*walker, false);
      }
    }
  }

  //方法Stop,Stop()方法用于停止控制器的工作，解除对行人对象的管理。
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

  // 方法GetRandomLocation
  // 获取一个随机的导航位置，供行人AI使用
  boost::optional<geom::Location> WalkerAIController::GetRandomLocation() {
      //GetRandomLocation()方法返回一个随机的位置，通常用于让AI行人随机选择一个目标位置。
    auto nav = GetEpisode().Lock()->GetNavigation();
    if (nav != nullptr) {
      return nav->GetRandomLocation();
    }
    return {};
  }

  //方法GoToLocation,GoToLocation()方法使得AI行人朝着指定的目标位置前进。
  void WalkerAIController::GoToLocation(const carla::geom::Location &destination) {
    auto nav = GetEpisode().Lock()->GetNavigation();
    if (nav != nullptr) {
      auto walker = GetParent();
      if (walker != nullptr) {
        if (!nav->SetWalkerTarget(walker->GetId(), destination)) {
            //SetWalkerTarget(walker->GetId(), destination)设置行人的目标位置
          log_warning("NAV: Failed to set request to go to ", destination.x, destination.y, destination.z);
        }
      } else {
        log_warning("NAV: Failed to set request to go to ", destination.x, destination.y, destination.z, "(parent does not exist)");
      }
    }
  }

  //方法SetMaxSpeed,SetMaxSpeed()方法设置行人的最大速度。
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
        //// 如果设置失败，则输出警告日志
      }
    }
  }

} // namespace client
} // namespace carla
