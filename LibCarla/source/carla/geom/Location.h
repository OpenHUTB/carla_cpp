// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// 位置类
// 
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once // 这是一个预处理指令，作用是确保该头文件在整个编译过程中只会被包含一次，
// 避免因多次包含同一头文件而导致的重复定义等编译错误。

#include "carla/geom/Vector3D.h" // 包含名为"carla/geom/Vector3D.h"的头文件，通常这个头文件中会定义三维向量相关的结构体、类或者函数等，
// 可能是用于表示空间中的向量信息，是当前代码实现功能所依赖的外部声明所在的头文件之一，
// 这里的位置类可能会基于三维向量的相关概念来构建其功能。
#include "carla/geom/Vector3DInt.h" // 包含名为"carla/geom/Vector3DInt.h"的头文件，推测里面定义了整数类型的三维向量相关内容，
// 可能用于在某些特定场景下（比如涉及到坐标的整数表示等情况）与位置相关的操作，同样是当前代码依赖的外部声明所在的头文件。
#include "carla/geom/Math.h"

#ifdef LIBCARLA_INCLUDED_FROM_UE4 // 如果从UE4中包含了LIBCARLA，则启用UE4宏
#include <compiler/enable-ue4-macros.h> // 启用UE4宏支持
#include "Math/Vector.h" // 引入UE4中用于处理向量的头文件
#include <compiler/disable-ue4-macros.h> // 禁用UE4宏支持
#endif // LIBCARLA_INCLUDED_FROM_UE4

namespace carla {
namespace geom {

  class Location : public Vector3D {
  public:

    // =========================================================================
    // -- 构造函数 -------------------------------------------------------------
    // =========================================================================

    Location() = default; // 默认构造函数，使用Vector3D的默认构造函数初始化。

    // 通过"using"声明将Vector3D的构造函数引入Location类，使得Location类可以直接使用Vector3D的构造函数。
    using Vector3D::Vector3D; 

    // 拷贝构造函数，将一个Vector3D类型的对象初始化为Location对象。
    Location(const Vector3D &rhs) : Vector3D(rhs) {} 

    // 通过Vector3D的构造函数将一个Vector3DInt类型的对象转换为Location对象。
    Location(const Vector3DInt &rhs) :
        Vector3D(static_cast<float>(rhs.x),
                 static_cast<float>(rhs.y),
                 static_cast<float>(rhs.z)) {}

    // =========================================================================
    // -- 其他方法 --------------------------------------------------------------
    // =========================================================================

    // 计算当前Location与另一个Location之间的平方距离。
    auto DistanceSquared(const Location &loc) const {
      return Math::DistanceSquared(*this, loc);
    }

    // 计算当前Location与另一个Location之间的欧几里得距离。
    auto Distance(const Location &loc) const {
      return Math::Distance(*this, loc);
    }

    // =========================================================================
    // -- 算术运算符 ------------------------------------------------------------
    // =========================================================================

    // 实现 += 运算符，用于Location与Location的加法运算。
    Location &operator+=(const Location &rhs) {
      static_cast<Vector3D &>(*this) += rhs;
      return *this;
    }

    // 实现 + 运算符，返回Location与Location加法运算的结果。
    friend Location operator+(Location lhs, const Location &rhs) {
      lhs += rhs;
      return lhs;
    }

    // 实现 -= 运算符，用于Location与Location的减法运算。
    Location &operator-=(const Location &rhs) {
      static_cast<Vector3D &>(*this) -= rhs;
      return *this;
    }

    // 实现 - 运算符，返回Location与Location减法运算的结果。
    friend Location operator-(Location lhs, const Location &rhs) {
      lhs -= rhs;
      return lhs;
    }

    // =========================================================================
    // -- 比较运算符 ------------------------------------------------------------
    // =========================================================================

    // 判断两个Location是否相等。
    bool operator==(const Location &rhs) const {
      return static_cast<const Vector3D &>(*this) == rhs;
    }

    // 判断两个Location是否不相等。
    bool operator!=(const Location &rhs) const {
      return !(*this == rhs);
    }

    // =========================================================================
    // -- 转换为 Unreal Engine 4 类型 ----------------------------------------
    // =========================================================================

#ifdef LIBCARLA_INCLUDED_FROM_UE4
    // 从FVector类型转换到Location，假设输入的FVector单位是厘米（cm），转换为米（m）。
    Location(const FVector &vector) // 将FVector中的数据从厘米转换为米（乘以0.01）。
      : Location(1e-2f * vector.X, 1e-2f * vector.Y, 1e-2f * vector.Z) {}

    // 将Location类型转换为FVector，假设目标输出单位为厘米（cm），将米转换为厘米（乘以100）。
    operator FVector() const {
      return FVector{1e2f * x, 1e2f * y, 1e2f * z}; // 将米转换为厘米（乘以100）。
    }

#endif // LIBCARLA_INCLUDED_FROM_UE4
  };

} // namespace geom
} // namespace carla
