// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/rpc/ActorAttribute.h"
#include "carla/sensor/data/Color.h"

#include <cstdlib>
#include <exception>
#include <type_traits>

namespace carla {
namespace client {

  // ===========================================================================
  // -- 无效属性值 InvalidAttributeValue ----------------------------------------
  // ===========================================================================

  /// 当赋予 ActorAttribute 的值无法转换为其类型时抛出异常。
  class InvalidAttributeValue : public std::invalid_argument {
  public:

    InvalidAttributeValue(const std::string &what) : std::invalid_argument(what) {}
  };

  // ===========================================================================
  // -- 错误的属性转换异常 BadAttributeCast --------------------------------------
  // ===========================================================================

  /// 当 ActorAttribute 的值无法转换为请求的类型时抛出异常。
  class BadAttributeCast : public std::logic_error {
  public:

    BadAttributeCast(const std::string &what) : std::logic_error(what) {}
  };

  // ===========================================================================
  // -- 参与者属性 --------------------------------------------------------------
  // ===========================================================================

  class ActorAttributeValueAccess
  {
  public:
    ActorAttributeValueAccess() = default;
    ActorAttributeValueAccess(ActorAttributeValueAccess const &) = default;
    ActorAttributeValueAccess(ActorAttributeValueAccess &&) = default;
    virtual ~ActorAttributeValueAccess() = default;

    ActorAttributeValueAccess & operator= (ActorAttributeValueAccess const & ) = default;
    ActorAttributeValueAccess & operator= (ActorAttributeValueAccess && ) = default;

    virtual const std::string &GetId() const = 0;

    virtual rpc::ActorAttributeType GetType() const = 0;

    /// 将值转换为给定的类型。
    ///
    /// @throw 如果转换失败跑出BadAttributeCast异常。
    template <typename T>
    T As() const;

    /// 将值转换为枚举 carla::rpc::ActorAttributeType 指定的类型。
    ///
    /// @throw 如果转换失败，则抛出 BadAttributeCast 的异常。
    template <rpc::ActorAttributeType Type>
    auto As() const;

    template <typename T>
    bool operator==(const T &rhs) const;

    template <typename T>
    bool operator!=(const T &rhs) const {
      return !(*this == rhs);
    }

  protected:
    virtual const std::string &GetValue() const = 0;

    void Validate() const;
  };

  template <>
  bool ActorAttributeValueAccess::As<bool>() const;

  template <>
  int ActorAttributeValueAccess::As<int>() const;

  template <>
  float ActorAttributeValueAccess::As<float>() const;

  template <>
  std::string ActorAttributeValueAccess::As<std::string>() const;

  template <>
  sensor::data::Color ActorAttributeValueAccess::As<sensor::data::Color>() const;

  template <>
  inline auto ActorAttributeValueAccess::As<rpc::ActorAttributeType::Bool>() const {
    return As<bool>();
  }

  template <>
  inline auto ActorAttributeValueAccess::As<rpc::ActorAttributeType::Int>() const {
    return As<int>();
  }

  template <>
  inline auto ActorAttributeValueAccess::As<rpc::ActorAttributeType::Float>() const {
    return As<float>();
  }

  template <>
  inline auto ActorAttributeValueAccess::As<rpc::ActorAttributeType::String>() const {
    return As<std::string>();
  }

  template <>
  inline auto ActorAttributeValueAccess::As<rpc::ActorAttributeType::RGBColor>() const {
    return As<sensor::data::Color>();
  }

  template <typename T>
  inline bool ActorAttributeValueAccess::operator==(const T &rhs) const {
    return As<T>() == rhs;
  }

  template <>
  inline bool ActorAttributeValueAccess::operator==(const ActorAttributeValueAccess &rhs) const {
    return
        (GetType() == rhs.GetType()) &&
        (GetValue() == rhs.GetValue());
  }

  class ActorAttributeValue: public ActorAttributeValueAccess {

  public:
    ActorAttributeValue(rpc::ActorAttributeValue attribute):
      _attribute(std::move(attribute))
    {
      Validate();
    }
    ActorAttributeValue(ActorAttributeValue const &) = default;
    ActorAttributeValue(ActorAttributeValue &&) = default;
    virtual ~ActorAttributeValue() = default;

    ActorAttributeValue & operator= (ActorAttributeValue const & ) = default;
    ActorAttributeValue & operator= (ActorAttributeValue && ) = default;

    virtual const std::string &GetId() const override {
      return _attribute.id;
    }

    virtual rpc::ActorAttributeType GetType() const override {
      return _attribute.type;
    }

    /// Serialize this object as a carla::rpc::ActorAttributeValue.
    operator rpc::ActorAttributeValue() const{
      return _attribute;
    }

    virtual const std::string &GetValue() const override {
      return _attribute.value;
    }

  private:

    rpc::ActorAttributeValue _attribute;
  };

  template <>
  inline bool ActorAttributeValueAccess::operator==(const ActorAttributeValue &rhs) const {
    return rhs.operator==(*this);
  }

  /// An attribute of an ActorBlueprint.
  class ActorAttribute: public ActorAttributeValueAccess {

  public:
    ActorAttribute(rpc::ActorAttribute attribute)
      : ActorAttributeValueAccess(),
        _attribute(std::move(attribute)) {
      Validate();
    }

    ActorAttribute(ActorAttribute const &) = default;
    ActorAttribute(ActorAttribute &&) = default;
    virtual ~ActorAttribute() = default;

    ActorAttribute & operator= (ActorAttribute const & ) = default;
    ActorAttribute & operator= (ActorAttribute && ) = default;

    virtual const std::string &GetId() const override {
      return _attribute.id;
    }

    virtual rpc::ActorAttributeType GetType() const override {
      return _attribute.type;
    }

    const std::vector<std::string> &GetRecommendedValues() const {
      return _attribute.recommended_values;
    }

    bool IsModifiable() const {
      return _attribute.is_modifiable;
    }

    /// 设置这个属性值
    ///
    /// @throw 如果属性不可修改，则抛出 InvalidAttributeValue 异常。
    /// @throw 如果格式不匹配这个类型，则抛出 InvalidAttributeValue 异常。
    void Set(std::string value);

    /// 将此对象序列化为 carla::rpc::ActorAttributeValue。
    operator rpc::ActorAttributeValue() const {
      return _attribute;
    }

    virtual const std::string &GetValue() const override {
      return _attribute.value;
    }

  private:
    rpc::ActorAttribute _attribute;
  };

  template <>
  inline bool ActorAttributeValueAccess::operator==(const ActorAttribute &rhs) const {
    return rhs.operator==(*this);
  }

} // namespace client
} // namespace carla
