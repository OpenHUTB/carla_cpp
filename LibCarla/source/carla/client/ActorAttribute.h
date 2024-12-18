// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
// 确保此头文件只被包含一次

#include "carla/rpc/ActorAttribute.h"
// 包含与Carla RPC（远程过程调用）中的参与者属性相关的头文件
#include "carla/sensor/data/Color.h"
// 包含与Carla传感器数据中的颜色相关的头文件

#include <cstdlib>
// 包含C标准库中的通用工具头文件，如一些宏定义、内存操作函数等
#include <exception>
// 包含C++ 异常处理相关的头文件
#include <type_traits>
// 包含C++ 类型特征相关的头文件，用于在编译时获取类型信息


namespace carla {
namespace client {


  // ===========================================================================
  // -- 无效属性值InvalidAttributeValue ----------------------------------------
  // ===========================================================================


  /// 当赋予ActorAttribute的值无法转换为其类型时抛出异常。
  class InvalidAttributeValue : public std::invalid_argument {
  public:
    InvalidAttributeValue(const std::string &what) : std::invalid_argument(what) {}
    // 构造函数，接受一个字符串参数what
    // 调用std::invalid_argument的构造函数，将what传递给父类构造函数
    // 用于创建一个带有特定错误信息的InvalidAttributeValue异常对象
  };


  // ===========================================================================
  // -- 错误的属性转换异常BadAttributeCast --------------------------------------
  // ===========================================================================


  /// 当ActorAttribute的值无法转换为请求的类型时抛出异常。
  class BadAttributeCast : public std::logic_error {
  public:
    BadAttributeCast(const std::string &what) : std::logic_error(what) {}
    // 构造函数，接受一个字符串参数what
    // 调用std::logic_error的构造函数，将what传递给父类构造函数
    // 用于创建一个带有特定错误信息的BadAttributeCast异常对象
  };


  // ===========================================================================
  // -- 参与者属性 --------------------------------------------------------------
  // ===========================================================================


  class ActorAttributeValueAccess
  {
  public:
    ActorAttributeValueAccess() = default;
    // 默认构造函数，使用默认初始化方式初始化对象
    ActorAttributeValueAccess(ActorAttributeValueAccess const &) = default;
    // 拷贝构造函数，使用默认拷贝语义
    ActorAttributeValueAccess(ActorAttributeValueAccess &&) = default;
    // 移动构造函数，使用默认移动语义
    virtual ~ActorAttributeValueAccess() = default;
    // 析构函数，使用默认析构语义

    ActorAttributeValueAccess & operator= (ActorAttributeValueAccess const & ) = default;
    // 拷贝赋值运算符，使用默认拷贝语义
    ActorAttributeValueAccess & operator= (ActorAttributeValueAccess && ) = default;
    // 移动赋值运算符，使用默认移动语义

    virtual const std::string &GetId() const = 0;
    // 纯虚函数，用于获取属性的唯一标识符，派生类必须实现
    virtual rpc::ActorAttributeType GetType() const = 0;
    // 纯虚函数，用于获取属性的类型（通过rpc::ActorAttributeType枚举），派生类必须实现

    /// 将值转换为给定的类型。
    ///
    /// @throw 如果转换失败跑出BadAttributeCast异常。
    template <typename T>
    T As() const;
    // 模板函数，用于将属性值转换为类型T，在函数体中会根据具体类型进行转换，如果失败会抛出BadAttributeCast异常

    /// 将值转换为枚举carla::rpc::ActorAttributeType指定的类型。
    ///
    /// @throw 如果转换失败，则抛出BadAttributeCast的异常。
    template <rpc::ActorAttributeType Type>
    auto As() const;
    // 模板函数，根据指定的rpc::ActorAttributeType枚举类型将属性值转换为相应类型，如果失败会抛出BadAttributeCast异常

    template <typename T>
    bool operator==(const T &rhs) const;
    // 模板函数，用于比较当前对象与类型为T的对象rhs是否相等，具体比较逻辑在函数体中实现

    template <typename T>
    bool operator!=(const T &rhs) const {
      return!(*this == rhs);
    }
    // 不等于运算符，通过调用==运算符取反得到不等于的结果


  protected:
    virtual const std::string &GetValue() const = 0;
    // 纯虚函数，用于获取属性的值，派生类必须实现
    void Validate() const;
    // 函数用于验证属性的合法性，具体验证逻辑在派生类中实现
  };


