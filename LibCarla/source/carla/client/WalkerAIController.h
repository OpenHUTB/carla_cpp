// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/client/Actor.h"   // 包含 Actor 类的头文件
#include "carla/geom/Vector3D.h"   // 包含三维向量的头文件

#include <boost/optional.hpp>   // 包含 Boost 库中的可选类型的头文件

namespace carla {  // 开始 carla 命名空间
namespace client {    // 开始 client 命名空间

  class WalkerAIController : public Actor {  // 定义 WalkerAIController 类，继承自 Actor 类
  public:

    explicit WalkerAIController(ActorInitializer init);  // 构造函数，接受 ActorInitializer 类型的参数


    void Start();   // 启动 WalkerAIController 的成员函数

    void Stop();   // 停止 WalkerAIController 的成员函数

    boost::optional<geom::Location> GetRandomLocation();   // 获取随机位置的成员函数，返回可选的几何位置

    void GoToLocation(const carla::geom::Location &destination);    // 移动到指定位置的成员函数


    void SetMaxSpeed(const float max_speed);     // 设置最大速度的成员函数
  };

} // namespace client
} // namespace carla
