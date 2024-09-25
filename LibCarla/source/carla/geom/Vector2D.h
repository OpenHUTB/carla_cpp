// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/MsgPack.h"

#include <cmath>
#include <limits>

namespace carla {
namespace geom { ///定义两个嵌套的命名空间：carla和geom。

  class Vector2D { ///定义一个名为Vector2D的公共类，用于表示二维向量。
  public:

    // =========================================================================
    // -- 公开数据成员 --------------------------------------------------
    // =========================================================================

    float x = 0.0f;

    float y = 0.0f; // 向量的两个分量，x和y

    // =========================================================================
    // -- 构造函数 ---------------------------------------------------------
    // =========================================================================

    Vector2D() = default; ///定义一个默认构造函数。

    Vector2D(float ix, float iy) // 构造函数，接受两个参数初始化x和y
      : x(ix),
        y(iy) {}

    // =========================================================================
    // -- 其他方法 --------------------------------------------------------
    // =========================================================================

    float SquaredLength() const { // 计算向量的长度的平方，避免开方运算，提高效率
      return x * x + y * y;
    }

    float Length() const { // 计算向量的长度
       return std::sqrt(SquaredLength());
    }

    Vector2D MakeUnitVector() const { // 将向量转换为单位向量，要求向量长度不为零
      const float len = Length();
      DEVELOPMENT_ASSERT(len > 2.0f * std::numeric_limits<float>::epsilon());
      const float k = 1.0f / len;
      return Vector2D(x * k, y * k);
    }

    // =========================================================================
    // -- 算术运算符 -------------------------------------------------
    // =========================================================================

    Vector2D &operator+=(const Vector2D &rhs) { // 重载+=运算符，实现向量的加法
      x += rhs.x;
      y += rhs.y;
      return *this;
    }

    friend Vector2D operator+(Vector2D lhs, const Vector2D &rhs) { // 重载+运算符，实现向量的加法
      lhs += rhs;
      return lhs;
    }

    Vector2D &operator-=(const Vector2D &rhs) { // 重载-=运算符，实现向量的减法
      x -= rhs.x;
      y -= rhs.y;
      return *this;
    }

    friend Vector2D operator-(Vector2D lhs, const Vector2D &rhs) { // 重载-运算符，实现向量的减法
      lhs -= rhs;
      return lhs;
    }

    Vector2D &operator*=(float rhs) {  // 重载*=运算符，实现向量与标量的乘法
      x *= rhs;
      y *= rhs;
      return *this;
    }

    friend Vector2D operator*(Vector2D lhs, float rhs) { // 重载*运算符，实现向量与标量的乘法
      lhs *= rhs;
      return lhs;
    }

    friend Vector2D operator*(float lhs, Vector2D rhs) { // 重载*运算符，实现标量与向量的乘法
      rhs *= lhs;
      return rhs;
    }

    Vector2D &operator/=(float rhs) { // 重载/=运算符，实现向量与标量的除法
      x /= rhs;
      y /= rhs;
      return *this;
    }

    friend Vector2D operator/(Vector2D lhs, float rhs) { // 重载/运算符，实现向量与标量的除法
      lhs /= rhs;
      return lhs;
    }

    friend Vector2D operator/(float lhs, Vector2D rhs) { // 重载/运算符，实现标量与向量的除法
      rhs /= lhs;
      return rhs;
    }

    // =========================================================================
    // -- 比较运算符 -------------------------------------------------
    // =========================================================================

    bool operator==(const Vector2D &rhs) const { // 重载==运算符，比较两个向量是否相等
      return (x == rhs.x) && (y == rhs.y);
    }

    bool operator!=(const Vector2D &rhs) const { // 重载!=运算符，比较两个向量是否不相等
      return !(*this == rhs);
    }

    // =========================================================================
    // -- 转换为UE4类型 ---------------------------------------------
    // =========================================================================

#ifdef LIBCARLA_INCLUDED_FROM_UE4

    /// 将向量从厘米转换为米
    Vector2D ToMeters() const {
      return *this * 1e-2f;
    }

    /// 将向量从米转换为厘米
    Vector2D ToCentimeters() const {
      return *this * 1e2f;
    }

    /// 将向量转换为UE4引擎的FVector2D类型
    FVector2D ToFVector2D() const {
      return FVector2D{x, y};
    }

#endif // LIBCARLA_INCLUDED_FROM_UE4

    MSGPACK_DEFINE_ARRAY(x, y) // 序列化向量数据，用于消息打包
  };

} // namespace geom
} // namespace carla