  template <>
  bool ActorAttributeValueAccess::As<bool>() const;
  // 显式特化模板函数As，用于将属性值转换为bool类型，具体实现在其他地方（可能在源文件中）

  template <>
  int ActorAttributeValueAccess::As<int>() const;
  // 显式特化模板函数As，用于将属性值转换为int类型，具体实现在其他地方（可能在源文件中）

  template <>
  float ActorAttributeValueAccess::As<float>() const;
  // 显式特化模板函数As，用于将属性值转换为float类型，具体实现在其他地方（可能在源文件中）

  template <>
  std::string ActorAttributeValueAccess::As<std::string>() const;
  // 显式特化模板函数As，用于将属性值转换为std::string类型，具体实现在其他地方（可能在源文件中）

  template <>
  sensor::data::Color ActorAttributeValueAccess::As<sensor::data::Color>() const;
  // 显式特化模板函数As，用于将属性值转换为sensor::data::Color类型，具体实现在其他地方（可能在源文件中）


  template <>
  inline auto ActorAttributeValueAccess::As<rpc::ActorAttributeType::Bool>() const {
    return As<bool>();
  }
  // 显式特化模板函数As，当指定类型为rpc::ActorAttributeType::Bool时，调用As<bool>()进行转换

  template <>
  inline auto ActorAttributeValueAccess::As<rpc::ActorAttributeType::Int>() const {
    return As<int>();
  }
  // 显式特化模板函数As，当指定类型为rpc::ActorAttributeType::Int时，调用As<int>()进行转换

  template <>
  inline auto ActorAttributeValueAccess::As<rpc::ActorAttributeType::Float>() const {
    return As<float>();
  }
  // 显式特化模板函数As，当指定类型为rpc::ActorAttributeType::Float时，调用As<float>()进行转换

  template <>
  inline auto ActorAttributeValueAccess::As<rpc::ActorAttributeType::String>() const {
    return As<std::string>();
  }
  // 显式特化模板函数As，当指定类型为rpc::ActorAttributeType::String时，调用As<std::string>()进行转换

  template <>
  inline auto ActorAttributeValueAccess::As<rpc::ActorAttributeType::RGBColor>() const {
    return As<sensor::data::Color>();
  }
  // 显式特化模板函数As，当指定类型为rpc::ActorAttributeType::RGBColor时，调用As<sensor::data::Color>()进行转换


  template <typename T>
  inline bool ActorAttributeValueAccess::operator==(const T &rhs) const {
    return As<T>() == rhs;
  }
  // 模板函数，用于比较当前对象转换为类型T后与rhs是否相等，通过调用As<T>()得到转换后的值再进行比较


  template <>
  inline bool ActorAttributeValueAccess::operator==(const ActorAttributeValueAccess &rhs) const {
    return
        (GetType() == rhs.GetType()) &&
        (GetValue() == rhs.GetValue());
  }
  // 显式特化模板函数operator==，用于比较两个ActorAttributeValueAccess对象是否相等
  // 通过比较它们的类型（GetType()）和值（GetValue()）来确定是否相等


  class ActorAttributeValue: public ActorAttributeValueAccess {


  public:
    ActorAttributeValue(rpc::ActorAttributeValue attribute):
      _attribute(std::move(attribute))
    {
      Validate();
    }
    // 构造函数，接受一个rpc::ActorAttributeValue类型的参数attribute
    // 使用std::move将attribute移动构造到内部成员变量_attribute中
    // 然后调用Validate()函数验证属性的合法性

    ActorAttributeValue(ActorAttributeValue const &) = default;
    // 拷贝构造函数，使用默认拷贝语义
    ActorAttributeValue(ActorAttributeValue &&) = default;
    // 移动构造函数，使用默认移动语义
    virtual ~ActorAttributeValue() = default;
    // 析构函数，使用默认析构语义

    ActorAttributeValue & operator= (ActorAttributeValue const & ) = default;
    // 拷贝赋值运算符，使用默认拷贝语义
    ActorAttributeValue & operator= (ActorAttributeValue && ) = default;
    // 移动赋值运算符，使用默认移动语义

    virtual const std::string &GetId() const override {
      return _attribute.id;
    }
    // 重写基类的GetId()函数，返回内部成员变量_attribute的id成员

