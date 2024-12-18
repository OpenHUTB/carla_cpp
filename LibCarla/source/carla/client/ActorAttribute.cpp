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
    if (GetType() == rpc::ActorAttributeType::Bool) {
      StringUtil::ToLower(value);
    }
	// 将传入的值赋给属性的内部存储，并移动该值。
    _attribute.value = std::move(value);
    Validate(); // 验证属性值的有效性。
  }

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
    switch (GetType()) {
      case rpc::ActorAttributeType::Bool:     As<rpc::ActorAttributeType::Bool>();     break;
      case rpc::ActorAttributeType::Int:      As<rpc::ActorAttributeType::Int>();      break;
      case rpc::ActorAttributeType::Float:    As<rpc::ActorAttributeType::Float>();    break;
      case rpc::ActorAttributeType::String:   As<rpc::ActorAttributeType::String>();   break;
      case rpc::ActorAttributeType::RGBColor: As<rpc::ActorAttributeType::RGBColor>(); break;
      default:
        LIBCARLA_THROW_INVALID_VALUE("invalid value type");
    }
  }

#undef LIBCARLA_THROW_BAD_VALUE_CAST
#undef LIBCARLA_THROW_INVALID_VALUE

} // namespace client
} // namespace carla
