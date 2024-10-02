// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Debug.h"

#include <array>

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

    value_type GetA() const {
      return _v[0];
    }

    value_type GetB() const {
      return _v[1];
    }

    value_type GetC() const {
      return _v[2];
    }

    value_type GetD() const {
      return _v[3];
    }

    value_type GetS() const {
      return _s;
    }

    // =========================================================================
    // -- 设置值 ---------------------------------------------------------------
    // =========================================================================

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

    CubicPolynomial &operator+=(const CubicPolynomial &rhs) {
      for (auto i = 0u; i < _v.size(); ++i) {
        _v[i] += rhs._v[i];
      }
      return *this;
    }

    friend CubicPolynomial operator+(CubicPolynomial lhs, const CubicPolynomial &rhs) {
      lhs += rhs;
      return lhs;
    }

    CubicPolynomial &operator*=(const value_type &rhs) {
      for (auto i = 0u; i < _v.size(); ++i) {
        _v[i] *= rhs;
      }
      return *this;
    }

    friend CubicPolynomial operator*(CubicPolynomial lhs, const value_type &rhs) {
      lhs *= rhs;
      return lhs;
    }

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
