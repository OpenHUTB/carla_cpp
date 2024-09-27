// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once  // 防止头文件被重复包含

#include "carla/MsgPack.h"  // 引入MsgPack库头文件
#include "carla/MsgPackAdaptors.h"  // 引入MsgPack适配器头文件

#include <boost/optional.hpp>  // 引入Boost选项库

#ifdef _MSC_VER  // 如果是MSVC编译器
#pragma warning(push)  // 保存当前警告状态
#pragma warning(disable:4583)  // 禁用特定警告
#pragma warning(disable:4582)  // 禁用特定警告
#include <boost/variant2/variant.hpp>  // 引入Boost变体库
#pragma warning(pop)  // 恢复之前的警告状态
#else  // 如果不是MSVC编译器
#include <boost/variant2/variant.hpp>  // 引入Boost变体库
#endif

#include <string>  // 引入字符串库

namespace carla {  // 定义carla命名空间
namespace rpc {  // 定义rpc子命名空间

  class ResponseError {  // 定义响应错误类
  public:

    ResponseError() = default;  // 默认构造函数

    explicit ResponseError(std::string message)  // 带字符串参数的构造函数
      : _what(std::move(message)) {}  // 移动构造字符串

    const std::string &What() const {  // 获取错误消息
      return _what;  // 返回错误消息
    }

    MSGPACK_DEFINE_ARRAY(_what)  // 定义MsgPack数组序列化

  private:

    /// @todo 需要初始化，空字符串会导致对nullptr调用memcpy。
    /// 可能是MsgPack的bug，但也可能是我们对变体的特化问题
    std::string _what{"unknown error"};  // 错误消息，默认值为“未知错误”
  };

  template <typename T>  // 模板类，类型为T
  class Response {  // 定义响应类
  public:

    using value_type = T;  // 使用类型T作为value_type

    using error_type = ResponseError;  // 使用ResponseError作为error_type

    Response() = default;  // 默认构造函数

    template <typename TValue>  // 模板构造函数，接收任意类型的值
    Response(TValue &&value) : _data(std::forward<TValue>(value)) {}  // 移动构造数据

    template <typename TValue>  // 模板函数，重置数据
    void Reset(TValue &&value) {
      _data = std::forward<TValue>(value);  // 移动赋值新数据
    }

    bool HasError() const {  // 检查是否有错误
      return _data.index() == 0;  // 如果第一个索引是0，则表示有错误
    }

    template <typename... Ts>  // 模板函数，设置错误
    void SetError(Ts &&... args) {
      _data = error_type(std::forward<Ts>(args)...);  // 移动构造错误对象
    }

    const error_type &GetError() const {  // 获取错误
      DEBUG_ASSERT(HasError());  // 确保有错误
      return boost::variant2::get<error_type>(_data);  // 返回错误对象
    }

    value_type &Get() {  // 获取有效数据
      DEBUG_ASSERT(!HasError());  // 确保没有错误
      return boost::variant2::get<value_type>(_data);  // 返回数据
    }

    const value_type &Get() const {  // 获取有效数据（常量版本）
      DEBUG_ASSERT(!HasError());  // 确保没有错误
      return boost::variant2::get<value_type>(_data);  // 返回数据
    }

    operator bool() const {  // 重载布尔转换运算符
      return !HasError();  // 如果没有错误返回true
    }

    MSGPACK_DEFINE_ARRAY(_data)  // 定义MsgPack数组序列化

  private:

    boost::variant2::variant<error_type, value_type> _data;  // 存储错误或有效数据的变体
  };

  template <>  // 对于特殊化的模板
  class Response<void> {  // 特化响应类，类型为void
  public:

    using value_type = void;  // 使用void作为value_type

    using error_type = ResponseError;  // 使用ResponseError作为error_type

    static Response Success() {  // 静态函数，返回成功的响应
      return success_flag{};  // 返回成功标志
    }

    Response() : _data(error_type{}) {}  // 默认构造函数，初始化为错误对象

    Response(ResponseError error) : _data(std::move(error)) {}  // 带错误参数的构造函数

    bool HasError() const {  // 检查是否有错误
      return _data.has_value();  // 如果有值则表示有错误
    }

    template <typename... Ts>  // 模板函数，设置错误
    void SetError(Ts &&... args) {
      _data = error_type(std::forward<Ts>(args)...);  // 移动构造错误对象
    }

    const error_type &GetError() const {  // 获取错误
      DEBUG_ASSERT(HasError());  // 确保有错误
      return *_data;  // 返回错误对象
    }

    operator bool() const {  // 重载布尔转换运算符
      return !HasError();  // 如果没有错误返回true
    }

    MSGPACK_DEFINE_ARRAY(_data)  // 定义MsgPack数组序列化

  private:

    struct success_flag {};  // 成功标志的内部结构体

    Response(success_flag) {}  // 带成功标志的私有构造函数

    boost::optional<error_type> _data;  // 可选的错误对象
  };

} // namespace rpc
} // namespace carla