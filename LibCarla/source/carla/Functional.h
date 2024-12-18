// 版权所有 (c) 2017 巴塞罗那自治大学 (UAB) 计算机视觉中心 (CVC)。
//
// 本作品依据 MIT 许可证的条款进行授权。
// 如需副本，请访问 https://opensource.org/licenses/MIT。

#pragma once  // 防止此头文件在单个编译单元中被多次包含

#include <utility>  // 包含标准库中的实用程序，用于完美转发

namespace carla {

  class Functional {
  public:

    /// 创建一个递归调用对象，该对象将自身作为第一个参数传递给提供的函数对象 @a func。
    /// 这允许创建递归 lambda 表达式。
    template <typename FuncT>
    static auto MakeRecursive(FuncT &&func) {
      return Recursive<FuncT>(std::forward<FuncT>(func));
    }

    /// 创建一个“重载调用对象”，由一个或多个可调用对象组成，
    /// 每个可调用对象将贡献一个 operator() 的重载。
    /// 用例：将多个 lambda 组合成一个 lambda。
    template <typename... FuncTs>
    static auto MakeOverload(FuncTs &&... fs) {
      // 使用完美转发将 fs 参数包传递给 Overload 结构体的构造函数
      return Overload<FuncTs...>(std::forward<FuncTs>(fs)...);
    }

    /// 创建一个递归重载调用对象，它结合了 MakeRecursive 和 MakeOverload 的功能。
    /// 这允许创建一个既递归又重载的调用对象。
    /// @see MakeRecursive 和 MakeOverload 以获取更多信息。
    template <typename... FuncTs>
    static auto MakeRecursiveOverload(FuncTs &&... fs) {
      // 首先创建一个重载调用对象，然后将其包装在一个递归调用对象中
      return MakeRecursive(MakeOverload(std::forward<FuncTs>(fs)...));
    }

  private:

    // 递归特化 Overload 结构体，用于处理两个或更多个可调用对象的情况
    template <typename... Ts>
    struct Overload;

    // 递归特化 Overload 结构体，用于处理两个或更多个可调用对象的情况
    template <typename T, typename... Ts>
    struct Overload<T, Ts...> : T, Overload<Ts...> {
      // 构造函数，使用完美转发初始化 T 和其余的 Overload 基类
      Overload(T &&func, Ts &&... rest)
        : T(std::forward<T>(func)),// 初始化基类 T
          Overload<Ts...>(std::forward<Ts>(rest)...) {}
      // 继承 T 和 Overload<Ts...> 的 operator()
      using T::operator();
      using Overload<Ts...>::operator();
    };

    // Overload 结构体的基本情况，仅处理一个可调用对象
    template <typename T>
    struct Overload<T> : T { // 基本情况，只处理一个可调用对象
      // 构造函数，使用完美转发初始化 T
      Overload(T &&func) : T(std::forward<T>(func)) {} // 初始化基类 T
      // 继承 T 的 operator()
      using T::operator();  // 继承 T 的 operator()
    };

    // 定义 Recursive 结构体，用于实现递归调用
    template<typename T>
    struct Recursive {  // 定义 Recursive 结构体，用于递归调用
      // 构造函数，使用完美转发初始化 _func
      explicit Recursive(T &&func) : _func(std::forward<T>(func)) {} 
      // 重载 operator() 以实现递归调用，将自身 (*this) 作为第一个参数传递给 _func
      template<typename... Ts> 
      auto operator()(Ts &&... arguments) const {     // 将自身 (*this) 作为第一个参数传递给 _func，实现递归
        return _func(*this, std::forward<Ts>(arguments)...);
      }

    private:

      T _func; // 存储提供的函数对象
    };

  };

} // 命名空间 carla
