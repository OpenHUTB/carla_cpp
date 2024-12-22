// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

// 防止头文件被多次包含
#pragma once 

// 包含 "carla/geom/Location.h" 头文件，可能包含与位置相关的类或函数的声明
#include "carla/geom/Location.h"

// 命名空间 carla，用于组织代码，防止命名冲突
namespace carla {
// 命名空间 rpc，可能是用于远程过程调用相关的代码组织
namespace rpc {

  // 使用 Location 类型，实际上是使用了 carla::geom::Location 类型，这样在这个 rpc 命名空间中可以直接使用 Location 来表示 carla::geom::Location，方便代码书写和阅读
  using Location = geom::Location;

} // namespace rpc
} // namespace carla
