// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

// 防止头文件重复包含
#pragma once 

// 引入Carla的MsgPack相关头文件，用于序列化等操作
#include "carla/MsgPack.h"
// 引入ActorAttributeType相关头文件，可能定义了属性类型的枚举等内容
#include "carla/rpc/ActorAttributeType.h"
// 引入String相关头文件，可能用于处理字符串相关操作
#include "carla/rpc/String.h"

// 引入标准库的vector容器头文件，用于存储一组元素，比如字符串列表等
#include <vector>

// 将carla::rpc::ActorAttributeType枚举类型添加到MsgPack序列化机制中，以便可以正确地对其进行序列化和反序列化操作
MSGPACK_ADD_ENUM(carla::rpc::ActorAttributeType);

// 如果是从UE4中包含此头文件（定义了相关宏来判断这种情况）
#ifdef LIBCARLA_INCLUDED_FROM_UE4
// 引入UE4相关宏启用的头文件，可能是为了适配UE4环境做一些前置处理
#include <compiler/enable-ue4-macros.h>
// 引入ActorAttribute相关头文件，可能是UE4中对Actor属性定义的相关类
#include "Carla/Actor/ActorAttribute.h"
// 引入UE4相关宏禁用的头文件，结束UE4相关特定处理
#include <compiler/disable-ue4-macros.h>
#endif // LIBCARLA_INCLUDED_FROM_UE4

// 定义在carla命名空间下的rpc子命名空间，用于组织相关的RPC（远程过程调用）相关的类和类型等
namespace carla {
namespace rpc {

  // ActorAttribute类，用于表示Actor的属性相关信息
  class ActorAttribute {
  public:
    // 默认构造函数，使用编译器自动生成的默认行为
    ActorAttribute() = default;

    // 表示属性的唯一标识符，通常是一个字符串，用于区分不同的属性
    std::string id;

    // 属性的类型，通过ActorAttributeType枚举来指定，初始化为ActorAttributeType::Int类型
    ActorAttributeType type = ActorAttributeType::Int;

    // 属性的值，以字符串形式存储，具体的格式可能根据type来解析不同含义
    std::string value;

    // 推荐的属性值列表，比如对于某些属性可能有一组限定的可选值，就存储在这里
    std::vector<std::string> recommended_values;

    // 表示该属性是否可修改，初始值为true，表示默认是可修改的
    bool is_modifiable = true;

    // 是否限制属性值只能从推荐的值中选取，初始值为false，表示不限制
    bool restrict_to_recommended = false;

#ifdef LIBCARLA_INCLUDED_FROM_UE4
    // 从UE4中的FActorAttribute类型构造ActorAttribute对象的转换构造函数
    // 用于将UE4中的相关属性结构转换为本类的对象，方便在不同环境下统一处理属性信息
    ActorAttribute(const FActorAttribute &Attribute)
      : id(FromFString(Attribute.Id)),
        type(static_cast<ActorAttributeType>(Attribute.Type)),
        value(FromFString(Attribute.Value)),
        is_modifiable(false) {}

    // 从UE4中的FActorVariation类型构造ActorAttribute对象的转换构造函数
    // 同样是进行不同环境下属性相关结构的转换，根据FActorVariation中的信息来初始化本类对象的各个成员
    ActorAttribute(const FActorVariation &Variation)
      : id(FromFString(Variation.Id)),
        type(static_cast<ActorAttributeType>(Variation.Type)),
        is_modifiable(true),
        restrict_to_recommended(Variation.bRestrictToRecommended) {
      // 预留足够的空间，避免在后续插入元素时频繁重新分配内存，提高效率
      recommended_values.reserve(Variation.RecommendedValues.Num());
      for (const auto &Item : Variation.RecommendedValues) {
        recommended_values.push_back(FromFString(Item));
      }
      if (!recommended_values.empty()) {
        value = recommended_values[0u];
      }
    }
#endif // LIBCARLA_INCLUDED_FROM_UE4

    // 使用MsgPack的宏来定义如何对ActorAttribute类的对象进行序列化和反序列化操作
    // 指定了按照id、type、value、recommended_values、is_modifiable、restrict_to_recommended这些成员的顺序进行处理
    MSGPACK_DEFINE_ARRAY(id, type, value, recommended_values, is_modifiable, restrict_to_recommended);
  };

  // ActorAttributeValue类，用于表示Actor属性的值相关信息（可能是简化版，只关注关键的属性标识、类型和值）
  class ActorAttributeValue {
  public:
    // 默认构造函数，使用编译器自动生成的默认行为
    ActorAttributeValue() = default;

    // 从ActorAttribute对象构造ActorAttributeValue对象的构造函数
    // 用于提取ActorAttribute中的关键信息（id、type、value）来初始化本对象
    ActorAttributeValue(const ActorAttribute &attribute)
      : id(attribute.id),
        type(attribute.type),
        value(attribute.value) {}

    // 表示属性的唯一标识符，与ActorAttribute中的id对应，用于区分不同的属性
    std::string id;

    // 属性的类型，与ActorAttribute中的type对应，通过ActorAttributeType枚举来指定，初始化为ActorAttributeType::Int类型
    ActorAttributeType type = ActorAttributeType::Int;

    // 属性的值，与ActorAttribute中的value对应，以字符串形式存储，具体的格式可能根据type来解析不同含义
    std::string value;

#ifdef LIBCARLA_INCLUDED_FROM_UE4
    // 从UE4中的FActorAttribute类型构造ActorAttributeValue对象的转换构造函数
    // 借助ActorAttribute的转换构造函数来实现从UE4相关类型到本类对象的转换
    ActorAttributeValue(const FActorAttribute &Attribute)
      : ActorAttributeValue(ActorAttribute(Attribute)) {}

    // 类型转换运算符重载，用于将ActorAttributeValue对象转换为UE4中的FActorAttribute类型
    // 方便在需要的地方将本类对象作为UE4相关类型来使用，实现不同环境下类型的兼容
    operator FActorAttribute() const {
      FActorAttribute Attribute;
      Attribute.Id = ToFString(id);
      Attribute.Type = static_cast<EActorAttributeType>(type);
      Attribute.Value = ToFString(value);
      return Attribute;
    }
#endif // LIBCARLA_INCLUDED_FROM_UE4

    // 使用MsgPack的宏来定义如何对ActorAttributeValue类的对象进行序列化和反序列化操作
    // 指定了按照id、type、value这些成员的顺序进行处理
    MSGPACK_DEFINE_ARRAY(id, type, value);
  };

} // namespace rpc
} // namespace carla
