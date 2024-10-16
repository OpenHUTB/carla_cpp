// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once /// 防止头文件被多次包含

#include "carla/Debug.h"
#include "carla/road/element/Geometry.h" /// 包含道路元素的几何定义
#include "carla/road/element/RoadInfo.h" /// 包含道路信息的定义

namespace carla {
namespace road {
namespace element {

  class RoadInfoGeometry final : public RoadInfo { // 构造函数，接受一个double类型的s参数和unique_ptr<Geometry>类型的geom参数
  public:

    RoadInfoGeometry(double s, std::unique_ptr<Geometry> &&geom)
      : RoadInfo(s), /// 调用基类RoadInfo的构造函数，传递s参数
        _geom(std::move(geom)) { /// 使用std::move将geom参数移动到成员变量_geom
      DEBUG_ASSERT(_geom != nullptr); /// 使用DEBUG_ASSERT宏确保_geom不为nullptr，以避免空指针解引用
    }

    void AcceptVisitor(RoadInfoVisitor &v) final { /// 接受访问者模式的访问
      v.Visit(*this); /// 访问者访问当前对象
    }

    const Geometry &GetGeometry() const { /// 获取道路的几何信息
      return *_geom; /// 返回_geom对象的引用
    }

  private:

    const std::unique_ptr<const Geometry> _geom; /// 存储道路几何信息的智能指针
  };

} // namespace element
} // namespace road
} // namespace carla