    virtual rpc::ActorAttributeType GetType() const override {
      return _attribute.type;
    }
    // 重写基类的GetType()函数，返回内部成员变量_attribute的type成员

    /// Serialize this object as a carla::rpc::ActorAttributeValue.
    operator rpc::ActorAttributeValue() const{
      return _attribute;
    }
    // 类型转换运算符，将ActorAttributeValue对象转换为rpc::ActorAttributeValue类型，直接返回内部成员变量_attribute


    virtual const std::string &GetValue() const override {
      return _attribute.value;
    }
    // 重写基类的GetValue()函数，返回内部成员变量_attribute的value成员


  private:

    rpc::ActorAttributeValue _attribute;
    // 内部成员变量，用于存储rpc::ActorAttributeValue类型的数据
  };


  template <>
  inline bool ActorAttributeValueAccess::operator==(const ActorAttributeValue &rhs) const {
    return rhs.operator==(*this);
  }
  // 显式特化模板函数operator==，用于比较当前对象与ActorAttributeValue对象rhs是否相等
  // 通过调用rhs的operator==函数并将当前对象作为参数传入来进行比较


  /// An attribute of an ActorBlueprint.
  class ActorAttribute: public ActorAttributeValueAccess {


  public:
    ActorAttribute(rpc::ActorAttribute attribute)
      : ActorAttributeValueAccess(),
        _attribute(std::move(attribute)) {
      Validate();
    }
    // 构造函数，接受一个rpc::ActorAttribute类型的参数attribute
    // 先调用基类的默认构造函数ActorAttributeValueAccess()
    // 然后使用std::move将attribute移动构造到内部成员变量_attribute中
    // 最后调用Validate()函数验证属性的合法性

    ActorAttribute(ActorAttribute const &) = default;
    // 拷贝构造函数，使用默认拷贝语义
    ActorAttribute(ActorAttribute &&) = default;
    // 移动构造函数，使用默认移动语义
    virtual ~ActorAttribute() = default;
    // 析构函数，使用默认析构语义

    ActorAttribute & operator= (ActorAttribute const & ) = default;
    // 拷贝赋值运算符，使用默认拷贝语义
    ActorAttribute & operator= (ActorAttribute && ) = default;
    // 移动赋值运算符，使用默认移动语义

    virtual const std::string &GetId() const override {
      return _attribute.id;
    }
    // 重写基类的GetId()函数，返回内部成员变量_attribute的id成员

    virtual rpc::ActorAttributeType GetType() const override {
      return _attribute.type;
    }
    // 重写基类的GetType()函数，返回内部成员变量_attribute的type成员

    const std::vector<std::string> &GetRecommendedValues() const {
      return _attribute.recommended_values;
    }
    // 函数用于获取推荐的值列表，直接返回内部成员变量_attribute的recommended_values成员

    bool IsModifiable() const {
      return _attribute.is_modifiable;
    }
    // 函数用于判断属性是否可修改，直接返回内部成员变量_attribute的is_modifiable成员


    /// 设置这个属性值
    ///
    /// @throw 如果属性不可修改，则抛出InvalidAttributeValue异常。
    /// @throw 如果格式不匹配这个类型，则抛出InvalidAttributeValue异常。
    void Set(std::string value);
    // 函数用于设置属性的值，在函数体中会进行属性可修改性和格式匹配性的检查，如果不满足则抛出InvalidAttributeValue异常


    /// 将此对象序列化为carla::rpc::ActorAttributeValue。
    operator rpc::ActorAttributeValue() const {
      return _attribute;
    }
    // 类型转换运算符，将ActorAttribute对象转换为rpc::ActorAttributeValue类型，直接返回内部成员变量_attribute


    virtual const std::string &GetValue() const override {
      return _attribute.value;
    }
    // 重写基类的GetValue()函数，返回内部成员变量_attribute的value成员


  private:
    rpc::ActorAttribute _attribute;
    // 内部成员变量，用于存储rpc::ActorAttribute类型的数据
  };


  template <>
  inline bool ActorAttributeValueAccess::operator==(const ActorAttribute &rhs) const {
    return rhs.operator==(*this);
  }
  // 显式特化模板函数operator==，用于比较当前对象与ActorAttribute对象rhs是否相等
  // 通过调用rhs的operator==函数并将当前对象作为参数传入来进行比较


} // namespace client
} // namespace carla