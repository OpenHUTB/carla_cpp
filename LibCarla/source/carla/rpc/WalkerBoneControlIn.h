// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once // 预处理指令，确保头文件只被包含一次，避免编译时重复定义

#include "carla/MsgPack.h" // 包含Carla项目中的MsgPack库，用于消息打包和序列化
#include "carla/rpc/BoneTransformDataIn.h"  // 包含骨骼变换数据结构的定义，用于RPC通信
#include "carla/rpc/String.h" // 包含RPC模块中定义的字符串类型
#include "carla/rpc/Transform.h" // 包含变换数据结构的定义，用于表示位置和方向

#ifdef LIBCARLA_INCLUDED_FROM_UE4
#include <compiler/enable-ue4-macros.h>
#include "Carla/Walker/WalkerBoneControlIn.h"  // 包含UE4项目中定义的WalkerBoneControlIn结构
#include <compiler/disable-ue4-macros.h>
#endif // LIBCARLA_INCLUDED_FROM_UE4

#include <vector> // 包含标准模板库中的向量容器

namespace carla {
namespace rpc {

  class WalkerBoneControlIn { // 定义WalkerBoneControlIn类，用于控制步行者（如角色）的骨骼变换
  public:

    WalkerBoneControlIn() = default;  // 默认构造函数，无参数，执行默认初始化

    explicit WalkerBoneControlIn( // 构造函数，接收一个骨骼变换数据的向量作为参数
        std::vector<rpc::BoneTransformDataIn> bone_transforms)
      : bone_transforms(bone_transforms) {} // 使用初始化列表来初始化成员变量

#ifdef LIBCARLA_INCLUDED_FROM_UE4 // 检查是否从UE4中包含

    operator FWalkerBoneControlIn() const { // 转换为UE4的FWalkerBoneControlIn类型
      FWalkerBoneControlIn Control; // 创建UE4的FWalkerBoneControlIn对象
      for (auto &bone_transform : bone_transforms) {  // 遍历每个骨骼变换数据
        Control.BoneTransforms.Add(ToFString(bone_transform.first), bone_transform.second); // 将骨骼变换数据添加到UE4对象中
      }
      return Control; // 返回转换后的UE4对象
    }

#endif // LIBCARLA_INCLUDED_FROM_UE4

    std::vector<rpc::BoneTransformDataIn> bone_transforms; // 成员变量，存储骨骼变换数据的向量

    MSGPACK_DEFINE_ARRAY(bone_transforms); // MsgPack序列化宏，定义如何序列化WalkerBoneControlIn对象
  };

} // namespace rpc
} // namespace carla
