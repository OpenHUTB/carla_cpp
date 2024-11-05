// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
/**
 * @brief CARLA内存管理相关功能的命名空间。
 *
 * 该命名空间包含与内存管理相关的类和函数，例如智能指针等。
 */
#include "carla/Memory.h"
 /**
  * @brief 定义了不可复制类的基类。
  *
  * 该头文件定义了一个基类，用于防止派生类被复制或赋值。
  */
#include "carla/NonCopyable.h"
  /**
   * @brief 定义了用于几何变换的类。
   *
   * 该头文件包含了处理3D几何变换的类，例如平移、旋转和缩放等。
   */
#include "carla/geom/Transform.h"
   /**
    * @brief 定义了车道标记的类。
    *
    * 该头文件包含了表示车道标记（如虚线、实线等）的类。
    */
#include "carla/road/element/LaneMarking.h"
    /**
     * @brief 定义了道路信息记录的类。
     *
     * 该头文件包含了表示道路信息记录（如交通标志、信号灯等）的类。
     */
#include "carla/road/element/RoadInfoMarkRecord.h"
     /**
      * @brief 定义了路径点的类。
      *
      * 该头文件包含了表示路径点（道路上的特定位置）的类。
      */
#include "carla/road/element/Waypoint.h"
      /**
       * @brief 定义了车道的类。
       *
       * 该头文件包含了表示车道（道路上的行驶区域）的类。
       */
#include "carla/road/Lane.h"
       /**
        * @brief 定义了道路类型的枚举和辅助函数。
        *
        * 该头文件包含了表示不同道路类型（如高速公路、普通道路等）的枚举以及相关的辅助函数。
        */
#include "carla/road/RoadTypes.h"
        /**
         * @brief Boost库中的可选值类型。
         *
         * 该头文件包含了Boost库中的boost::optional模板类，用于表示一个可能不存在的值。
         */
#include <boost/optional.hpp>
         /**
          * @namespace carla::client
          * @brief CARLA客户端相关的命名空间。
          */
namespace carla {
namespace client {
    // 前向声明其他相关类  
  class Map;
  class Junction;
  class Landmark;
  /**
   * @class Waypoint
   * @brief 路径点类，表示道路上的特定位置。
   *
   * Waypoint类继承自EnableSharedFromThis，使得它可以安全地生成自身的共享指针。
   * 同时，它继承自NonCopyable，防止被复制。
   */
  class Waypoint
    : public EnableSharedFromThis<Waypoint>,
    private NonCopyable {
  public:
      /**
           * @brief 析构函数。
           */
    ~Waypoint();

    /**
      * @brief 获取此路径点的唯一标识符。
      *
      * 该标识符结合了OpenDrive的道路ID、车道ID和s距离，精度可达半厘米。
      *
      * @return 唯一标识符（uint64_t类型）。
      */
    uint64_t GetId() const {
      return std::hash<road::element::Waypoint>()(_waypoint);
    }
    /**
     * @brief 获取道路ID。
     *
     * @return 道路ID。
     */
    auto GetRoadId() const {
      return _waypoint.road_id;
    }
    /**
     * @brief 获取路段ID。
     *
     * @return 路段ID。
     */
    auto GetSectionId() const {
      return _waypoint.section_id;
    }
    /**
    * @brief 获取车道ID。
    *
    * @return 车道ID。
    */
    auto GetLaneId() const {
      return _waypoint.lane_id;
    }
    /**
     * @brief 获取沿车道的距离（s值）。
     *
     * @return 距离（double类型）。
     */
    auto GetDistance() const {
      return _waypoint.s;
    }

    const geom::Transform &GetTransform() const {
      return _transform;
    }

    road::JuncId GetJunctionId() const;

    bool IsJunction() const;

    SharedPtr<Junction> GetJunction() const;

    double GetLaneWidth() const;

    road::Lane::LaneType GetType() const;

    std::vector<SharedPtr<Waypoint>> GetNext(double distance) const;

    std::vector<SharedPtr<Waypoint>> GetPrevious(double distance) const;

    /// 返回与当前路点按距离分隔的路点列表
    /// 持续到道路终点
    std::vector<SharedPtr<Waypoint>> GetNextUntilLaneEnd(double distance) const;

    /// 返回与当前路点按距离分隔的路点列表
    /// 持续到道路起点
    std::vector<SharedPtr<Waypoint>> GetPreviousUntilLaneStart(double distance) const;

    SharedPtr<Waypoint> GetRight() const;

    SharedPtr<Waypoint> GetLeft() const;

    boost::optional<road::element::LaneMarking> GetRightLaneMarking() const;

    boost::optional<road::element::LaneMarking> GetLeftLaneMarking() const;

    road::element::LaneMarking::LaneChange GetLaneChange() const;

    /// 返回从当前位置到指定距离的地标列表
    std::vector<SharedPtr<Landmark>> GetAllLandmarksInDistance(
        double distance, bool stop_at_junction = false) const;

    /// 返回从当前位置到指定距离的地标列表
    /// 地标点按指定类型筛选
    std::vector<SharedPtr<Landmark>> GetLandmarksOfTypeInDistance(
        double distance, std::string filter_type, bool stop_at_junction = false) const;

  private:

    friend class Map;

    Waypoint(SharedPtr<const Map> parent, road::element::Waypoint waypoint);

    SharedPtr<const Map> _parent;

    road::element::Waypoint _waypoint;

    geom::Transform _transform;

    // 分别在右侧和左侧标记记录.
    std::pair<
        const road::element::RoadInfoMarkRecord *,
        const road::element::RoadInfoMarkRecord *> _mark_record;
  };

} // namespace client
} // namespace carla
