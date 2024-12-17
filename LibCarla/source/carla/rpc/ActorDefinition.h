// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

// 防止头文件被重复包含，这是一种常见的C/C++编程中的预处理指令用法
#pragma once 

// 引入Carla的MsgPack相关头文件，MsgPack通常用于对象的序列化与反序列化操作，方便数据存储、传输等
#include "carla/MsgPack.h"
// 引入ActorAttribute相关头文件，可能定义了与Actor属性相关的类、结构体或者枚举等内容，用于描述Actor的各种特性
#include "carla/rpc/ActorAttribute.h"
// 引入ActorId相关头文件，大概率是用于标识Actor的唯一标识符相关的数据类型或结构定义
#include "carla/rpc/ActorId.h"
// 引入String相关头文件，应该是提供了对字符串处理相关的功能、类型等支持
#include "carla/rpc/String.h"

// 引入标准库的vector容器头文件，用于存储一组同类型的元素，在这里用于存放Actor的相关属性等内容
#include <vector>

// 定义在carla命名空间下的rpc子命名空间，用于组织与远程过程调用（RPC）相关的各类定义，使得代码结构更清晰，便于模块化管理
namespace carla {
namespace rpc {

  // ActorDefinition类，用于定义Actor（可能是游戏、模拟场景等中的角色、实体等概念）的相关描述信息
  class ActorDefinition {
  public:
    // 默认构造函数，使用编译器自动生成的默认行为，即对类中的成员进行默认初始化
    ActorDefinition() = default;

    // Actor的唯一标识符，类型为ActorId，初始化为0u（具体含义可能根据ActorId类型的定义来确定，可能是无符号整数等形式的一个标识值）
    ActorId uid = 0u;

    // 用于表示Actor的一个通用标识符，通常是一个字符串，可能是在系统内具有唯一性或者特定语义的名字等，方便对Actor进行查找、区分等操作
    std::string id;

    // 用于存储与Actor相关的标签信息，也是以字符串形式存在，可能包含多个以特定分隔符隔开的标签，用于对Actor进行分类、筛选等用途
    std::string tags;

    // 存储Actor的一系列属性，每个属性由ActorAttribute类型表示，通过vector容器可以容纳多个属性，完整地描述Actor的各种特性
    std::vector<ActorAttribute> attributes;

#ifdef LIBCARLA_INCLUDED_FROM_UE4
    // 从UE4中的FActorDefinition类型构造ActorDefinition对象的转换构造函数
    // 目的是在与UE4环境交互时，能够将UE4中定义的Actor相关描述结构转换为当前代码环境下的ActorDefinition对象，方便统一处理
    ActorDefinition(const FActorDefinition &Definition)
      : uid(Definition.UId),
        id(FromFString(Definition.Id)),
        tags(FromFString(Definition.Tags)) {
      // 预留足够的空间，避免后续往attributes向量中添加元素时频繁进行内存重新分配操作，提高程序运行效率
      attributes.reserve(Definition.Variations.Num() + Definition.Attributes.Num());
      // 遍历UE4中FActorDefinition结构里的Variations成员（具体类型应该是包含ActorAttribute类型元素的某种容器），将其中的每个元素添加到当前对象的attributes向量中
      for (const auto &Item : Definition.Variations) {
        attributes.push_back(Item);
      }
      // 同样地，遍历UE4中FActorDefinition结构里的Attributes成员（也是包含ActorAttribute类型元素的容器），把元素添加到当前对象的attributes向量中，这样就整合了所有相关属性信息
      for (const auto &Item : Definition.Attributes) {
        attributes.push_back(Item);
      }
    }
#endif // LIBCARLA_INCLUDED_FROM_UE4

    // 使用MsgPack的宏来定义如何对ActorDefinition类的对象进行序列化和反序列化操作
    // 按照uid、id、tags、attributes这些成员的顺序来指定处理方式，方便将ActorDefinition对象转换为适合存储或传输的数据格式，以及从相应格式还原对象
    MSGPACK_DEFINE_ARRAY(uid, id, tags, attributes);
  };

} // namespace rpc
} // namespace carla
