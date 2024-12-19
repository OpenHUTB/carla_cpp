// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
// 遵循MIT许可协议，查看许可协议链接
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/MsgPack.h"
#include <cmath>
#include <limits>

namespace carla {
namespace geom {

// 三维向量类，用于表示三维空间中的向量，包含了一系列针对该向量的操作方法
class Vector3D {
public:
    // =========================================================================
    // -- 公开数据成员 ----------------------------------------------------------
    // =========================================================================

    // x轴分量
    float x = 0.0f;
    // y轴分量
    float y = 0.0f;
    // z轴分量
    float z = 0.0f;

    // =========================================================================
    // -- 构造函数 --------------------------------------------------------------
    // =========================================================================

    // 默认构造函数，使用默认初始化，各分量为0.0f
    Vector3D() = default;

    // 带参数的构造函数，用于初始化向量的x、y、z轴分量
    Vector3D(float ix, float iy, float iz)
        : x(ix),
          y(iy),
          z(iz) {}

    // =========================================================================
    // -- 其他方法 --------------------------------------------------------------
    // =========================================================================

    // 计算向量的长度的平方
    float SquaredLength() const {
        return x * x + y * y + z * z;
    }

    // 计算向量的长度（即模长），通过先求长度的平方再开方得到
    float Length() const {
        return std::sqrt(SquaredLength());
    }

    // 计算二维平面下向量长度的平方（只考虑x、y轴分量）
    float SquaredLength2D() const {
        return x * x + y * y;
    }

    // 计算二维平面下向量的长度（只考虑x、y轴分量），通过先求二维长度的平方再开方得到
    float Length2D() const {
        return std::sqrt(SquaredLength2D());
    }

    // 获取向量各分量取绝对值后的新向量
    Vector3D Abs() const {
        return Vector3D(abs(x), abs(y), abs(z));
    }

    // 将向量转换为单位向量，先获取向量长度，确保长度大于一定极小值（避免除以0等异常情况），然后各分量除以长度得到单位向量
    Vector3D MakeUnitVector() const {
        const float length = Length();
        DEVELOPMENT_ASSERT(length > 2.0f * std::numeric_limits<float>::epsilon());
        const float k = 1.0f / length;
        return Vector3D(x * k, y * k, z * k);
    }

    // 将向量转换为安全的单位向量，根据给定的epsilon值判断长度情况，若长度大于epsilon则按正常方式转换为单位向量，否则直接返回原向量（各分量乘以1.0f，相当于不变）
    Vector3D MakeSafeUnitVector(const float epsilon) const {
        const float length = Length();
        const float k = (length > std::max(epsilon, 0.0f))? (1.0f / length) : 1.0f;
        return Vector3D(x * k, y * k, z * k);
    }

    // =========================================================================
    // -- 算术运算符 ------------------------------------------------------------
    // =========================================================================

    // 重载 += 运算符，实现向量与另一个向量相加并更新自身，将对应分量相加
    Vector3D &operator+=(const Vector3D &rhs) {
        x += rhs.x;
        y += rhs.y;
        z += rhs.z;
        return *this;
    }

    // 重载 + 运算符，实现向量相加，通过调用 += 运算符实现，返回相加后的新向量
    friend Vector3D operator+(Vector3D lhs, const Vector3D &rhs) {
        lhs += rhs;
        return lhs;
    }

    // 重载 -= 运算符，实现向量与另一个向量相减并更新自身，将对应分量相减
    Vector3D &operator-=(const Vector3D &rhs) {
        x -= rhs.x;
        y -= rhs.y;
        z -= rhs.z;
        return *this;
    }

    // 重载 - 运算符，实现向量相减，通过调用 -= 运算符实现，返回相减后的新向量
    friend Vector3D operator-(Vector3D lhs, const Vector3D &rhs) {
        lhs -= rhs;
        return lhs;
    }

    // 重载 -= 运算符，实现向量减去一个浮点数并更新自身，各分量减去该浮点数
    Vector3D& operator-=(const float f) {
        x -= f;
        y -= f;
        z -= f;
        return *this;
    }

    // 重载 *= 运算符，实现向量与一个浮点数相乘并更新自身，各分量乘以该浮点数
    Vector3D &operator*=(float rhs) {
        x *= rhs;
        y *= rhs;
        z *= rhs;
        return *this;
    }

