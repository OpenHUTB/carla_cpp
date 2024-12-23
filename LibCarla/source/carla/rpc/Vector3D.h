// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once// “#pragma once”是一个预处理指令，其作用是确保这个头文件在同一个编译单元中只会被包含一次。这样能有效避免因多次包含该头文件而引发的重复定义等编译问题，是C/C++编程中常用的头文件保护机制。

#include "carla/geom/Vector3D.h"// 引入“carla/geom/Vector3D.h”头文件，通常情况下，这个头文件中会定义与三维向量（Vector3D）相关的各种内容。比如，可能包含了三维向量的结构体定义，描述其在三维空间中的坐标表示（一般包含x、y、z三个分量），

namespace carla {//定义了嵌套的命名空间
namespace rpc {

  using Vector3D = geom::Vector3D;// 使用“using”关键字创建类型别名。在此处，将“rpc”命名空间下的“Vector3D”类型声明为等同于“carla::geom::Vector3D”类型。
    // 这么做带来的好处是，当在“rpc”这个特定的命名空间中编写代码时，如果需要使用三维向量类型
} // namespace rpc
} // namespace carla
