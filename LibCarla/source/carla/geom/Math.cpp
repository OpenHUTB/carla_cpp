// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/geom/Math.h"

#include "carla/geom/Rotation.h"

namespace carla {
namespace geom {

  double Math::GetVectorAngle(const Vector3D &a, const Vector3D &b) {
    return std::acos(Dot(a, b) / (a.Length() * b.Length()));
  }

  std::pair<float, float> Math::DistanceSegmentToPoint(
      const Vector3D &p,
      const Vector3D &v,
      const Vector3D &w) {
    const float l2 = DistanceSquared2D(v, w);
    const float l = std::sqrt(l2);
    if (l2 == 0.0f) {
      return std::make_pair(0.0f, Distance2D(v, p));
    }
    const float dot_p_w = Dot2D(p - v, w - v);
    const float t = Clamp(dot_p_w / l2);
    const Vector3D projection = v + t * (w - v);
    return std::make_pair(t * l, Distance2D(projection, p));
  }

  std::pair<float, float> Math::DistanceArcToPoint(
      Vector3D p,
      Vector3D start_pos,
      const float length,
      float heading,       // [弧度]
      float curvature) {

    /// @todo: 因为虚幻的坐标，纠正-y的方法是比较老套的，这个以后必须改掉
    p.y = -p.y;
    start_pos.y = -start_pos.y;
    heading = -heading;
    curvature = -curvature;

    // 由于此算法适用于正曲率，并且我们仅计算距离，
    // 因此我们可以反转 y 轴（以及曲率和航向），因此如果曲率为负，算法将按预期工作
    if (curvature < 0.0f) {
      p.y = -p.y;
      start_pos.y = -start_pos.y;
      heading = -heading;
      curvature = -curvature;
    }

    // 运输点(transport point)相对于圆弧起始位置的旋转
    const Vector3D rotated_p(RotatePointOnOrigin2D(p - start_pos, -heading));

    const float radius = 1.0f / curvature;
    const Vector3D circ_center(0.0f, radius, 0.0f);

    // 检查点是否位于圆心，因此我们知道 p 与圆弧上每个可能点的距离都相同
    if (rotated_p == circ_center) {
      return std::make_pair(0.0f, radius);  // std::make_pair 用于创建包含两个元素的 std::pair，元素类型可以不同
    }

    // 使用从圆心到该点的单位向量并乘以半径来找到交点位置
    const Vector3D intersection = ((rotated_p - circ_center).MakeUnitVector() * radius) + circ_center;

    // 使用弧长来计算最后一个点的角度
    // 圆的周长(circumference) = 2 * PI * r
    // 最后一个点的角度 last_point_angle = (length / circumference) * 2 * PI
    // 所以，last_point_angle = length / radius
    const float last_point_angle = length / radius;

    constexpr float pi_half = Pi<float>() / 2.0f;

    // 相对于圆心移动该点，并找到该点与 rad 中坐标中心之间的角度
    float angle = std::atan2(intersection.y - radius, intersection.x) + pi_half;

    if (angle < 0.0f) {
      angle += Pi<float>() * 2.0f;
    }

    // 查看角度是否在 0 和 last_point_angle 之间
    DEBUG_ASSERT(angle >= 0.0f);
    if (angle <= last_point_angle) {
      return std::make_pair(
          angle * radius,
          Distance2D(intersection, rotated_p));
    }

    // 找到距离交叉点最近的点、起点或终点
    const float start_dist = Distance2D(Vector3D(), rotated_p);

    const Vector3D end_pos(
        radius * std::cos(last_point_angle - pi_half),
        radius * std::sin(last_point_angle - pi_half) + circ_center.y,
        0.0f);
    const float end_dist = Distance2D(end_pos, rotated_p);
    return (start_dist < end_dist) ?
        std::make_pair(0.0f, start_dist) :
        std::make_pair(length, end_dist);
  }

  Vector3D Math::RotatePointOnOrigin2D(Vector3D p, float angle) {
    const float s = std::sin(angle);
    const float c = std::cos(angle);
    return Vector3D(p.x * c - p.y * s, p.x * s + p.y * c, 0.0f);
  }

  Vector3D Math::GetForwardVector(const Rotation &rotation) {
    const float cp = std::cos(ToRadians(rotation.pitch));
    const float sp = std::sin(ToRadians(rotation.pitch));
    const float cy = std::cos(ToRadians(rotation.yaw));
    const float sy = std::sin(ToRadians(rotation.yaw));
    return {cy * cp, sy * cp, sp};
  }

  Vector3D Math::GetRightVector(const Rotation &rotation) {
    const float cy = std::cos(ToRadians(rotation.yaw));
    const float sy = std::sin(ToRadians(rotation.yaw));
    const float cr = std::cos(ToRadians(rotation.roll));
    const float sr = std::sin(ToRadians(rotation.roll));
    const float cp = std::cos(ToRadians(rotation.pitch));
    const float sp = std::sin(ToRadians(rotation.pitch));
    return {
         cy * sp * sr - sy * cr,
         sy * sp * sr + cy * cr,
        -cp * sr};
  }

  Vector3D Math::GetUpVector(const Rotation &rotation) {
    const float cy = std::cos(ToRadians(rotation.yaw));
    const float sy = std::sin(ToRadians(rotation.yaw));
    const float cr = std::cos(ToRadians(rotation.roll));
    const float sr = std::sin(ToRadians(rotation.roll));
    const float cp = std::cos(ToRadians(rotation.pitch));
    const float sp = std::sin(ToRadians(rotation.pitch));
    return {
        -cy * sp * cr - sy * sr,
        -sy * sp * cr + cy * sr,
        cp * cr};
  }

  std::vector<int> Math::GenerateRange(int a, int b) {
    std::vector<int> result;
    if (a < b) {
      for(int i = a; i <= b; ++i) {
        result.push_back(i);
      }
    } else {
      for(int i = a; i >= b; --i) {
        result.push_back(i);
      }
    }
    return result;
  }

} // namespace geom
} // namespace carla
