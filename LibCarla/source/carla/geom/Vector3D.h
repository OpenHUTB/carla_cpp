// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
// 遵循MIT许可协议，查看许可协议链接
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

// 防止头文件被多重包含，确保每个头文件只被包含一次
#pragma once

// 引入 carla 库中的 MsgPack 相关头文件
#include "carla/MsgPack.h"

// 引入 C++ 标准库中的数学计算功能（例如：数学常数、三角函数等）
#include <cmath>

// 引入 C++ 标准库中的数字极限（例如：浮点数的最大最小值等）
#include <limits>

namespace carla {
namespace geom {

// 三维向量类，用于表示三维空间中的向量，包含了一系列针对该向量的操作方法
class Vector3D {
public:
    // =========================================================================
    // -- 公开数据成员 ----------------------------------------------------------
    // =========================================================================

    // x轴分量，表示向量在x轴方向上的大小
    float x = 0.0f;
    // y轴分量，表示向量在y轴方向上的大小
    float y = 0.0f;
    // z轴分量，表示向量在z轴方向上的大小
    float z = 0.0f;

    // =========================================================================
    // -- 构造函数 --------------------------------------------------------------
    // =========================================================================

    // 默认构造函数，初始化向量的x、y、z分量为0.0f
    Vector3D() = default;

    // 带参数的构造函数，用于初始化向量的x、y、z轴分量
    Vector3D(float ix, float iy, float iz)
        : x(ix),
          y(iy),
          z(iz) {}

    // =========================================================================
    // -- 其他方法 --------------------------------------------------------------
    // =========================================================================

    // 计算向量的平方长度（即模长的平方），避免开方操作，提高效率
    float SquaredLength() const {
        return x * x + y * y + z * z;
    }

    // 计算向量的模长（即向量的实际长度），通过平方根计算
    float Length() const {
        return std::sqrt(SquaredLength());
    }

    // 计算二维平面（只考虑x、y分量）下的向量长度的平方
    float SquaredLength2D() const {
        return x * x + y * y;
    }

    // 计算二维平面（只考虑x、y分量）下的向量长度
    float Length2D() const {
        return std::sqrt(SquaredLength2D());
    }

    // 获取该向量的绝对值向量，即各分量取绝对值后的新向量
    Vector3D Abs() const {
        return Vector3D(abs(x), abs(y), abs(z));
    }

    // 将该向量转换为单位向量（长度为1的向量），确保原向量的长度大于极小值避免除以0
    Vector3D MakeUnitVector() const {
        const float length = Length();
        DEVELOPMENT_ASSERT(length > 2.0f * std::numeric_limits<float>::epsilon());
        const float k = 1.0f / length;
        return Vector3D(x * k, y * k, z * k);
    }

    // 将该向量转换为安全的单位向量，若长度小于给定的epsilon，则返回原向量，否则返回单位向量
    Vector3D MakeSafeUnitVector(const float epsilon) const {
        const float length = Length();
        const float k = (length > std::max(epsilon, 0.0f))? (1.0f / length) : 1.0f;
        return Vector3D(x * k, y * k, z * k);
    }

    // =========================================================================
    // -- 算术运算符 ------------------------------------------------------------
    // =========================================================================

    // 重载 += 运算符，使得向量加法可以直接通过该运算符更新向量
    Vector3D &operator+=(const Vector3D &rhs) {
        x += rhs.x;
        y += rhs.y;
        z += rhs.z;
        return *this;
    }

    // 重载 + 运算符，实现向量相加，返回新向量
    friend Vector3D operator+(Vector3D lhs, const Vector3D &rhs) {
        lhs += rhs;
        return lhs;
    }

    // 重载 -= 运算符，使得向量减法可以直接通过该运算符更新向量
    Vector3D &operator-=(const Vector3D &rhs) {
        x -= rhs.x;
        y -= rhs.y;
        z -= rhs.z;
        return *this;
    }

    // 重载 - 运算符，实现向量相减，返回新向量
    friend Vector3D operator-(Vector3D lhs, const Vector3D &rhs) {
        lhs -= rhs;
        return lhs;
    }

    // 重载 -= 运算符，实现向量与一个常数相减，并更新向量
    Vector3D& operator-=(const float f) {
        x -= f;
        y -= f;
        z -= f;
        return *this;
    }

    // 重载 *= 运算符，实现向量与一个浮点数相乘并更新向量
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

    // 重载 == 运算符，判断两个向量是否相等，比较每个分量是否相等
    bool operator==(const Vector3D &rhs) const {
        return (x == rhs.x) && (y == rhs.y) && (z == rhs.z);
    }

    // 重载 != 运算符，判断两个向量是否不相等，取反 == 运算符的结果
    bool operator!=(const Vector3D &rhs) const {
        return!(*this == rhs);
    }

    // =========================================================================
    // -- 转换为 UE4 类型 -------------------------------------------------------
    // =========================================================================

#ifdef LIBCARLA_INCLUDED_FROM_UE4
    // 删除将 Vector3D 转换为 FVector 类型的构造函数和赋值运算符，避免误用
    Vector3D(const FVector &v) = delete;
    Vector3D& operator=(const FVector &rhs) = delete;

    // 将该向量从厘米单位转换为米单位，返回新向量
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
    /// MSGPACK_DEFINE_ARRAY 宏可能遮蔽了我们的“z”变量。
    /// 解决方法参考：https://github.com/msgpack/msgpack-c/issues/709
    // =========================================================================
    // 用于将 Vector3D 对象进行 msgpack 序列化，确保数据按顺序打包
    template <typename Packer>
    void msgpack_pack(Packer& pk) const
    {
        clmdep_msgpack::type::make_define_array(x, y, z).msgpack_pack(pk);
    }
    // 用于从 msgpack 数据反序列化，恢复 Vector3D 对象
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
