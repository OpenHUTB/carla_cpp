// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Debug.h"

#include <array>
// 定义名为 carla 的命名空间，在这个命名空间下组织相关的代码，有助于避免命名冲突等问题
namespace carla {
namespace geom {

  /// 定义一个三次多项式CubicPolynomial类，用于描述和计算三次多项式 f(x) = a + b * x + c * x^2 + d * x^3 
  class CubicPolynomial {
  public:
      // 定义CubicPolynomial中使用的数值类型，这里选择为double，适用于需要高精度的数学计算
    using value_type = double;

    // =========================================================================
    // -- 构造函数 --------------------------------------------------------------
    // =========================================================================
    // 默认构造函数，使用=default意味着使用编译器自动生成的默认构造函数，不执行任何操作
    CubicPolynomial() = default;
    // 拷贝构造函数，同样使用=default意味着使用编译器自动生成的拷贝构造函数， 它会自动复制源对象的所有成员到新对象中
    CubicPolynomial(const CubicPolynomial &) = default;
  // 构造函数，用于创建一个 CubicPolynomial 类的对象，接受四个参数，分别对应三次多项式的系数 a、b、c、d
  // 例如对于三次多项式 f(x) = a + b * x + c * x^2 + d * x^3，这里传入的参数就是各项的系数，
  // 并使用初始化列表对类的成员进行初始化，将传入的系数依次放入 _v 数组中，同时将表示偏移量的 _s 初始化为 0.0
    CubicPolynomial(
        const value_type &a,
        const value_type &b,
        const value_type &c,
        const value_type &d)
      : _v{ {a, b, c, d} },
        _s(0.0) {}

    CubicPolynomial(
        const value_type &a,
        const value_type &b,
        const value_type &c,
        const value_type &d,
        const value_type &s) // lateral offset
      : _v{ {a - b * s + c * s * s - d * s * s * s,
             b - 2 * c * s + 3 * d * s * s,
             c - 3 * d * s,
             d} },
        _s(s) {}

    // =========================================================================
    // -- 获取值 ----------------------------------------------------------------
    // =========================================================================
 // 获取三次多项式中系数 a 的值，返回类型为 value_type（也就是 double 类型），
 // 该函数为 const 成员函数，表示不会修改类的成员变量，只是获取其值
    value_type GetA() const {
      return _v[0];
    }
// 获取三次多项式中系数 b 的值，返回类型为 value_type（也就是 double 类型），
 // 该函数为 const 成员函数，表示不会修改类的成员变量，只是获取其值
    value_type GetB() const {
      return _v[1];
    }
// 获取三次多项式中系数 c 的值，返回类型为 value_type（也就是 double 类型）
// 该函数为 const 成员函数，表示不会修改类的成员变量，只是获取其值
    value_type GetC() const {
      return _v[2];
    }
// 获取三次多项式中系数 d 的值，返回类型为 value_type（也就是 double 类型），
// 该函数为 const 成员函数，表示不会修改类的成员变量，只是获取其值
    value_type GetD() const {
      return _v[3];
    }
// 获取表示偏移量（横向偏移量等相关概念）的 _s 的值，返回类型为 value_type（也就是 double 类型），
// 该函数为 const 成员函数，表示不会修改类的成员变量，只是获取其值
    value_type GetS() const {
      return _s;
    }

    // =========================================================================
    // -- 设置值 ---------------------------------------------------------------
    // =========================================================================
  // 设置三次多项式的系数以及偏移量的值，接受五个参数，分别对应调整后的系数 a、b、c、d 和偏移量 s，
  // 按照特定的数学计算方式（与前面构造函数中根据偏移量调整系数的方式一致）更新 _v 数组中的系数值，并将传入的偏移量 s 赋值给 _s
    void Set(
        const value_type &a,
        const value_type &b,
        const value_type &c,
        const value_type &d,
        const value_type &s) { // lateral offset
      _v = { a - b * s + c * s * s - d * s * s * s,
             b - 2 * c * s + 3 * d * s * s,
             c - 3 * d * s,
             d };
      _s = s;
    }
    // 设置三次多项式的系数的值，接受四个参数，分别对应系数 a、b、c、d，
   // 将传入的系数依次放入 _v 数组中，并将表示偏移量的 _s 初始化为 0.0，用于在不需要考虑偏移量或者重置系数时使用
    void Set(
        const value_type &a,
        const value_type &b,
        const value_type &c,
        const value_type &d) {
      _v = {a, b, c, d};
      _s = 0.0;
    }

