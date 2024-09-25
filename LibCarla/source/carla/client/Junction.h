// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Memory.h"   // 引入内存管理相关的头文件
#include "carla/NonCopyable.h"    // 引入不可复制类的头文件
#include "carla/road/Junction.h"    // 引入道路交叉口的相关定义
#include "carla/road/RoadTypes.h"   // 引入道路交叉口的相关定义
#include "carla/geom/BoundingBox.h"   // 引入边界框的相关定义
#include "carla/client/Waypoint.h"    // 引入路径点的相关定义

#include <vector>    // 引入标准库中的向量容器


namespace carla {     // 定义carla命名空间
namespace client {    // 定义client命名空间

  class Map;   // 前向声明Map类

  class Junction     // 定义Junction类
    : public EnableSharedFromThis<Junction>,  // 允许共享自身的指针
    private NonCopyable   // 禁止拷贝构造
  {
  public:

    carla::road::JuncId GetId() const {   // 获取交叉口ID
      return _id;   // 返回交叉口ID
    }

    std::vector<std::pair<SharedPtr<Waypoint>,SharedPtr<Waypoint>>> GetWaypoints(    // 获取路径点
        road::Lane::LaneType type = road::Lane::LaneType::Driving) const;    // 默认类型为行驶道

    geom::BoundingBox GetBoundingBox() const;   // 获取交叉口的边界框

  private:

    friend class Map;    // 声明Map类为友元类，允许其访问私有成员

    Junction(SharedPtr<const Map> parent, const road::Junction *junction);   // 构造函数

    SharedPtr<const Map> _parent;   // 父地图的共享指针

    geom::BoundingBox _bounding_box;   // 交叉口的边界框

    road::JuncId _id;    // 交叉口ID
  };

} // namespace client
} // namespace carla
