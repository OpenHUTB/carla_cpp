// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once// 预处理指令，通过#pragma once来确保该头文件在一个编译单元中只会被包含一次

#include "carla/geom/Transform.h"// 推测这个头文件中定义了与几何变换相关的结构体、类或者函数等内容

namespace carla {
namespace rpc {

  using Rotation = geom::Rotation;// 使用using关键字创建类型别名 Rotation，将其等同于 carla::geom::Rotation 类型

  using Transform = geom::Transform;// 同样地，使用using关键字创建类型别名 Transform，使其等同于 carla::geom::Transform 类型。

} // namespace rpc
} // namespace carla