    // =========================================================================
    // -- 评估方法 -------------------------------------------------------------
    // =========================================================================

    /// 评估 f(x) = a + bx + cx^2 + dx^3
    value_type Evaluate(const value_type &x) const {
      // return _v[0] + _v[1] * (x) + _v[2] * (x * x) + _v[3] * (x * x * x);
      return _v[0] + x * (_v[1] + x * (_v[2] + x * _v[3]));
    }

    /// 使用 df/dx = b + 2cx + 3dx^2 计算正切值
    value_type Tangent(const value_type &x) const {
      return _v[1] + x * (2 * _v[2] + x * 3 * _v[3]);
    }

    // =========================================================================
    // -- 算术运算符 ------------------------------------------------------------
    // =========================================================================
   // 复合赋值运算符 += 的重载函数，用于实现当前的 CubicPolynomial 对象与另一个同类型对象相加，并将结果赋值给当前对象，
  // 通过遍历系数数组 _v，将对应位置的系数相加，实现多项式的相加操作，最后返回当前对象的引用，以便支持连续的复合赋值操作（如 a += b += c; 这样的语法形式）
    CubicPolynomial &operator+=(const CubicPolynomial &rhs) {
      for (auto i = 0u; i < _v.size(); ++i) {
        _v[i] += rhs._v[i];
      }
      return *this;
    }
    // 加法运算符 + 的重载函数，以友元函数的形式实现，它创建一个当前对象的副本（通过传值参数 lhs），
    // 然后调用 += 运算符重载函数将传入的另一个对象 rhs 与之相加，最后返回相加后的结果对象，这样就实现了两个 CubicPolynomial 对象相加的操作
    friend CubicPolynomial operator+(CubicPolynomial lhs, const CubicPolynomial &rhs) {
      lhs += rhs;
      return lhs;
    }
    // 复合赋值运算符 *= 的重载函数，用于实现当前的 CubicPolynomial 对象与一个数值（value_type 类型，也就是 double 类型）相乘，并将结果赋值给当前对象，
    // 通过遍历系数数组 _v，将每个系数都与传入的数值 rhs 相乘，实现多项式的数乘操作，最后返回当前对象的引用，以便支持连续的复合赋值操作（如 a *= b *= c; 这样的语法形式）
    CubicPolynomial &operator*=(const value_type &rhs) {
      for (auto i = 0u; i < _v.size(); ++i) {
        _v[i] *= rhs;
      }
      return *this;
    }
    // 乘法运算符 * 的重载函数，以友元函数的形式实现，它创建一个当前对象的副本（通过传值参数 lhs），
   // 然后调用 *= 运算符重载函数将传入的数值 rhs 与之相乘，最后返回相乘后的结果对象，这样就实现了一个 CubicPolynomial 对象与一个数值相乘的操作
    friend CubicPolynomial operator*(CubicPolynomial lhs, const value_type &rhs) {
      lhs *= rhs;
      return lhs;
    }
    // 乘法运算符 * 的另一种重载形式，同样以友元函数的形式实现，用于实现一个数值与一个 CubicPolynomial 对象相乘的操作，
   // 它通过将传入的数值 lhs 与 rhs 对象调用 *= 运算符重载函数相乘，最后返回相乘后的结果对象，实现了乘法的交换律，使得数值与多项式相乘的顺序不影响结果
    friend CubicPolynomial operator*(const value_type &lhs, CubicPolynomial rhs) {
      rhs *= lhs;
      return rhs;
    }

  private:

    // =========================================================================
    // -- 私有数据成员 ----------------------------------------------------------
    // =========================================================================

    // a - 截距 elevation
    // b - 斜率
    // c - 垂直曲率 vertical curvature
    // d - 曲率变化
    std::array<value_type, 4> _v = {0.0, 0.0, 0.0, 0.0};

    // s - 距离
    value_type _s;
  };

} // namespace geom
} // namespace carla
