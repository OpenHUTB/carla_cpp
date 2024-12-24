// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
// 包含必要的CARLA头文件
#include "carla/Debug.h"
#include "carla/MsgPack.h"
#include "carla/geom/Transform.h"
#include "carla/geom/Location.h"
#include "carla/geom/Vector3D.h"

#include <array>
// 如果从UE4中包含CARLA库，则包含UE4的相关宏
#ifdef LIBCARLA_INCLUDED_FROM_UE4
#include <compiler/enable-ue4-macros.h>
#include "Carla/Util/BoundingBox.h"
#include <compiler/disable-ue4-macros.h>
#endif // LIBCARLA_INCLUDED_FROM_UE4

namespace carla {
namespace geom {
 // 定义一个边界框类，用于表示三维空间中的边界框。
  class BoundingBox { // 定义一个边界框类。
  public:
 // 默认构造函数，使用默认初始化。
    BoundingBox() = default;

    // =========================================================================
    // -- 构造函数 ---------------------------------------------------------
    // =========================================================================

    explicit BoundingBox(const Location &in_location, const Vector3D &in_extent, const Rotation &in_rotation)
      : location(in_location), // 边界框的中心位置
        extent(in_extent),     // 边界框的半大小
        rotation(in_rotation)  // 边界框的旋转
    {}

    explicit BoundingBox(const Location &in_location, const Vector3D &in_extent) // 构造一个边界框，只指定位置和大小。
      : location(in_location),  // 边界框的中心位置
        extent(in_extent),      // 边界框的半大小
        rotation()              // 默认旋转
    {}

    explicit BoundingBox(const Vector3D &in_extent) // 仅指定大小的构造函数。
      : location(),            // 默认位置
        extent(in_extent),     // 边界框的半大小
        rotation()             // 默认旋转
    {}

    // 成员变量定义了边界框的中心位置、半大小和旋转。
    Location location;  ///< 本地空间中边界框的中心
    Vector3D extent;    ///< 本地空间中边界框的一半大小
    Rotation rotation;  ///< 本地空间中边界框的旋转

    // =========================================================================
    // -- 其他方法 --------------------------------------------------------
    // =========================================================================

    /**
     * 检查世界空间中的某一点是否在该边界框内。
     * @paramin_world_point指向世界空间中要查询是否在内部的点。
     * @param in_bbox_to_world_transform 从边界框空间到世界空间的变换。
     */
    bool Contains(const Location &in_world_point, const Transform &in_bbox_to_world_transform) const {
        auto point_in_bbox_space = in_world_point;
        in_bbox_to_world_transform.InverseTransformPoint(point_in_bbox_space);
        point_in_bbox_space -= location;

        // 判断点是否在边界框的范围内
        return  point_in_bbox_space.x >= -extent.x && point_in_bbox_space.x <= extent.x &&
                point_in_bbox_space.y >= -extent.y && point_in_bbox_space.y <= extent.y &&
                point_in_bbox_space.z >= -extent.z && point_in_bbox_space.z <= extent.z;
    }

    /**
     *  返回本地空间中边界框的8个顶点的位置。
     */
    std::array<Location, 8> GetLocalVertices() const { // 定义顶点的局部位置

        return {{
            location + Location(rotation.RotateVector({-extent.x,-extent.y,-extent.z})),
            location + Location(rotation.RotateVector({-extent.x,-extent.y, extent.z})),
            location + Location(rotation.RotateVector({-extent.x, extent.y,-extent.z})),
            location + Location(rotation.RotateVector({-extent.x, extent.y, extent.z})),
            location + Location(rotation.RotateVector({ extent.x,-extent.y,-extent.z})),
            location + Location(rotation.RotateVector({ extent.x,-extent.y, extent.z})),
            location + Location(rotation.RotateVector({ extent.x, extent.y,-extent.z})),
            location + Location(rotation.RotateVector({ extent.x, extent.y, extent.z}))
        }};
    }

    /**
     *  返回本地空间中边界框的8个顶点的位置，但不包含自身的旋转。
     */
    std::array<Location, 8> GetLocalVerticesNoRotation() const { // 定义顶点的局部位置，不应用旋转

        return {{
            location + Location(-extent.x,-extent.y,-extent.z),
            location + Location(-extent.x,-extent.y, extent.z),
            location + Location(-extent.x, extent.y,-extent.z),
            location + Location(-extent.x, extent.y, extent.z),
            location + Location( extent.x,-extent.y,-extent.z),
            location + Location( extent.x,-extent.y, extent.z),
            location + Location( extent.x, extent.y,-extent.z),
            location + Location( extent.x, extent.y, extent.z)
        }};
    }

    /**
     * 返回世界空间中边界框的8个顶点的位置。
     * @param in_bbox_to_world_tr 从边界框空间到世界空间的变换。
     */
    std::array<Location, 8> GetWorldVertices(const Transform &in_bbox_to_world_tr) const { // 获取局部顶点，然后将它们转换到世界空间
        auto world_vertices = GetLocalVertices();
        std::for_each(world_vertices.begin(), world_vertices.end(), [&in_bbox_to_world_tr](auto &world_vertex) {
          in_bbox_to_world_tr.TransformPoint(world_vertex);
        });
        return world_vertices;
    }

    // =========================================================================
    // -- 比较运算符 -------------------------------------------------
    // =========================================================================

    bool operator==(const BoundingBox &rhs) const  { // 判断两个边界框是否相等
      return (location == rhs.location) && (extent == rhs.extent) && (rotation == rhs.rotation);
    }

    bool operator!=(const BoundingBox &rhs) const  { // 判断两个边界框是否不相等
      return !(*this == rhs);
    }

    // =========================================================================
    // -- 转换为UE4类型 ---------------------------------------------
    // =========================================================================

#ifdef LIBCARLA_INCLUDED_FROM_UE4

    BoundingBox(const FBoundingBox &Box) // 从UE4的边界框类型构造一个carla::geom::BoundingBox对象。
      : location(Box.Origin),
        extent(1e-2f * Box.Extent.X, 1e-2f * Box.Extent.Y, 1e-2f * Box.Extent.Z),
        rotation(Box.Rotation) {}

#endif // LIBCARLA_INCLUDED_FROM_UE4

    MSGPACK_DEFINE_ARRAY(location, extent, rotation); // 序列化边界框对象
  };

} // namespace geom
} // namespace carla
