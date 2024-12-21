// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once// 预处理指令#pragma once用于确保该头文件在一个编译单元中只会被包含一次，避免出现重复定义等编译错误。

#include "carla/geom/Vector2D.h"// 引入 "carla/geom/Vector2D.h" 头文件，推测这个头文件中定义了二维向量（Vector2D）相关的数据结构、操作函数等内容，

namespace carla {
namespace rpc {

  using Vector2D = geom::Vector2D;// 使用using关键字创建类型别名，这里将rpc命名空间下的 Vector2D 定义为等同于 carla::geom::Vector2D 类型，

} // namespace rpc
} // namespace carla
