// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
// 这是一个预处理指令，用于确保该头文件在整个编译过程中只会被包含一次

#include "carla/MsgPack.h"
// 引入carla项目中的MsgPack.h头文件

#include "carla/rpc/BoneTransformDataOut.h"
// 引入carla项目中rpc模块下的BoneTransformDataOut.h头文件

#include "carla/rpc/String.h"
// 引入carla项目中rpc模块下的String.h头文件

#include "carla/rpc/Transform.h"
// 引入carla项目中rpc模块下的Transform.h头文件

#ifdef LIBCARLA_INCLUDED_FROM_UE4
// 条件编译指令，判断是否定义了LIBCARLA_INCLUDED_FROM_UE4这个宏，如果定义了，则编译下面包含的代码块内容

#include <compiler/enable-ue4-macros.h>
// 当满足上述条件（从UE4项目中引入该头文件）时，引入compiler/enable-ue4-macros.h头文件

#include "Carla/Walker/WalkerBoneControlOut.h"
// 引入Carla/Walker目录下的WalkerBoneControlOut.h头文件

#include <compiler/disable-ue4-macros.h>
// 在引入完UE4相关的头文件后，引入compiler/disable-ue4-macros.h头文件

#endif // LIBCARLA_INCLUDED_FROM_UE4

#include <vector>
// 引入C++标准库中的vector头文件，vector是一个动态大小的数组容器，用于方便地存储和操作一组同类型的元素

namespace carla {
namespace rpc {

  class WalkerBoneControlOut {
 // 开始定义名为WalkerBoneControlOut的类

  public:

    WalkerBoneControlOut() = default;
// 定义类的默认构造函数，使用 = default语法让编译器自动生成默认构造函数的实现，也就是会进行默认的初始化操作

    explicit WalkerBoneControlOut(
        std::vector<rpc::BoneTransformDataOut> bone_transforms)
      : bone_transforms(bone_transforms) {}
 // 定义一个显式的构造函数，它接受一个std::vector<rpc::BoneTransformDataOut>类型的参数bone_transforms，用于初始化类中的同名成员变量bone_transforms，通过初始化列表的方式将传入的参数值赋给成员变量

    std::vector<rpc::BoneTransformDataOut> bone_transforms;
 // 定义一个成员变量bone_transforms，类型为std::vector<rpc::BoneTransformDataOut>，用于存储多个BoneTransformDataOut类型的对象，也就是一组骨骼变换相关的数据，代表了步行者骨骼的各种变换情况信息

    MSGPACK_DEFINE_ARRAY(bone_transforms);
是一个和MsgPack序列化库相关的宏，用于告诉MsgPack序列化库如何将包含bone_transforms成员变量的WalkerBoneControlOut类对象进行序列化操作

  };

} // namespace rpc
} // namespace carla
