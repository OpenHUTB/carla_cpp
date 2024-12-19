// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

// 引入调试相关头文件
#include "carla/Debug.h"
// 引入消息打包相关头文件，用于序列化
#include "carla/MsgPack.h"
// 引入变换相关头文件，用于3D变换操作
#include "carla/geom/Transform.h"
// 引入位置相关头文件，定义位置坐标
#include "carla/geom/Location.h"
// 引入三维向量相关头文件，表示3D空间中的向量
#include "carla/geom/Vector3D.h"

#include <array>  // 引入标准数组容器头文件，存储边界框的顶点信息

#ifdef LIBCARLA_INCLUDED_FROM_UE4
// 如果代码是从UE4环境中包含的，启用相关UE4宏

#include <compiler/enable-ue4-macros.h>  // 启用UE4相关宏
#include "Carla/Util/BoundingBox.h"     // 引入UE4中的边界框类
#include <compiler/disable-ue4-macros.h> // 禁用UE4相关宏

#endif // LIBCARLA_INCLUDED_FROM_UE4


namespace carla {
namespace geom {

  /**
   * @brief BoundingBox 类表示一个三维空间中的矩形边界框。 
   * 它包含了边界框的中心位置、半尺寸（extent）和旋转（rotation）。
   * 该类主要用于空间对象的包围盒表示，并能进行点是否在边界框内的检测，以及转换顶点等操作。
   */
  class BoundingBox { // 边界框类，表示一个3D空间中的矩形区域。
  public:
    // 默认构造函数
    BoundingBox() = default;

    // =========================================================================
    // -- 构造函数 ---------------------------------------------------------
    // =========================================================================

    /**
     * @brief 构造一个边界框对象，指定位置、大小和旋转。
     * @param in_location 边界框的中心位置
     * @param in_extent 边界框的半尺寸（每个轴方向上的半宽、半高、半深）
     * @param in_rotation 边界框的旋转
     */
    explicit BoundingBox(const Location &in_location, const Vector3D &in_extent, const Rotation &in_rotation)
      : location(in_location), // 构造一个边界框，指定其中心位置、半大小和旋转。
        extent(in_extent),
        rotation(in_rotation) {}

     /**
     * @brief 构造一个边界框对象，指定位置和大小，旋转默认为零。
     * @param in_location 边界框的中心位置
     * @param in_extent 边界框的半尺寸
     */
    explicit BoundingBox(const Location &in_location, const Vector3D &in_extent) // 仅指定位置和大小的构造函数。
      : location(in_location), 
        extent(in_extent),
        rotation() {}

    /**
     * @brief 构造一个边界框对象，指定大小，位置和旋转默认为零。
     * @param in_extent 边界框的半尺寸
     */
    explicit BoundingBox(const Vector3D &in_extent) // 仅指定大小的构造函数，位置和旋转默认为默认值。
      : location(),
        extent(in_extent),
        rotation() {}

    // =========================================================================
    // -- 成员变量 --------------------------------------------------------
    // =========================================================================
    // 成员变量定义了边界框的中心位置、半大小和旋转。
    Location location;  ///< 边界框的中心位置（本地坐标系下）
    Vector3D extent;    ///< 边界框的半尺寸（本地坐标系下，表示在每个轴方向上的半宽、半高和半深）
    Rotation rotation;  ///< 边界框的旋转（本地坐标系下）

    // =========================================================================
    // -- 其他方法 --------------------------------------------------------
    // =========================================================================

