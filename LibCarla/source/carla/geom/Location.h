// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// 位置类
// 
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/geom/Vector3D.h"
#include "carla/geom/Vector3DInt.h"
#include "carla/geom/Math.h"

#ifdef LIBCARLA_INCLUDED_FROM_UE4
#include <compiler/enable-ue4-macros.h>
#include "Math/Vector.h"
#include <compiler/disable-ue4-macros.h>
#endif // LIBCARLA_INCLUDED_FROM_UE4

namespace carla {
namespace geom {

  class Location : public Vector3D {
  public:

    // =========================================================================
    // -- 构造函数 -------------------------------------------------------------
    // =========================================================================

    Location() = default;

    using Vector3D::Vector3D;

    Location(const Vector3D &rhs) : Vector3D(rhs) {}

    Location(const Vector3DInt &rhs) :
        Vector3D(static_cast<float>(rhs.x),
                 static_cast<float>(rhs.y),
                 static_cast<float>(rhs.z)) {}

    // =========================================================================
    // -- 其他方法 --------------------------------------------------------------
    // =========================================================================

    auto DistanceSquared(const Location &loc) const {
      return Math::DistanceSquared(*this, loc);
    }

    auto Distance(const Location &loc) const {
      return Math::Distance(*this, loc);
    }

    // =========================================================================
    // -- 算术运算符 ------------------------------------------------------------
    // =========================================================================

    Location &operator+=(const Location &rhs) {
      static_cast<Vector3D &>(*this) += rhs;
      return *this;
    }

    friend Location operator+(Location lhs, const Location &rhs) {
      lhs += rhs;
      return lhs;
    }

    Location &operator-=(const Location &rhs) {
      static_cast<Vector3D &>(*this) -= rhs;
      return *this;
    }

    friend Location operator-(Location lhs, const Location &rhs) {
      lhs -= rhs;
      return lhs;
    }

    // =========================================================================
    // -- 比较运算符 ------------------------------------------------------------
    // =========================================================================

    // 等于
    bool operator==(const Location &rhs) const {
      return static_cast<const Vector3D &>(*this) == rhs;
    }

    // 不等于
    bool operator!=(const Location &rhs) const {
      return !(*this == rhs);
    }

    // =========================================================================
    // -- 转换为 UE4 类型 -------------------------------------------------------
    // =========================================================================

#ifdef LIBCARLA_INCLUDED_FROM_UE4

    Location(const FVector &vector) // 从厘米到米（乘以0.01）。
      : Location(1e-2f * vector.X, 1e-2f * vector.Y, 1e-2f * vector.Z) {}

    operator FVector() const {
      return FVector{1e2f * x, 1e2f * y, 1e2f * z}; // 从米到厘米（乘以100）。
    }

#endif // LIBCARLA_INCLUDED_FROM_UE4
  };

} // namespace geom
} // namespace carla
