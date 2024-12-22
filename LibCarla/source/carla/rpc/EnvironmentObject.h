// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/MsgPack.h"

#include <cstdint>

#include "carla/geom/BoundingBox.h"
#include "carla/rpc/Transform.h"
#include "carla/rpc/ObjectLabel.h"

namespace carla {
namespace rpc {

  // Name is under discussion
  // 定义了一个名为 `EnvironmentObject` 的结构体，从名字和其成员变量来看，它用于描述环境中的某个对象的相关属性信息，不过结构体的名称可能还在讨论中，也许后续会根据实际情况进行调整更名等操作。
  struct EnvironmentObject {
    // 定义一个 `Transform` 类型的成员变量 `transform`，用于存储该环境对象在空间中的坐标变换信息，比如它在场景中的位置、朝向等情况，方便后续进行基于位置和姿态的各种计算、渲染或者交互操作。
    Transform transform;
    geom::BoundingBox bounding_box;
    // 定义一个 `geom::BoundingBox` 类型的成员变量 `bounding_box`，用于表示该环境对象在空间中的包围盒信息，通过包围盒可以直观地界定对象所占据的空间范围，在很多场景中（如碰撞检测判断该对象是否与其他对象相交、渲染时确定是否在视野范围内等）都有重要作用。
    uint64_t id = 0;
    // 定义一个 `uint64_t` 类型的成员变量 `id`，并初始化为 0，用于作为该环境对象的唯一标识符，在整个系统中通过这个唯一的 `id` 可以区分不同的环境对象，便于查找、引用以及进行对象间关系管理等操作，初始值 0 可能只是一个默认的占位值，实际使用中会赋予其真正有意义的唯一标识值。
    std::string name;
    // 定义一个 `std::string` 类型的成员变量 `name`，用于存储该环境对象的名称信息，比如可以是具体的名字“路灯 001”“建筑物 A”等，方便在程序中通过名称来直观地识别和操作对象，名称的具体取值根据对象的实际定义和业务逻辑来设定。
    CityObjectLabel type = CityObjectLabel::None;
    // 定义一个 `CityObjectLabel` 类型的成员变量 `type`，并初始化为 `CityObjectLabel::None`，用于标记该环境对象所属的类别标签，通过这个标签可以将环境对象分类，例如是属于车辆、行人、路灯等不同类型，方便进行批量处理同类型对象、按类型进行不同逻辑操作等情况，初始值 `None` 表示未指定具体类型的默认状态。
#ifdef LIBCARLA_INCLUDED_   
   // 这是一个条件编译指令块，判断是否是从 UE4 中包含进来的这个头文件（通过定义的宏 `LIBCARLA_INCLUDED_FROM_UE4` 来判断），如果是从 UE4 引入的话，就执行下面的代码，用于实现从 UE4 中的 `FEnvironmentObject` 类型到当前 `EnvironmentObject` 结构体的转换构造函数，方便在与 UE4 相关的项目集成或者交互时进行数据类型的适配。
FROM_UE4

    EnvironmentObject(const FEnvironmentObject &EnvironmentObject)
      : transform(EnvironmentObject.Transform),
        bounding_box(EnvironmentObject.BoundingBox),
        id(EnvironmentObject.Id),
        name(TCHAR_TO_UTF8(*EnvironmentObject.Name)),
        type(EnvironmentObject.ObjectLabel) {}

#endif // LIBCARLA_INCLUDED_FROM_UE4

    MSGPACK_DEFINE_ARRAY(transform, bounding_box, id, name, type);
    // 使用 `MSGPACK_DEFINE_ARRAY` 宏（来自于之前引入的 `carla/MsgPack.h` 文件相关的序列化功能）来定义如何将这个 `EnvironmentObject` 结构体对象序列化为数组形式。这里指定了按照 `transform`、`bounding_box`、`id`、`name`、`type` 的顺序将成员变量进行序列化，方便在网络传输、存储等场景下对该结构体对象进行相应的处理，使得对象可以方便地在不同地方进行恢复和使用，例如在不同的进程、模块或者系统之间传递这个环境对象的相关信息时，可以先将其序列化后发送，接收方再通过反序列化还原出原始的结构体对象进行后续操作。
  };

} // namespace rpc
} // namespace carla

