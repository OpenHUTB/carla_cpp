// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.


//包含头文件
#include "carla/client/WalkerAIController.h" // 引入 WalkerAIController 类的头文件，用于控制虚拟行人AI行为

#include "carla/client/detail/Simulator.h" // 引入 Simulator 类的头文件，用于访问 Carla 仿真环境的模拟器功能
#include "carla/client/detail/WalkerNavigation.h" // 引入 WalkerNavigation 类的头文件，用于虚拟行人的导航系统

namespace carla {
namespace client {


  // 构造函数: 初始化 WalkerAIController 对象，并将初始化参数传递给父类 Actor。
  // 该控制器用于管理 AI 行人行为，并与导航系统交互。
  WalkerAIController::WalkerAIController(ActorInitializer init)
    : Actor(std::move(init)) {}


  // 方法 Start: 启动控制器时调用，注册该 AI 控制器到模拟环境，并将行人添加到导航系统中。
  // 在 Recast & Detour 导航系统中为行人创建路径，并禁用物理与碰撞计算。
  void WalkerAIController::Start() {
    // 注册 AI 控制器以启用模拟中的控制功能
    GetEpisode().Lock()->RegisterAIController(*this);


    // 获取当前控制的行人对象（如果存在）
    auto walker = GetParent();   
    //GetParent()获取控制器控制的行人对象
    if (walker != nullptr) {
      // 获取当前模拟环境的导航系统
      auto nav = GetEpisode().Lock()->GetNavigation();
      if (nav != nullptr) {
        // 将行人添加到导航系统中，提供行人 ID 和当前位置
        nav->AddWalker(walker->GetId(), walker->GetLocation());

        // 禁用行人的物理模拟与碰撞计算
        GetEpisode().Lock()->SetActorSimulatePhysics(*walker, false);
        GetEpisode().Lock()->SetActorCollisions(*walker, false);
      }
    }
  }


  // 方法 Stop: 停止控制器的工作，取消注册并从导航系统中移除行人。
  // 此方法用于关闭控制器，解除对行人对象的管理。
  void WalkerAIController::Stop() {
    // 取消注册该 AI 控制器
    GetEpisode().Lock()->UnregisterAIController(*this);

    // 获取当前控制的行人对象（如果存在）
    auto walker = GetParent();
    if (walker != nullptr) {
      // 获取当前模拟环境的导航系统
      auto nav = GetEpisode().Lock()->GetNavigation();
      if (nav != nullptr) {
        // 从导航系统中移除该行人
        nav->RemoveWalker(walker->GetId());
      }
    }
  }


  // 方法 GetRandomLocation: 获取一个随机的导航位置，供 AI 行人使用。
  // 返回值是一个可选的地点（如果导航系统可用），通常用于设置行人的随机目标位置。
  boost::optional<geom::Location> WalkerAIController::GetRandomLocation() {
      //GetRandomLocation()方法返回一个随机的位置，通常用于让AI行人随机选择一个目标位置。
    auto nav = GetEpisode().Lock()->GetNavigation(); // 获取当前模拟环境的导航系统
    if (nav != nullptr) {
      // 从导航系统中获取一个随机的可行走位置
      return nav->GetRandomLocation();
    }
    return {};
  }


  // 方法 GoToLocation: 使 AI 行人朝着指定的目标位置前进。
  // 行人将根据导航系统规划的路径向目标位置移动。

  void WalkerAIController::GoToLocation(const carla::geom::Location &destination) {
    // 获取当前模拟环境的导航系统
    auto nav = GetEpisode().Lock()->GetNavigation();
    if (nav != nullptr) {
      // 获取控制的行人对象
      auto walker = GetParent();
      if (walker != nullptr) {
        // 请求导航系统为该行人设置目标位置
        if (!nav->SetWalkerTarget(walker->GetId(), destination)) {
          log_warning("NAV: Failed to set request to go to ", destination.x, destination.y, destination.z);
        }
      } else {
        log_warning("NAV: Failed to set request to go to ", destination.x, destination.y, destination.z, "(parent does not exist)");
      }
    }
  }


  // 方法 SetMaxSpeed: 设置行人的最大速度，控制行人移动的速度限制。
  // 行人速度的最大值将影响其在导航中的运动表现。

  void WalkerAIController::SetMaxSpeed(const float max_speed) {
    // 获取当前模拟环境的导航系统
    auto nav = GetEpisode().Lock()->GetNavigation();
    if (nav != nullptr) {
      // 获取控制的行人对象
      auto walker = GetParent();
      if (walker != nullptr) {
        // 设置行人的最大速度，如果设置失败，输出警告日志
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
