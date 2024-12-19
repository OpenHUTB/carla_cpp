// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once  // 防止头文件被多次包含。编译器会确保该头文件在一个编译单元中只包含一次。
              // 它是传统的 #ifndef/#define 预处理指令的替代方式，避免重复包含问题。

#include "carla/MsgPack.h"  // 包含与 Carla 相关的 MsgPack 库头文件。
                           // MsgPack 是一种二进制序列化格式，通常用于高效的数据传输或存储。

#include <cmath>             // 包含标准数学库的头文件。提供数学相关函数，如 sin(), cos(), pow() 等。
#include <limits>            // 包含标准库中的限制常量，提供如数值类型的最小值和最大值等信息。
                           // 它常用于检查数值的边界条件，比如最小值、最大值等。


namespace carla {
namespace geom { /// 定义一个名为geom的命名空间，它是carla命名空间的一部分。

  class Vector2D { /// 定义一个名为Vector2D的公共类，用于表示二维向量，包含x和y两个分量。
  public:

    // =========================================================================
    // -- 公开数据成员 --------------------------------------------------
    // =========================================================================

    float x = 0.0f; ///< 向量的x分量，默认为0。

    float y = 0.0f; ///< 向量的y分量，默认为0。

    // =========================================================================
    // -- 构造函数 ---------------------------------------------------------
    // =========================================================================

    Vector2D() = default; ///< 默认构造函数，初始化x和y为0。

    Vector2D(float ix, float iy) // 构造函数，接受两个参数ix和iy来初始化x和y。
      : x(ix),
        y(iy) {}

    // =========================================================================
    // -- 其他方法 --------------------------------------------------------
    // =========================================================================

    float SquaredLength() const { // 计算向量的长度的平方，避免开方运算，提高效率
      return x * x + y * y; ///< 返回向量长度的平方（即x² + y²）。
    }

    float Length() const { // 计算向量的长度
       return std::sqrt(SquaredLength()); ///< 返回向量的实际长度，等于平方根(x² + y²)。
    }

    Vector2D MakeUnitVector() const { // 将向量转换为单位向量，要求向量长度不为零
      const float len = Length(); ///< 计算当前向量的长度。
      DEVELOPMENT_ASSERT(len > 2.0f * std::numeric_limits<float>::epsilon()); ///< 确保向量的长度不为零。
      const float k = 1.0f / len; ///< 计算单位化的比例。
      return Vector2D(x * k, y * k); ///< 返回单位向量，即x和y分别除以原来的长度。
    }

    // =========================================================================
    // -- 算术运算符 -------------------------------------------------
    // =========================================================================

    Vector2D &operator+=(const Vector2D &rhs) { // 重载+=运算符，实现向量的加法
      x += rhs.x;
      y += rhs.y;
      return *this; ///< 返回当前向量的引用，便于链式操作。
    }

    friend Vector2D operator+(Vector2D lhs, const Vector2D &rhs) { // 重载+运算符，实现向量的加法
      lhs += rhs;
      return lhs; ///< 返回相加后的新向量。
    }

    Vector2D &operator-=(const Vector2D &rhs) { // 重载-=运算符，实现向量的减法
      x -= rhs.x;
      y -= rhs.y;
      return *this;  ///< 返回当前向量的引用，便于链式操作。
    }

    friend Vector2D operator-(Vector2D lhs, const Vector2D &rhs) { // 重载-运算符，实现向量的减法
      lhs -= rhs;
      return lhs; ///< 返回相减后的新向量。
    }

    Vector2D &operator*=(float rhs) {  // 重载*=运算符，实现向量与标量的乘法
      x *= rhs;
      y *= rhs;
      return *this; ///< 返回当前向量的引用，便于链式操作。
    }

    friend Vector2D operator*(Vector2D lhs, float rhs) { // 重载*运算符，实现向量与标量的乘法
      lhs *= rhs;
      return lhs;  ///< 返回乘法后的新向量。
    }

    friend Vector2D operator*(float lhs, Vector2D rhs) { // 重载*运算符，实现标量与向量的乘法
      rhs *= lhs;
      return rhs; ///< 返回乘法后的新向量。
    }

    Vector2D &operator/=(float rhs) { // 重载/=运算符，实现向量与标量的除法
      x /= rhs;
      y /= rhs;
      return *this;  ///< 返回当前向量的引用，便于链式操作。
    }

    friend Vector2D operator/(Vector2D lhs, float rhs) { // 重载/运算符，实现向量与标量的除法
      lhs /= rhs;
      return lhs; ///< 返回除法后的新向量。
    }

    friend Vector2D operator/(float lhs, Vector2D rhs) { // 重载/运算符，实现标量与向量的除法
      rhs /= lhs;
      return rhs; ///< 返回除法后的新向量。
    }

    // =========================================================================
    // -- 比较运算符 -------------------------------------------------
    // =========================================================================

    bool operator==(const Vector2D &rhs) const { // 重载==运算符，比较两个向量是否相等
      return (x == rhs.x) && (y == rhs.y); ///< 如果x和y都相等，返回true。
    }

    bool operator!=(const Vector2D &rhs) const { // 重载!=运算符，比较两个向量是否不相等
      return !(*this == rhs); ///< 如果向量不相等，返回true。
    }

    // =========================================================================
    // -- 转换为UE4类型 ---------------------------------------------
    // =========================================================================

#ifdef LIBCARLA_INCLUDED_FROM_UE4

    /// 将向量从厘米转换为米
    Vector2D ToMeters() const {
      return *this * 1e-2f; ///< 当前向量的每个分量都乘以1e-2（厘米转米）。
    }

    /// 将向量从米转换为厘米、适用于UE4。
    Vector2D ToCentimeters() const {
      return *this * 1e2f; ///< 当前向量的每个分量都乘以1e2（米转厘米）。
    }

    /// 将向量转换为UE4引擎的FVector2D类型
    FVector2D ToFVector2D() const {
      return FVector2D{x, y}; ///< 返回一个包含x和y的FVector2D对象。
    }

#endif // LIBCARLA_INCLUDED_FROM_UE4

    MSGPACK_DEFINE_ARRAY(x, y) ///< 使用MsgPack库序列化向量的x和y分量，便于网络通信或数据存储。
  };

} // namespace geom
} // namespace carla
