// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once  // 防止头文件被多次包含

#include <utility>  // 包含标准库中用于处理完美转发的头文件

namespace carla {

  class Functional {
  public:

      /// 创建一个递归调用对象，将自身作为第一个参数传递给 @a func。
      /// 用法示例：创建递归 lambda
    template <typename FuncT>
    static auto MakeRecursive(FuncT &&func) {
      return Recursive<FuncT>(std::forward<FuncT>(func));
    }

    /// 创建一个“重载调用对象”，由一个或多个可调用对象组成，
    /// 每个可调用对象将贡献一个 operator() 的重载。
    /// 用例：将多个 lambda 组合成一个 lambda。
    template <typename... FuncTs>
    static auto MakeOverload(FuncTs &&... fs) {
      return Overload<FuncTs...>(std::forward<FuncTs>(fs)...);
    }

    /// 创建递归重载调用对象，组合 MakeRecursive 和 MakeOverload 的功能。
    /// @see MakeRecursive 和 MakeOverload。
    template <typename... FuncTs>
    static auto MakeRecursiveOverload(FuncTs &&... fs) {
      return MakeRecursive(MakeOverload(std::forward<FuncTs>(fs)...));
    }

  private:

    // 声明一个模板结构体 Overload，用于组合多个可调用对象
    template <typename... Ts>
    struct Overload;

    // 处理多个可调用对象的情况，递归继承 Overload
    template <typename T, typename... Ts>
    struct Overload<T, Ts...> : T, Overload<Ts...> {
        // 构造函数，初始化 T 和其余的 Overload 基类
      Overload(T &&func, Ts &&... rest)
        : T(std::forward<T>(func)),// 初始化基类 T
          Overload<Ts...>(std::forward<Ts>(rest)...) {}

      using T::operator();

      using Overload<Ts...>::operator();
    };

    template <typename T>
    struct Overload<T> : T {
      Overload(T &&func) : T(std::forward<T>(func)) {}

      using T::operator();
    };

    template<typename T>
    struct Recursive {

      explicit Recursive(T &&func) : _func(std::forward<T>(func)) {}

      template<typename... Ts>
      auto operator()(Ts &&... arguments) const {
        return _func(*this, std::forward<Ts>(arguments)...);
      }

    private:

      T _func;
    };

  };

} // namespace carla
