// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once // 防止重复包含头文件

#include "carla/Debug.h" // 引入调试相关的头文件
#include "carla/geom/Vector3D.h" // 引入三维向量相关的头文件

#include <cmath> // 引入数学库
#include <type_traits> // 引入类型特征库
#include <utility> // 引入通用工具库

namespace carla { // 定义 carla 命名空间
namespace geom { // 定义 geom 子命名空间

  class Rotation; // 前向声明 Rotation 类

  class Math { // 定义 Math 类
  public:

    // 返回浮点数类型的 π 值
    template <typename T>
    static constexpr T Pi() {
      static_assert(std::is_floating_point<T>::value, "type must be floating point"); // 确保 T 是浮点数类型
      return static_cast<T>(3.14159265358979323846264338327950288); // 返回 π 的值
    }

    // 返回浮点数类型的 2π 值
    template <typename T>
    static constexpr T Pi2() {
      static_assert(std::is_floating_point<T>::value, "type must be floating point"); // 确保 T 是浮点数类型
      return static_cast<T>(static_cast<T>(2) * Pi<T>()); // 返回 2π 的值
    }

    // 将弧度转换为度数
    template <typename T>
    static constexpr T ToDegrees(T rad) {
      static_assert(std::is_floating_point<T>::value, "type must be floating point"); // 确保 T 是浮点数类型
      return rad * (T(180.0) / Pi<T>()); // 转换弧度到度数
    }

    // 将度数转换为弧度
    template <typename T>
    static constexpr T ToRadians(T deg) {
      static_assert(std::is_floating_point<T>::value, "type must be floating point"); // 确保 T 是浮点数类型
      return deg * (Pi<T>() / T(180.0)); // 转换度数到弧度
    }

    // 限制值在指定范围内
    template <typename T>
    static T Clamp(T a, T min = T(0), T max = T(1)) {
      return std::min(std::max(a, min), max); // 返回限制后的值
    }

    // 计算平方值
    template <typename T>
    static T Square(const T &a) {
      return a * a; // 返回 a 的平方
    }

    // 计算两个三维向量的叉积
    static auto Cross(const Vector3D &a, const Vector3D &b) {
      return Vector3D(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x); // 返回叉积结果
    }

    // 计算两个三维向量的点积
    static auto Dot(const Vector3D &a, const Vector3D &b) {
      return a.x * b.x + a.y * b.y + a.z * b.z; // 返回点积结果
    }

    // 计算两个二维向量的点积
    static auto Dot2D(const Vector3D &a, const Vector3D &b) {
      return a.x * b.x + a.y * b.y; // 返回二维点积结果
    }

    // 计算两个三维点之间的距离平方
    static auto DistanceSquared(const Vector3D &a, const Vector3D &b) {
      return Square(b.x - a.x) + Square(b.y - a.y) + Square(b.z - a.z); // 返回距离平方
    }

    // 计算两个二维点之间的距离平方
    static auto DistanceSquared2D(const Vector3D &a, const Vector3D &b) {
      return Square(b.x - a.x) + Square(b.y - a.y); // 返回二维距离平方
    }

    // 计算两个三维点之间的距离
    static auto Distance(const Vector3D &a, const Vector3D &b) {
      return std::sqrt(DistanceSquared(a, b)); // 返回距离
    }

    // 计算两个二维点之间的距离
    static auto Distance2D(const Vector3D &a, const Vector3D &b) {
      return std::sqrt(DistanceSquared2D(a, b)); // 返回二维距离
    }

    // 线性插值计算
    static float LinearLerp(float a, float b, float f) {
      return a * (1.0f - f) + (b * f); // 返回插值结果
    }

    /// 返回两个向量之间的夹角（弧度）
    static double GetVectorAngle(const Vector3D &a, const Vector3D &b);

    /// 计算点到线段的距离
    /// 返回一个包含：
    /// - @b first: 从 v 到 p' 的距离，其中 p' = p 投影到线段 (w - v) 上
    /// - @b second: 从 p 到 p' 的欧几里得距离
    /// @param p 要计算距离的点
    /// @param v 线段的第一个点
    /// @param w 线段的第二个点
    static std::pair<float, float> DistanceSegmentToPoint(
        const Vector3D &p,
        const Vector3D &v,
        const Vector3D &w);

    /// 计算点到弧的距离
    /// 返回一个包含：
    /// - @b first: 从 start_pos 到 p' 的弧长距离，其中 p' = p 投影到弧上
    /// - @b second: 从 p 到 p' 的欧几里得距离
    static std::pair<float, float> DistanceArcToPoint(
        Vector3D p,
        Vector3D start_pos,
        float length,
        float heading,   // [radians]
        float curvature);

    // 在原点旋转二维点
    static Vector3D RotatePointOnOrigin2D(Vector3D p, float angle);

    /// 计算指向 @a rotation 的 X 轴的单位向量
    static Vector3D GetForwardVector(const Rotation &rotation);

    /// 计算指向 @a rotation 的 Y 轴的单位向量
    static Vector3D GetRightVector(const Rotation &rotation);

    /// 计算指向 @a rotation 的 Z 轴的单位向量
    static Vector3D GetUpVector(const Rotation &rotation);

    // 生成从 a 到 b 的连续整数向量
    static std::vector<int> GenerateRange(int a, int b);

  };
} // namespace geom
} // namespace carla
