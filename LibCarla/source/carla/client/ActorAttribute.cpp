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
// 定义一个宏，用于抛出属性类型转换错误异常，会检查当前属性的实际类型是否与期望转换的类型一致，
// 如果不一致，则拼接属性ID以及相应错误消息并抛出BadAttributeCast类型的异常，
// #type会被替换为实际传入的类型名称（在使用宏的地方）
#define LIBCARLA_THROW_INVALID_VALUE(message) throw_exception(InvalidAttributeValue(GetId() + ": " + message));
#define LIBCARLA_THROW_BAD_VALUE_CAST(type) \
    if (GetType() != rpc::ActorAttributeType:: type) { \
      throw_exception(BadAttributeCast(GetId() + ": bad attribute cast: cannot convert to " #type)); \
    }
// ActorAttribute 的成员函数 Set，用于设置属性值。
  void ActorAttribute::Set(std::string value) {
	   // 如果属性不可修改，则抛出无效值异常。
    if (!_attribute.is_modifiable) {
      LIBCARLA_THROW_INVALID_VALUE("read-only attribute");
    }
	// 如果属性类型是布尔类型，则将传入的值转换为小写。
	  // ActorAttribute 的成员函数 Set，用于设置属性值，它接受一个字符串类型的参数来设置对应属性的值
  void ActorAttribute::Set(std::string value) {
      // 如果属性不可修改（通过属性内部的标识判断），则抛出无效值异常，提示该属性是只读属性
    if (GetType() == rpc::ActorAttributeType::Bool) {
      StringUtil::ToLower(value);
    }
	// 将传入的值赋给属性的内部存储，并移动该值。
	  // 如果属性类型是布尔类型（通过与rpc定义的类型枚举对比判断），则将传入的字符串值转换为小写形式，
      // 可能是为了后续统一处理布尔值的比较等情况
    _attribute.value = std::move(value);
    Validate(); // 验证属性值的有效性。
  }

// 模板特化，用于将属性值转换为布尔类型，通过ActorAttributeValueAccess这个类型（应该是提供属性值访问相关功能的类型）来实现
// 模板特化，用于将属性值转换为布尔类型。
  template <>
  bool ActorAttributeValueAccess::As<bool>() const {
	// 如果属性类型不是布尔类型，则抛出类型转换错误异常。
    LIBCARLA_THROW_BAD_VALUE_CAST(Bool);
	// 将获取到的值转换为小写副本。
    auto value = StringUtil::ToLowerCopy(GetValue());
	// 如果值为 "true"，则返回 true；如果值为 "false"，则返回 false；否则抛出无效值异常。
    if (value == "true") {
      return true;
    } else if (value == "false") {
      return false;
    }
	   // 如果字符串值既不是 "true" 也不是 "false"，则抛出无效值异常，提示是无效的布尔值表示形式，并带上实际获取到的属性值字符串
    LIBCARLA_THROW_INVALID_VALUE("invalid bool: " + GetValue());
  }
// 模板特化，用于将属性值转换为整数类型。
  template<>
  int ActorAttributeValueAccess::As<int>() const {
	// 如果属性类型不是整数类型，则抛出类型转换错误异常。
    LIBCARLA_THROW_BAD_VALUE_CAST(Int);
	// 使用 std::atoi 将字符串转换为整数
    return std::atoi(GetValue().c_str());
  }
// 模板特化，用于将属性值转换为浮点数类型。
  template<>
  float ActorAttributeValueAccess::As<float>() const {
    LIBCARLA_THROW_BAD_VALUE_CAST(Float);
	// 使用 std::atof 将字符串转换为双精度浮点数。
    double x = std::atof(GetValue().c_str());
	// 如果转换后的双精度浮点数超出了单精度浮点数的范围，则抛出无效值异常。
    if ((x > std::numeric_limits<float>::max()) ||
        (x < std::numeric_limits<float>::lowest())) {
      LIBCARLA_THROW_INVALID_VALUE("float overflow");
    }
    return static_cast<float>(x);
  }

  template <>
  std::string ActorAttributeValueAccess::As<std::string>() const {
    LIBCARLA_THROW_BAD_VALUE_CAST(String);
    return GetValue();
  }
// 模板特化，用于将属性值转换为传感器数据颜色类型。
  template <>
  sensor::data::Color ActorAttributeValueAccess::As<sensor::data::Color>() const {
    LIBCARLA_THROW_BAD_VALUE_CAST(RGBColor);
// 将颜色字符串分割为三个通道的字符串向量。
    std::vector<std::string> channels;
    StringUtil::Split(channels, GetValue(), ",");
	// 如果颜色通道数量不是 3，则抛出无效值异常。
    if (channels.size() != 3u) {
      log_error("invalid color", GetValue());
      LIBCARLA_THROW_INVALID_VALUE("colors must have 3 channels (R,G,B)");
    }
// 定义一个 lambda 表达式，用于将字符串转换为整数，并检查是否溢出。
    auto to_int = [this](const std::string &str) {
      int i = std::atoi(str.c_str());
      if (i > std::numeric_limits<uint8_t>::max()) {
        LIBCARLA_THROW_INVALID_VALUE("integer overflow in color channel");
      }
      return static_cast<uint8_t>(i);
    };
 // 将三个通道的字符串转换为无符号 8 位整数，并返回颜色对象。
    return {to_int(channels[0u]), to_int(channels[1u]), to_int(channels[2u])};
  }
// 验证属性值的有效性，根据属性类型进行相应的类型转换并检查是否抛出异常。
  void ActorAttributeValueAccess::Validate() const {
    switch (GetType()) {// 当属性类型为布尔类型（rpc::ActorAttributeType::Bool）时，
        // 调用
      case rpc::ActorAttributeType::Bool:     As<rpc::ActorAttributeType::Bool>();     break;    // 当属性类型为整数类型（rpc::ActorAttributeType::Int）时，
        // 调用As<rpc::ActorAttributeType::Int>函数进行相应的验证或转换操作（具体验证逻辑在As函数的整数类型特化版本中实现）。
      case rpc::ActorAttributeType::Int:      As<rpc::ActorAttributeType::Int>();      break; // 当属性类型为浮点数类型（rpc::ActorAttributeType::Float）时，
        // 调用As<rpc::ActorAttributeType::Float>函数进行相应的验证或转换操作（具体验证逻辑在As函数的浮点数类型特化版本中实现）。
      case rpc::ActorAttributeType::Float:    As<rpc::ActorAttributeType::Float>();    break; // 当属性类型为字符串类型（rpc::ActorAttributeType::String）时，
        // 调用As<rpc::ActorAttributeType::String>函数进行相应的验证或转换操作（具体验证逻辑在As函数的字符串类型特化版本中实现）。
      case rpc::ActorAttributeType::String:   As<rpc::ActorAttributeType::String>();   break;  // 当属性类型为RGBColor类型（rpc::ActorAttributeType::RGBColor）时，
        // 调用
      case rpc::ActorAttributeType::RGBColor: As<rpc::ActorAttributeType::RGBColor>(); break;  // 如果属性类型不属于上述已经列举的任何一种有效类型，
        // 则抛出无效值异常，提示当前属性具有无效的类型值。
      default:
        LIBCARLA_THROW_INVALID_VALUE("invalid value type");
    }
  }
// 取消之前定义的宏
#undef LIBCARLA_THROW_BAD_VALUE_CAST// 取消之前定义的宏
#undef LIBCARLA_THROW_INVALID_VALUE// 同理，避免后续出现同名宏定义相关的潜在问题，使其恢复到未定义该宏的状态。

} // namespace client// 结束carla命名空间的定义，对应前面的namespace carla { ，表示整个carla命名空间内的代码块结束。
} // namespace carla
