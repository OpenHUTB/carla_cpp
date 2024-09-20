// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

<<<<<<< HEAD
#pragma once  // ȷ��ͷ�ļ�ֻ������һ��

#include <type_traits>  // �����������Կ⣬��������������صĹ���
#include <utility>  //  ����ʵ�ù��߿⣬�ṩ std::move �ȹ���

namespace carla {  // ���������ռ� carla
namespace detail {   //  ���������ռ� detail������ʵ��ϸ��

  template <typename FunctorT>  // ģ�������������������� FunctorT
  struct MoveWrapper : FunctorT {// ���� MoveWrapper �ṹ���̳��� FunctorT
    MoveWrapper(FunctorT &&f) : FunctorT(std::move(f)) {}
=======
#pragma once  // 确保此头文件仅被包含一次

#include <type_traits>  // 包含类型特征相关的头文件，提供类型特性支持
#include <utility>  // 包含通用工具函数，比如 std::move

namespace carla {  // 定义命名空间 carla
namespace detail {   // 定义命名空间 detail，用于实现细节


  template <typename FunctorT>  // 定义一个模板结构体，接受任意类型 FunctorT
  struct MoveWrapper : FunctorT {  // MoveWrapper 继承自 FunctorT
    MoveWrapper(FunctorT &&f) : FunctorT(std::move(f)) {}  // 构造函数，移动构造 FunctorT
>>>>>>> 6bf499bf364e66adc2c81feba27503c6e62f698e

    MoveWrapper(MoveWrapper &&) = default;  // 移动构造函数，使用默认实现
    MoveWrapper& operator=(MoveWrapper &&) = default;  // 移动赋值运算符，使用默认实现

    MoveWrapper(const MoveWrapper &);  // 声明复制构造函数（未定义）
    MoveWrapper& operator=(const MoveWrapper &);  // 声明复制赋值运算符（未定义）
  };

} // namespace detail

  /// 下面是一种hack，用于绕过Asio库的限制，使其能够接受仅可移动的处理程序。
  /// 如果Asio试图复制一个仅可移动的处理程序，那么编译时可能不会立即报错
  ///
  /// @see https://stackoverflow.com/a/22891509.
  template <typename FunctorT>  // 定义一个模板函数，接受任意类型 FunctorT
  auto MoveHandler(FunctorT &&func) {   // MoveHandler 函数返回一个移动处理器
    using F = typename std::decay<FunctorT>::type;  // 使用 std::decay 处理 FunctorT 的类型
    return detail::MoveWrapper<F>{std::move(func)};   // 返回一个 MoveWrapper 实例，移动传入的 func
  }

} // namespace carla