     /**
     * @brief 检查给定的世界空间中的点是否在边界框内。
     * 通过将世界空间中的点转换为边界框的局部坐标系，再判断该点是否位于边界框的范围内。
     * 
     * @param in_world_point 要检查的世界空间中的点
     * @param in_bbox_to_world_transform 从边界框空间到世界空间的变换矩阵
     * @return 如果点在边界框内，返回true，否则返回false
     */
    bool Contains(const Location &in_world_point, const Transform &in_bbox_to_world_transform) const {
        auto point_in_bbox_space = in_world_point;
        in_bbox_to_world_transform.InverseTransformPoint(point_in_bbox_space); // 将世界空间中的点转换到边界框空间
        point_in_bbox_space -= location; // 以边界框中心为原点，计算相对位置

        // 判断点是否在边界框的范围内（根据边界框的半大小和坐标轴方向）
        return  point_in_bbox_space.x >= -extent.x && point_in_bbox_space.x <= extent.x &&
                point_in_bbox_space.y >= -extent.y && point_in_bbox_space.y <= extent.y &&
                point_in_bbox_space.z >= -extent.z && point_in_bbox_space.z <= extent.z;
    }

     /**
     * @brief 获取边界框在本地空间中的8个顶点的位置（不考虑旋转）。
     * 
     * @return 边界框8个顶点的位置数组
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
     * @brief 获取边界框在本地空间中的8个顶点的位置（不考虑旋转）。
     * 该函数返回的顶点位置不应用旋转，只考虑边界框的位置和半尺寸。
     * 
     * @return 边界框8个顶点的位置数组（不考虑旋转）
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
     * @brief 获取边界框在世界空间中的8个顶点的位置。
     * 该函数先获取边界框的局部顶点，再将这些顶点通过变换矩阵转换到世界空间。
     * 
     * @param in_bbox_to_world_tr 从边界框空间到世界空间的变换矩阵
     * @return 边界框8个顶点的位置数组（转换到世界空间）
     */
    std::array<Location, 8> GetWorldVertices(const Transform &in_bbox_to_world_tr) const { // 获取局部顶点，然后将它们转换到世界空间
        auto world_vertices = GetLocalVertices();
        // 将每个局部顶点转换到世界空间
        std::for_each(world_vertices.begin(), world_vertices.end(), [&in_bbox_to_world_tr](auto &world_vertex) {
          in_bbox_to_world_tr.TransformPoint(world_vertex);
        });
        return world_vertices;
    }

    // =========================================================================
    // -- 比较运算符 -------------------------------------------------
    // =========================================================================

    /**
     * @brief 比较两个边界框是否相等。边界框相等的条件是其位置、半尺寸和旋转都相同。
     * 
     * @param rhs 另一个要比较的边界框
     * @return 如果两个边界框的中心位置、半尺寸和旋转相同，返回true；否则返回false
     */
    bool operator==(const BoundingBox &rhs) const  { // 判断两个边界框是否相等
      return (location == rhs.location) && (extent == rhs.extent) && (rotation == rhs.rotation);
    }

    /**
     * 比较两个边界框是否不相等。
     * @param rhs 另一个要比较的边界框。
     * @return 如果两个边界框的任何一个属性不同，返回true；否则返回false。
     */
    bool operator!=(const BoundingBox &rhs) const  { // 判断两个边界框是否不相等
      return !(*this == rhs);
    }

    // =========================================================================
    // -- 转换为UE4类型 ---------------------------------------------
    // =========================================================================

#ifdef LIBCARLA_INCLUDED_FROM_UE4

    /**
     * 从UE4的边界框类型（FBoundingBox）构造一个carla::geom::BoundingBox对象。
     * @param Box UE4中的边界框对象。
     */
    BoundingBox(const FBoundingBox &Box) // 从UE4的边界框类型构造一个carla::geom::BoundingBox对象。
      : location(Box.Origin),
        extent(1e-2f * Box.Extent.X, 1e-2f * Box.Extent.Y, 1e-2f * Box.Extent.Z),
        rotation(Box.Rotation) {}

#endif // LIBCARLA_INCLUDED_FROM_UE4
    // 序列化边界框对象，使用MsgPack格式进行存储。
    MSGPACK_DEFINE_ARRAY(location, extent, rotation); 
  };

} // namespace geom
} // namespace carla