    // 重载 * 运算符，实现向量与一个浮点数相乘，通过调用 *= 运算符实现，返回相乘后的新向量
    friend Vector3D operator*(Vector3D lhs, float rhs) {
        lhs *= rhs;
        return lhs;
    }

    // 重载 * 运算符，实现浮点数与一个向量相乘，通过调用向量的 *= 运算符实现，返回相乘后的新向量
    friend Vector3D operator*(float lhs, Vector3D rhs) {
        rhs *= lhs;
        return rhs;
    }

    // 重载 /= 运算符，实现向量与一个浮点数相除并更新自身，各分量除以该浮点数
    Vector3D &operator/=(float rhs) {
        x /= rhs;
        y /= rhs;
        z /= rhs;
        return *this;
    }

    // 重载 / 运算符，实现向量与一个浮点数相除，通过调用 /= 运算符实现，返回相除后的新向量
    friend Vector3D operator/(Vector3D lhs, float rhs) {
        lhs /= rhs;
        return lhs;
    }

    // 重载 / 运算符，实现浮点数与一个向量相除，通过调用向量的 /= 运算符实现，返回相除后的新向量（这里可能不符合常规数学意义，只是按照代码逻辑实现的对应操作）
    friend Vector3D operator/(float lhs, Vector3D rhs) {
        rhs /= lhs;
        return rhs;
    }

    // =========================================================================
    // -- 比较运算符 ------------------------------------------------------------
    // =========================================================================

    // 重载 == 运算符，比较两个向量是否相等，通过比较各对应分量是否相等来判断
    bool operator==(const Vector3D &rhs) const {
        return (x == rhs.x) && (y == rhs.y) && (z == rhs.z);
    }

    // 重载!= 运算符，比较两个向量是否不相等，通过对 == 运算符结果取反来判断
    bool operator!=(const Vector3D &rhs) const {
        return!(*this == rhs);
    }

    // =========================================================================
    // -- 转换为 UE4 类型 -------------------------------------------------------
    // =========================================================================

#ifdef LIBCARLA_INCLUDED_FROM_UE4
    // 明确删除了这两种构造函数和赋值运算符重载，目的是避免其他用户错误创建它们，因为向量与位置不同，有些向量有单位，有些没有，删除这些可发现缺少单位转换的地方
    Vector3D(const FVector &v) = delete;
    Vector3D& operator=(const FVector &rhs) = delete;

    // 将向量的单位从厘米转换为米并返回新向量，通过各分量乘以1e-2f实现
    Vector3D ToMeters() const {
        return *this * 1e-2f;
    }

    // 将向量的单位从米转换为厘米并返回新向量，通过各分量乘以1e2f实现
    Vector3D ToCentimeters() const {
        return *this * 1e2f;
    }

    // 将该Vector3D类型向量转换为UE4中的FVector类型并返回
    FVector ToFVector() const {
        return FVector{x, y, z};
    }
#endif // LIBCARLA_INCLUDED_FROM_UE4

    // =========================================================================
    /// @todo 以下内容是从 MSGPACK_DEFINE_ARRAY 复制粘贴的。这是 msgpack 库中问题的解决方法。
    /// MSGPACK_DEFINE_ARRAY 宏正在遮蔽我们的“z”变量。
    /// https://github.com/msgpack/msgpack-c/issues/709
    // =========================================================================
    // 用于将该向量对象进行msgpack序列化，调用相关函数实现序列化操作
    template <typename Packer>
    void msgpack_pack(Packer& pk) const
    {
        clmdep_msgpack::type::make_define_array(x, y, z).msgpack_pack(pk);
    }
    // 用于将msgpack序列化的数据进行反序列化，恢复该向量对象
    void msgpack_unpack(clmdep_msgpack::object const& o)
    {
        clmdep_msgpack::type::make_define_array(x, y, z).msgpack_unpack(o);
    }
    // 用于处理msgpack对象相关操作（可能涉及到更复杂的msgpack使用场景下的对象操作）
    template <typename MSGPACK_OBJECT>
    void msgpack_object(MSGPACK_OBJECT* o, clmdep_msgpack::zone& sneaky_variable_that_shadows_z) const
    {
        clmdep_msgpack::type::make_define_array(x, y, z).msgpack_object(o, sneaky_variable_that_shadows_z);
    }
    // =========================================================================
};

} // namespace geom
} // namespace carla
