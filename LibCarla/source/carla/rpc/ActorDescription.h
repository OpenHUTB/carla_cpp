// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
// 防止头文件重复包含的编译指令。
#include "carla/MsgPack.h"
#include "carla/rpc/ActorAttribute.h"
#include "carla/rpc/ActorId.h"
#include "carla/rpc/String.h"
// 引入Carla项目中的相关头文件，提供消息打包、Actor属性、Actor ID以及字符串相关的支持。

#include <vector>
//标题库的头文件，用于处理动态信息组。
#ifdef LIBCARLA_INCLUDED_FROM_UE4               // 如果是从UE4引入的代码块，则启用UE4相关的宏。
#include <compiler/enable-ue4-macros.h>
#include "Carla/Actor/ActorDescription.h"
#include <compiler/disable-ue4-macros.h>
#endif // LIBCARLA_INCLUDED_FROM_UE4
// 用于在UE4环境中加载特定的头文件，并切换特定的编译宏。
namespace carla {
namespace rpc {

  class ActorDescription {
  public:

    ActorDescription() = default;              // 默认构造函数。

    ActorId uid = 0u;                          // 定义Actor的唯一标识符，初始值为0。

    std::string id;                             // Actor的类型标识符，例如“车辆”或“行人”。

    std::vector<ActorAttributeValue> attributes;// Actor的属性列表，例如颜色、车型等。


#ifdef LIBCARLA_INCLUDED_FROM_UE4

    ActorDescription(const FActorDescription &Description)
      : uid(Description.UId),
        id(FromFString(Description.Id)) {
      // 从UE4的FActorDescription结构体转换为当前类的构造函数。
      // 将UE4的字符串类型转换为标准字符串，并初始化`uid`和`id`。
      attributes.reserve(Description.Variations.Num());  // 预留空间存储所有属性。
      for (const auto &Item : Description.Variations) {
        attributes.emplace_back(Item.Value);
      }
        // 遍历FActorDescription的Variations，将其转换并存储到attributes中。
    }

    operator FActorDescription() const {                // 将当前类转换为UE4的FActorDescription类型的操作符重载。
      FActorDescription Description;
      Description.UId = uid;                            // 设置UID。
      Description.Id = ToFString(id);                   // 将标准字符串转换为UE4字符串类型。
      Description.Variations.Reserve(attributes.size());// 为Variations分配内存。
      for (const auto &item : attributes) {
        Description.Variations.Emplace(ToFString(item.id), item);// 将当前类的属性逐一添加到UE4结构体的Variations中。
      }
      return Description;
    }

#endif // LIBCARLA_INCLUDED_FROM_UE4

    MSGPACK_DEFINE_ARRAY(uid, id, attributes);
  // 定义用于序列化和反序列化的MsgPack宏，将uid、id和attributes打包为数组形式。
  };

} // namespace rpc
} // namespace carla
