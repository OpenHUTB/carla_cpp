// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

// 该指令确保头文件只会被编译器包含一次，防止重复包含导致的编译错误
#pragma once

// 包含项目中定义的转换类的头文件
#include "carla/rpc/Transform.h"

// 包含标准库中的 string和 utility
// string用于存储骨骼名称
#include <string>
#include <utility>

namespace carla {
namespace rpc {

  // using BoneTransformDataOut = std::pair<std::string, geom::Transform>;
  class BoneTransformDataOut
  {
    public:
    // 成员变量
    std::string bone_name;// 骨骼的名称。
    Transform world;// 骨骼在世界空间中的变换。
    Transform component;// 骨骼在组件空间中的变换。
    Transform relative;// 骨骼的相对变换。
  
    /**
     * @brief 比较运算符：判断两个 BoneTransformDataOut 对象是否不相等。
     * @param rhs 另一个 BoneTransformDataOut 对象。
     * @return true 如果两个对象的成员变量不完全相等；否则返回 false。
     */
    bool operator!=(const BoneTransformDataOut &rhs) const {
      return
      bone_name != rhs.bone_name ||
      world != rhs.world ||
      component != rhs.component ||
      relative != rhs.relative;
    }

    /**
     * @brief 比较运算符：判断两个 BoneTransformDataOut 对象是否相等。
     * @param rhs 另一个 BoneTransformDataOut 对象。
     * @return true 如果两个对象的所有成员变量完全相等；否则返回 false。
     */
    bool operator==(const BoneTransformDataOut &rhs) const {
      return !(*this != rhs);// 使用 !=运算符的结果进行判断。
    }

    /**
     * @brief 序列化宏，用于将成员变量打包成数组形式。
     * @note 该宏是 msgpack 库提供的，便于对象的序列化和反序列化。
     */
    MSGPACK_DEFINE_ARRAY(bone_name, world, component, relative);
  };

} // namespace rpc
} // namespace carla
