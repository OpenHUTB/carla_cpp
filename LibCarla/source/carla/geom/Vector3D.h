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
namespace geom {

  // 三维向量类
  class Vector3D {
  public:

    // =========================================================================
    // -- 公开数据成员 ----------------------------------------------------------
    // =========================================================================

    float x = 0.0f;

    float y = 0.0f;

    float z = 0.0f;

    // =========================================================================
    // -- 构造函数 --------------------------------------------------------------
    // =========================================================================

    Vector3D() = default;

    Vector3D(float ix, float iy, float iz)
      : x(ix),
        y(iy),
        z(iz) {}

    // =========================================================================
    // -- 其他方法 --------------------------------------------------------------
    // =========================================================================

    float SquaredLength() const {
      return x * x + y * y + z * z;
    }

    float Length() const {
       return std::sqrt(SquaredLength());
    }

    float SquaredLength2D() const {
      return x * x + y * y;
    }

    float Length2D() const {
      return std::sqrt(SquaredLength2D());
    }

    Vector3D Abs() const {
       return Vector3D(abs(x), abs(y), abs(z));
    }

    Vector3D MakeUnitVector() const {
      const float length = Length();
      DEVELOPMENT_ASSERT(length > 2.0f * std::numeric_limits<float>::epsilon());
      const float k = 1.0f / length;
      return Vector3D(x * k, y * k, z * k);
    }

    Vector3D MakeSafeUnitVector(const float epsilon) const  {
      const float length = Length();
      const float k = (length > std::max(epsilon, 0.0f)) ? (1.0f / length) : 1.0f;
      return Vector3D(x * k, y * k, z * k);
    }

    // =========================================================================
    // -- 算术运算符 ------------------------------------------------------------
    // =========================================================================

    Vector3D &operator+=(const Vector3D &rhs) {
      x += rhs.x;
      y += rhs.y;
      z += rhs.z;
      return *this;
    }

    friend Vector3D operator+(Vector3D lhs, const Vector3D &rhs) {
      lhs += rhs;
      return lhs;
    }

    Vector3D &operator-=(const Vector3D &rhs) {
      x -= rhs.x;
      y -= rhs.y;
      z -= rhs.z;
      return *this;
    }

    friend Vector3D operator-(Vector3D lhs, const Vector3D &rhs) {
      lhs -= rhs;
      return lhs;
    }

    Vector3D& operator-=(const float f) {
      x -= f;
      y -= f;
      z -= f;
      return *this;
    }

    Vector3D &operator*=(float rhs) {
      x *= rhs;
      y *= rhs;
      z *= rhs;
      return *this;
    }

    friend Vector3D operator*(Vector3D lhs, float rhs) {
      lhs *= rhs;
      return lhs;
    }

    friend Vector3D operator*(float lhs, Vector3D rhs) {
      rhs *= lhs;
      return rhs;
    }

    Vector3D &operator/=(float rhs) {
      x /= rhs;
      y /= rhs;
      z /= rhs;
      return *this;
    }

    friend Vector3D operator/(Vector3D lhs, float rhs) {
      lhs /= rhs;
      return lhs;
    }

    friend Vector3D operator/(float lhs, Vector3D rhs) {
      rhs /= lhs;
      return rhs;
    }

    // =========================================================================
    // -- 比较运算符 ------------------------------------------------------------
    // =========================================================================

    bool operator==(const Vector3D &rhs) const {
      return (x == rhs.x) && (y == rhs.y) && (z == rhs.z);
    }

    bool operator!=(const Vector3D &rhs) const {
      return !(*this == rhs);
    }

    // =========================================================================
    // -- 转换为 UE4 类型 -------------------------------------------------------
    // =========================================================================

#ifdef LIBCARLA_INCLUDED_FROM_UE4

    /// 明确删除了这两种方法，以避免其他用户创建它们，与位置不同，有些矢量有单位，有些没有，
    /// 通过删除这些方法，我们发现有几个地方缺少从厘米到米的转换
    Vector3D(const FVector &v) = delete;
    Vector3D& operator=(const FVector &rhs) = delete;

    /// 返回从厘米转换为米的 Vector3D。
    Vector3D ToMeters() const {
      return *this * 1e-2f;
    }

    /// 返回从米转换为厘米的 Vector3D。
    Vector3D ToCentimeters() const {
      return *this * 1e2f;
    }

    FVector ToFVector() const {
      return FVector{x, y, z};
    }

#endif // LIBCARLA_INCLUDED_FROM_UE4

    // =========================================================================
    /// @todo 以下内容是从 MSGPACK_DEFINE_ARRAY 复制粘贴的。这是 msgpack 库中问题的解决方法。
    /// MSGPACK_DEFINE_ARRAY 宏正在遮蔽我们的“z”变量。
    /// https://github.com/msgpack/msgpack-c/issues/709
    // =========================================================================
    template <typename Packer>
    void msgpack_pack(Packer& pk) const
    {
        clmdep_msgpack::type::make_define_array(x, y, z).msgpack_pack(pk);
    }
    void msgpack_unpack(clmdep_msgpack::object const& o)
    {
        clmdep_msgpack::type::make_define_array(x, y, z).msgpack_unpack(o);
    }
    template <typename MSGPACK_OBJECT>
    void msgpack_object(MSGPACK_OBJECT* o, clmdep_msgpack::zone& sneaky_variable_that_shadows_z) const
    {
        clmdep_msgpack::type::make_define_array(x, y, z).msgpack_object(o, sneaky_variable_that_shadows_z);
    }
    // =========================================================================
  };

} // namespace geom
} // namespace carla
