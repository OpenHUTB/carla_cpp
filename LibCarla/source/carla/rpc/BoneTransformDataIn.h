// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// 此部分注释表明该代码的版权所属，归巴塞罗那自治大学的计算机视觉中心（CVC）所有，
// 并且其遵循MIT许可协议来授权使用，若要查看协议副本可访问对应网址。
// For a copy, see <https://opensource.org/licenses/MIT>.

// 头文件保护指令，作用是防止该头文件在同一个编译单元中被重复包含，避免出现重复定义等编译错误。
#pragma once

// 包含名为"carla/rpc/Transform.h"的头文件，
// 推测这里面应该定义了和变换（可能是空间变换之类的相关内容，比如坐标变换等）有关的类型、函数等，
// 后续代码或许会依赖其中的定义来进行相关操作。
#include "carla/rpc/Transform.h"

// 包含C++标准库中处理字符串相关操作的头文件，后续可能会用到字符串类型来存储、操作文本信息。
#include <string>
// 包含C++标准库中定义了pair（对）模板类型的头文件，pair常用于将两个不同类型的值组合在一起，方便统一处理。
#include <utility>

// 定义名为carla的命名空间，用于将和Carla项目相关的代码在逻辑上进行统一组织，
// 避免与其他项目代码产生命名冲突，使代码结构更清晰，便于维护和扩展。
namespace carla {
    // 在carla命名空间下再定义一个rpc子命名空间，
// 推测这个子命名空间是用来存放与远程过程调用（RPC）相关的各种类型定义、函数声明等内容的。
    namespace rpc {
        // 使用类型别名（using关键字实现，类似于typedef的功能），
        // 将std::pair<std::string, geom::Transform>这种类型组合重命名为BoneTransformDataIn。
        // 这里表示BoneTransformDataIn类型其实就是一个由字符串和某种几何变换（geom::Transform，类型应该来自之前包含的Transform.h头文件）组成的对，
        // 可能用于在相关代码中传递骨骼（Bone，推测可能是在图形学、动画等相关场景下的概念）变换相关的数据，
        // 其中字符串部分也许用于标识骨骼的名称之类的信息，而几何变换部分则代表该骨骼对应的空间变换情况。
        using BoneTransformDataIn = std::pair<std::string, geom::Transform>;
    } // namespace rpc
} // namespace carla
