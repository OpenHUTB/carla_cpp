// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once // 只包含一次的预处理指令

namespace carla { // 定义carla命名空间
namespace client { // 定义client子命名空间

  enum class GarbageCollectionPolicy { // 定义垃圾回收策略的枚举类
    Disabled, // 禁用垃圾回收
    Enabled,  // 启用垃圾回收
    Inherit   // 继承垃圾回收设置
  };

} // namespace client
} // namespace carla
