// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.
// 引入头文件，包含了 ActorAttribute 的定义。
#include "carla/client/ActorAttribute.h"
// 引入其他必要的头文件。
#include "carla/Exception.h"
#include "carla/Logging.h"
#include "carla/StringUtil.h"

// 使用命名空间，避免命名冲突。
namespace carla {
namespace client {

// 定义一个宏，用于方便地抛出表示属性值无效的异常。
// 它会调用 throw_exception 函数（应该在相关异常处理模块定义）抛出 InvalidAttributeValue 类型的异常，
// 异常信息包含属性的标识（通过 GetId() 获取）以及传入的具体错误提示消息。
#define LIBCARLA_THROW_INVALID_VALUE(message) throw_exception(InvalidAttributeValue(GetId() + ": " + message));

// 定义一个宏，用于在属性值类型转换时进行类型检查并抛出异常。
// 如果当前属性的类型（通过 GetType() 获取）与期望转换的类型（由参数 type 指定）不一致，
// 就会抛出 BadAttributeCast 类型的异常，异常信息包含属性标识以及说明不能转换到指定类型的提示内容。
#define LIBCARLA_THROW_BAD_VALUE_CAST(type) \
    if (GetType()!= rpc::ActorAttributeType:: type) { \
      throw_exception(BadAttributeCast(GetId() + ": bad attribute cast: cannot convert to " #type)); \
    }

// ActorAttribute 的成员函数 Set，用于设置属性值。
  void ActorAttribute::Set(std::string value) {
      // _attribute 应该是 ActorAttribute 类内部用于管理属性相关信息的结构体或成员对象等。
      // 这里判断属性是否可修改，如果不可修改（_attribute.is_modifiable 为 false），
      // 意味着该属性是只读的，此时就通过宏抛出属性值无效的异常，提示是只读属性不能设置值。
      if (!_attribute.is_modifiable) {
          LIBCARLA_THROW_INVALID_VALUE("read-only attribute");
      }
      // 如果属性类型是布尔类型（通过 GetType() 判断），
      // 则调用 StringUtil 命名空间下的 ToLower 函数将传入的表示布尔值的字符串转换为小写形式，
      // 方便后续统一格式进行布尔值的判断和处理。
      if (GetType() == rpc::ActorAttributeType::Bool) {
          StringUtil::ToLower(value);
      }
      // 使用 std::move 进行移动语义赋值，将传入的 value 字符串高效地转移资源所有权给 _attribute.value，
      // 完成对属性值的设置，避免不必要的拷贝开销。
      _attribute.value = std::move(value);
      // 调用 Validate 函数（应该是对刚设置的属性值进行合法性、有效性验证的函数）来确保设置的属性值符合要求。
      // 如果验证过程中发现不符合要求的情况，会根据具体验证逻辑抛出相应的异常。
      Validate(); 
  }

// 模板特化，用于将属性值转换为布尔类型。
  template <>
  bool ActorAttributeValueAccess::As<bool>() const {
      // 首先进行类型检查，确保当前属性的类型是布尔类型，如果不是就通过宏抛出类型转换错误的异常，
      // 提示不能转换到布尔类型。
      LIBCARLA_THROW_BAD_VALUE_CAST(Bool);
      // 调用 StringUtil 命名空间下的 ToLowerCopy 函数，获取属性值对应的小写形式字符串副本，
      // 避免修改原字符串，用于后续准确判断是否为有效的布尔值表示形式。
      auto value = StringUtil::ToLowerCopy(GetValue());
      // 如果获取到的小写字符串值为 "true"，则返回 true，表示解析为布尔真。
      if (value == "true") {
          return true;
      } 
      // 如果获取到的小写字符串值为 "false"，则返回 false，表示解析为布尔假。
      else if (value == "false") {
          return false;
      }
      // 如果获取到的字符串既不是 "true" 也不是 "false"，则说明不是有效的布尔值表示形式，
      // 通过宏抛出属性值无效的异常，异常信息包含提示是无效的布尔值以及具体传入的属性值内容。
      LIBCARLA_THROW_INVALID_VALUE("invalid bool: " + GetValue());
  }

// 模板特化，用于将属性值转换为整数类型。
  template<>
  int ActorAttributeValueAccess::As<int>() const {
      // 先进行类型检查，确保当前属性的类型是整数类型，如果不是就通过宏抛出类型转换错误的异常，
      // 提示不能转换到整数类型。
      LIBCARLA_THROW_BAD_VALUE_CAST(Int);
      // 使用 std::atoi 函数将表示整数的字符串（通过 GetValue() 获取属性值对应的字符串）转换为整数类型返回。
      // std::atoi 会尝试解析字符串开头的连续数字字符部分为整数，如果字符串无法正确解析为整数可能会导致未定义行为，
      // 但在这里应该是期望属性值对应的字符串是合法的整数表示形式。
      return std::atoi(GetValue().c_str());
  }

// 模板特化，用于将属性值转换为浮点数类型。
  template<>
  float ActorAttributeValueAccess::As<float>() const {
      // 首先进行类型检查，确保当前属性的类型是浮点数类型，如果不是就通过宏抛出类型转换错误的异常，
      // 提示不能转换到浮点数类型。
      LIBCARLA_THROW_BAD_VALUE_CAST(Float);
      // 使用 std::atof 函数将表示浮点数的字符串（通过 GetValue() 获取属性值对应的字符串）转换为双精度浮点数类型。
      double x = std::atof(GetValue().c_str());
      // 检查转换后的双精度浮点数是否超出了单精度浮点数的表示范围（最大值和最小值），
      // 如果超出范围，就通过宏抛出属性值无效的异常，提示浮点数溢出。
      if ((x > std::numeric_limits<float>::max()) ||
          (x < std::numeric_limits<float>::lowest())) {
          LIBCARLA_THROW_INVALID_VALUE("float overflow");
      }
      // 将双精度浮点数通过 static_cast 强制转换为单精度浮点数并返回，完成属性值到浮点数类型的转换。
      return static_cast<float>(x);
  }

  template <>
  std::string ActorAttributeValueAccess::As<std::string>() const {
      // 进行类型检查，确保当前属性的类型是字符串类型，如果不是就通过宏抛出类型转换错误的异常，
      // 提示不能转换到字符串类型。
      LIBCARLA_THROW_BAD_VALUE_CAST(String);
      // 如果属性类型是字符串类型，直接返回通过 GetValue() 获取到的属性值对应的字符串，完成转换操作。
      return GetValue();
  }

// 模板特化，用于将属性值转换为传感器数据颜色类型。
  template <>
  sensor::data::Color ActorAttributeValueAccess::As<sensor::data::Color>() const {
      // 先进行类型检查，确保当前属性的类型是 RGBColor（传感器数据颜色相关类型，这里应该是对应颜色表示的一种类型定义）类型，
      // 如果不是就通过宏抛出类型转换错误的异常，提示不能转换到该颜色类型。
      LIBCARLA_THROW_BAD_VALUE_CAST(RGBColor);
      // 定义一个 std::vector<std::string> 类型的容器 channels，用于存储分割后的颜色通道字符串。
      std::vector<std::string> channels;
      // 调用 StringUtil 命名空间下的 Split 函数，将属性值对应的字符串（通过 GetValue() 获取）按照逗号分隔符进行分割，
      // 分割后的各个子字符串分别代表颜色的红、绿、蓝三个通道值，存储在 channels 容器中。
      StringUtil::Split(channels, GetValue(), ",");
      // 检查分割后得到的颜色通道数量是否为 3 个，如果不是则说明颜色表示格式不正确，
      // 通过日志记录函数（log_error，应该在 carla/Logging.h 中定义）记录错误信息，
      // 并通过宏抛出属性值无效的异常，提示颜色必须有 3 个通道（即 R、G、B 三个通道）。
      if (channels.size()!= 3u) {
          log_error("invalid color", GetValue());
          LIBCARLA_THROW_INVALID_VALUE("colors must have 3 channels (R,G,B)");
      }
      // 定义一个 lambda 表达式，用于将表示颜色通道的字符串转换为无符号 8 位整数（uint8_t）类型。
      // 先使用 std::atoi 将字符串转换为整数，然后检查转换后的整数是否超出了 uint8_t 类型能表示的最大值范围，
      // 如果超出范围就通过宏抛出属性值无效的异常，提示颜色通道整数溢出，
      // 如果在范围内则将整数强制转换为 uint8_t 类型并返回。
      auto to_int = [this](const std::string &str) {
          int i = std::atoi(str.c_str());
          if (i > std::numeric_limits<uint8_t>::max()) {
              LIBCARLA_THROW_INVALID_VALUE("integer overflow in color channel");
          }
          return static_cast<uint8_t>(i);
      };
      // 使用定义好的 lambda 表达式将三个通道的字符串分别转换为无符号 8 位整数，
      // 然后构造并返回一个 sensor::data::Color 类型的对象（代表颜色值），其参数分别为三个通道对应的整数值。
      return {to_int(channels[0u]), to_int(channels[1u]), to_int(channels[2u])};
  }

// 验证属性值的有效性，根据属性类型进行相应的类型转换并检查是否抛出异常。
  void ActorAttributeValueAccess::Validate() const {
      // 根据属性的类型（通过 GetType() 获取）使用 switch 语句进行不同情况的处理。
      switch (GetType()) {
          // 如果属性类型是布尔类型，调用 As<rpc::ActorAttributeType::Bool>() 函数进行布尔类型的转换验证，
          // 如果转换过程中出现问题（比如类型不对、值格式不对等）会抛出相应的异常，这里只是调用进行验证流程。
          case rpc::ActorAttributeType::Bool:     As<rpc::ActorAttributeType::Bool>();     break;
          // 如果属性类型是整数类型，调用 As<rpc::ActorAttributeType::Int>() 函数进行整数类型的转换验证。
          case rpc::ActorAttributeType::Int:      As<rpc::ActorAttributeType::Int>();      break;
          // 如果属性类型是浮点数类型，调用 As<rpc::ActorAttributeType::Float>() 函数进行浮点数类型的转换验证。
          case rpc::ActorAttributeType::Float:    As<rpc::ActorAttributeType::Float>();    break;
          // 如果属性类型是字符串类型，调用 As<rpc::ActorAttributeType::String>() 函数进行字符串类型的转换验证。
          case rpc::ActorAttributeType::String:   As<rpc::ActorAttributeType::String>();   break;
          // 如果属性类型是 RGBColor（传感器数据颜色类型），调用 As<rpc::ActorAttributeType::RGBColor>() 函数进行颜色类型的转换验证。
          case rpc::ActorAttributeType::RGBColor: As<rpc::ActorAttributeType::RGBColor>(); break;
          // 如果属性类型不属于上述任何一种已知的合法类型，就通过宏抛出属性值无效的异常，提示是无效的值类型。
          default:
              LIBCARLA_THROW_INVALID_VALUE("invalid value type");
      }
  }

#undef LIBCARLA_THROW_BAD_VALUE_CAST
#undef LIBCARLA_THROW_INVALID_VALUE

} // namespace client
} // namespace carla
