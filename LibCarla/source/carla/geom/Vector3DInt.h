// Copyright (c) 2021 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/MsgPack.h" // 包含消息打包库

#include <cmath>
#include <limits>

namespace carla {
namespace geom {

  class Vector3DInt {
  public:

    // =========================================================================
    // -- 公共数据成员 --------------------------------------------------
    // =========================================================================

    int32_t x = 0; // X轴的整数值，默认为0

    int32_t y = 0; // Y轴的整数值，默认为0

    int32_t z = 0; // Z轴的整数值，默认为0

    // =========================================================================
    // -- 构造函数 ---------------------------------------------------------
    // =========================================================================

    Vector3DInt() = default;

    Vector3DInt(int32_t ix, int32_t iy, int32_t iz)
      : x(ix),
        y(iy),
        z(iz) {} // 参数化构造函数，初始化x、y、z

    // =========================================================================
    // -- 其他方法 --------------------------------------------------------
    // =========================================================================

    int64_t SquaredLength() const { // 计算向量的平方长度
      return x * x + y * y + z * z;
    }

    double Length() const { // 计算向量的长度
       return std::sqrt(SquaredLength());
    }

    // =========================================================================
    // -- 算术运算符 -------------------------------------------------
    // =========================================================================

    Vector3DInt &operator+=(const Vector3DInt &rhs) { // 加法赋值运算符
      x += rhs.x;
      y += rhs.y;
      z += rhs.z;
      return *this;
    }

    friend Vector3DInt operator+(Vector3DInt lhs, const Vector3DInt &rhs) { // 加法运算符
      lhs += rhs;
      return lhs;
    }

    Vector3DInt &operator-=(const Vector3DInt &rhs) { // 减法赋值运算符
      x -= rhs.x;
      y -= rhs.y;
      z -= rhs.z;
      return *this;
    }

    friend Vector3DInt operator-(Vector3DInt lhs, const Vector3DInt &rhs) { // 减法运算符
      lhs -= rhs;
      return lhs;
    }

    Vector3DInt &operator*=(int32_t rhs) { // 乘法赋值运算符
      x *= rhs;
      y *= rhs;
      z *= rhs;
      return *this;
    }

    friend Vector3DInt operator*(Vector3DInt lhs, int32_t rhs) { // 乘法运算符
      lhs *= rhs;
      return lhs;
    }

    friend Vector3DInt operator*(int32_t lhs, Vector3DInt rhs) { // 乘法运算符
      rhs *= lhs;
      return rhs;
    }

    Vector3DInt &operator/=(int32_t rhs) { // 除法赋值运算符
      x /= rhs;
      y /= rhs;
      z /= rhs;
      return *this;
    }

    friend Vector3DInt operator/(Vector3DInt lhs, int32_t rhs) { // 除法运算符
      lhs /= rhs;
      return lhs;
    }

    friend Vector3DInt operator/(int32_t lhs, Vector3DInt rhs) { // 除法运算符
      rhs /= lhs;
      return rhs;
    }

    // =========================================================================
    // -- 比较运算符 -------------------------------------------------
    // =========================================================================

    bool operator==(const Vector3DInt &rhs) const { // 等于运算符
      return (x == rhs.x) && (y == rhs.y) && (z == rhs.z);
    }

    bool operator!=(const Vector3DInt &rhs) const { // 不等于运算符
      return !(*this == rhs);
    }

    // =========================================================================
    // -- 转换为UE4类型 ---------------------------------------------
    // =========================================================================

#ifdef LIBCARLA_INCLUDED_FROM_UE4

    Vector3DInt(const FIntVector &v) = delete; // 禁止从UE4的FIntVector转换
    Vector3DInt& operator=(const FIntVector &rhs) = delete; // 禁止赋值UE4的FIntVector

    /// 将厘米转换为米
    Vector3DInt ToMeters() const {
      return *this / 100;
    }

    ///将米转换为厘米
    Vector3DInt ToCentimeters() const {
      return *this * 100;
    }

    FIntVector ToFIntVector() const { // 转换为UE4的FIntVector
      return FIntVector{x, y, z};
    }

#endif // LIBCARLA_INCLUDED_FROM_UE4

    // =========================================================================
    /// 待办事项：以下是从MSGPACK_DEFINE_ARRAY复制粘贴过来的。
    /// 这是解决msgpack库中一个问题的临时方法。
    /// MSGPACK_DEFINE_ARRAY宏正在遮蔽我们的z变量。
    /// https://github.com/msgpack/msgpack-c/issues/709
    // =========================================================================
    template <typename Packer>
    void msgpack_pack(Packer& pk) const // 消息打包函数
    {
        clmdep_msgpack::type::make_define_array(x, y, z).msgpack_pack(pk);
    }
    void msgpack_unpack(clmdep_msgpack::object const& o) // 消息解包函数
    {
        clmdep_msgpack::type::make_define_array(x, y, z).msgpack_unpack(o);
    }
    template <typename MSGPACK_OBJECT>
    void msgpack_object(MSGPACK_OBJECT* o, clmdep_msgpack::zone& sneaky_variable_that_shadows_z) const  // 消息对象函数
    {
        clmdep_msgpack::type::make_define_array(x, y, z).msgpack_object(o, sneaky_variable_that_shadows_z);
    }
    // =========================================================================
  };

} // namespace geom
} // namespace carla
