// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.
#pragma once  // 防止头文件被多次包含，确保每个源文件只包含一次此头文件

#include <utility>  // 包含标准库中的实用工具头文件，用于完美转发等特性

namespace carla {

  // Functional 类定义，提供创建特殊调用对象的静态方法
  class Functional {
  public:

      /// 创建一个递归调用对象
      ///
      /// 该函数模板接受一个可调用对象（FuncT 类型），并返回一个递归调用对象。
      /// 递归调用对象将自身作为第一个参数传递给传入的可调用对象（func），
      /// 从而允许实现递归逻辑而无需显式传递函数指针或对象。
      ///
      /// @param func 传入的可调用对象，它接受一个与自身类型相同的参数（递归调用时提供）。
      /// @return 返回一个递归调用对象，该对象可以调用原始的可调用对象，并传递自身作为参数。
      ///
      /// 用法示例：可以创建递归 lambda 表达式。
      template <typename FuncT>
      static auto MakeRecursive(FuncT &&func) {
        return Recursive<FuncT>(std::forward<FuncT>(func));
      }

      /// 创建一个重载调用对象
      ///
      /// 该函数模板接受一个或多个可调用对象（FuncTs... 类型），
      /// 并返回一个重载调用对象。重载调用对象将贡献一个或多个 operator() 的重载，
      /// 每个重载对应一个传入的可调用对象。
      ///
      /// @param fs 传入的一个或多个可调用对象，它们将贡献 operator() 的重载。
      /// @return 返回一个重载调用对象，该对象可以根据参数类型或数量调用相应的可调用对象。
      ///
      /// 用例：可以将多个 lambda 表达式组合成一个具有多个重载的 lambda。
      template <typename... FuncTs>
      static auto MakeOverload(FuncTs &&... fs) {
        return Overload<FuncTs...>(std::forward<FuncTs>(fs)...);
      }

      /// 创建递归重载调用对象
      ///
      /// 该函数模板结合了 MakeRecursive 和 MakeOverload 的功能，
      /// 接受一个或多个可调用对象，并返回一个递归重载调用对象。
      /// 递归重载调用对象既支持递归调用，又支持根据参数类型或数量选择重载。
      ///
      /// @param fs 传入的一个或多个可调用对象，它们将组合成递归重载调用对象。
      /// @return 返回一个递归重载调用对象，该对象结合了递归和重载的特性。
      ///
      /// @see MakeRecursive 和 MakeOverload，了解如何分别创建递归调用对象和重载调用对象。
      template <typename... FuncTs>
      static auto MakeRecursiveOverload(FuncTs &&... fs) {
        return MakeRecursive(MakeOverload(std::forward<FuncTs>(fs)...));
      }

  }; // Functional 类结束

} // carla 命名空间结束
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
    struct Overload<T> : T { // 基本情况，只处理一个可调用对象
      Overload(T &&func) : T(std::forward<T>(func)) {} // 初始化基类 T

      using T::operator();  // 继承 T 的 operator()
    };

    template<typename T>
    struct Recursive {  // 定义 Recursive 结构体，用于递归调用

      explicit Recursive(T &&func) : _func(std::forward<T>(func)) {}  // 构造函数，初始化 _func
       
      template<typename... Ts>           // 重载 operator() 实现递归调用
      auto operator()(Ts &&... arguments) const {     // 将自身 (*this) 作为第一个参数传递给 _func，实现递归
        return _func(*this, std::forward<Ts>(arguments)...);
      }

    private:

      T _func;
    };

  };

} // namespace carla
