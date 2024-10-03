// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once // 防止头文件被重复包含

#include "carla/AtomicList.h" // 引入原子列表头文件
#include "carla/nav/Navigation.h" // 引入导航头文件
#include "carla/NonCopyable.h" // 引入不可复制类的头文件
#include "carla/client/Timestamp.h" // 引入时间戳头文件
#include "carla/rpc/ActorId.h" // 引入参与者ID头文件

#include <memory> // 引入智能指针头文件

namespace carla { // 定义carla命名空间
namespace client { // 定义client子命名空间
namespace detail { // 定义detail子命名空间

  class Episode; // 前向声明Episode类
  class EpisodeState; // 前向声明EpisodeState类
  class Simulator; // 前向声明Simulator类

  class WalkerNavigation // 定义WalkerNavigation类
    : public std::enable_shared_from_this<WalkerNavigation>, // 允许共享自身指针
      private NonCopyable { // 禁止复制

  public:

    explicit WalkerNavigation(std::weak_ptr<Simulator> simulator); // 构造函数，接受弱指针模拟器

    void RegisterWalker(ActorId walker_id, ActorId controller_id) { // 注册行人
      _walkers.Push(WalkerHandle { walker_id, controller_id }); // 添加到列表中
    }

    void UnregisterWalker(ActorId walker_id, ActorId controller_id) { // 注销行人
      auto list = _walkers.Load(); // 加载行人列表
      unsigned int i = 0; // 初始化索引
      while (i < list->size()) { // 遍历列表
        if ((*list)[i].walker == walker_id && // 如果找到匹配的行人
            (*list)[i].controller == controller_id) { // 且控制器匹配
          _walkers.DeleteByIndex(i); // 从列表中删除
          break; // 退出循环
        }
        ++i; // 继续下一个索引
      }
    }

    void RemoveWalker(ActorId walker_id) { // 移除行人
      _nav.RemoveAgent(walker_id); // 从导航中移除行人
    }

    void AddWalker(ActorId walker_id, carla::geom::Location location) { // 添加行人
      _nav.AddWalker(walker_id, location); // 在导航中创建行人
    }

    void Tick(std::shared_ptr<Episode> episode); // 更新函数，接受剧集的共享指针

    // 从导航网格中获取随机位置
    boost::optional<geom::Location> GetRandomLocation() {
      geom::Location random_location(0, 0, 0); // 初始化随机位置
      if (_nav.GetRandomLocation(random_location)) // 如果成功获取随机位置
        return boost::optional<geom::Location>(random_location); // 返回随机位置
      else
        return {}; // 否则返回空
    }

    // 设置行人的目标点
    bool SetWalkerTarget(ActorId id, const carla::geom::Location to) {
      return _nav.SetWalkerTarget(id, to); // 设置目标位置
    }

    // 设置行人的最大速度
    bool SetWalkerMaxSpeed(ActorId id, float max_speed) {
      return _nav.SetWalkerMaxSpeed(id, max_speed); // 设置最大速度
    }

    // 设置可以过马路的行人百分比
    void SetPedestriansCrossFactor(float percentage) {
      _nav.SetPedestriansCrossFactor(percentage); // 设置过马路的比例
    }

    void SetPedestriansSeed(unsigned int seed) { // 设置行人的随机种子
      _nav.SetSeed(seed); // 设置随机种子
    }

  private:

    std::weak_ptr<Simulator> _simulator; // 存储弱指针模拟器

    unsigned long _next_check_index; // 存储下一个检查索引

    carla::nav::Navigation _nav; // 存储导航对象

    struct WalkerHandle { // 定义WalkerHandle结构
      ActorId walker; // 存储行人ID
      ActorId controller; // 存储控制器ID
    };

    AtomicList<WalkerHandle> _walkers;

    /// 检查一些行人，如果不存在，则将其从人群中移除
    void CheckIfWalkerExist(std::vector<WalkerHandle> walkers, const EpisodeState &state);
    /// 添加/更新/删除人群中的所有车辆
    void UpdateVehiclesInCrowd(std::shared_ptr<Episode> episode, bool show_debug = false);
  };

} // namespace detail
} // namespace client
} // namespace carla
