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
    /**
    * @brief 获取路径点的几何变换信息。
    *
    * @return 几何变换信息（geom::Transform类型）。
    */
    const geom::Transform &GetTransform() const {
      return _transform;
    }
    /**
     * @brief 获取交叉路口ID。
     *
     * @return 交叉路口ID（road::JuncId类型）。
     */
    road::JuncId GetJunctionId() const;
    /**
     * @brief 判断当前路径点是否在交叉路口。
     *
     * @return 如果是交叉路口，则返回true；否则返回false。
     */
    bool IsJunction() const;
    /**
    * @brief 获取交叉路口对象。
    *
    * @return 交叉路口对象的共享指针（SharedPtr<Junction>类型）。
    */
    SharedPtr<Junction> GetJunction() const;
    /**
     * @brief 获取车道宽度。
     *
     * @return 车道宽度（double类型）。
     */
    double GetLaneWidth() const;
    /**
     * @brief 获取车道类型。
     *
     * @return 车道类型（road::Lane::LaneType类型）。
     */
    road::Lane::LaneType GetType() const;
    /**
     * @brief 获取指定距离内的下一个路径点列表。
     *
     * @param distance 距离（double类型）。
     * @return 下一个路径点列表（SharedPtr<Waypoint>类型的vector）。
     */
    std::vector<SharedPtr<Waypoint>> GetNext(double distance) const;
    /**
     * @brief 获取指定距离内的上一个路径点列表。
     *
     * @param distance 距离（double类型）。
     * @return 上一个路径点列表（SharedPtr<Waypoint>类型的vector）。
     */
    std::vector<SharedPtr<Waypoint>> GetPrevious(double distance) const;
    /**
      * @brief 获取从当前位置开始，沿车道方向直到道路终点的路径点列表。
      *
      * @param distance 每个路径点之间的间隔距离（double类型）。
      * @return 路径点列表（SharedPtr<Waypoint>类型的vector）。
      */
    std::vector<SharedPtr<Waypoint>> GetNextUntilLaneEnd(double distance) const;
    /**
     * @brief 获取从当前位置开始，沿车道反方向直到道路起点的路径点列表。
     *
     * @param distance 每个路径点之间的间隔距离（double类型）。
     * @return 路径点列表（SharedPtr<Waypoint>类型的vector）。
     */
    std::vector<SharedPtr<Waypoint>> GetPreviousUntilLaneStart(double distance) const;
    /**
     * @brief 获取当前路径点右侧的路径点。
     *
     * @return 右侧路径点的共享指针（SharedPtr<Waypoint>类型）。
     */
    SharedPtr<Waypoint> GetRight() const;
    /**
     * @brief 获取当前路径点左侧的路径点。
     *
     * @return 左侧路径点的共享指针（SharedPtr<Waypoint>类型）。
     */
    SharedPtr<Waypoint> GetLeft() const;
    /**
     * @brief 获取当前路径点右侧的车道标记。
     *
     * @return 右侧车道标记（boost::optional<road::element::LaneMarking>类型）。
     */
    boost::optional<road::element::LaneMarking> GetRightLaneMarking() const;
    /**
     * @brief 获取当前路径点左侧的车道标记。
     *
     * @return 左侧车道标记（boost::optional<road::element::LaneMarking>类型）。
     */
    boost::optional<road::element::LaneMarking> GetLeftLaneMarking() const;
    /**
     * @brief 获取车道变更信息。
     *
     * @return 车道变更信息（road::element::LaneMarking::LaneChange类型）。
     */
    road::element::LaneMarking::LaneChange GetLaneChange() const;
    /**
     * @brief 获取从当前位置到指定距离内的所有地标列表。
     *
     * @param distance 距离（double类型）。
     * @param stop_at_junction 是否在交叉路口停止搜索（bool类型）。
     * @return 地标列表（SharedPtr<Landmark>类型的vector）。
     */
    std::vector<SharedPtr<Landmark>> GetAllLandmarksInDistance(
        double distance, bool stop_at_junction = false) const;
    /**
    * @brief 获取从当前位置到指定距离内，按指定类型筛选的地标列表。
    *
    * @param distance 距离（double类型）。
    * @param filter_type 地标类型过滤器（std::string类型）。
    * @param stop_at_junction 是否在交叉路口停止搜索（bool类型）。
    * @return 地标列表（SharedPtr<Landmark>类型的vector）。
    */
    std::vector<SharedPtr<Landmark>> GetLandmarksOfTypeInDistance(
        double distance, std::string filter_type, bool stop_at_junction = false) const;

  private:
      /**
          * @brief Map类的友元类，允许Map类访问私有构造函数。
          */
    friend class Map;
    /**
    * @brief 私有构造函数，用于内部创建Waypoint对象。
    *
    * @param parent Map对象的共享指针（SharedPtr<const Map>类型）。
    * @param waypoint OpenDrive中的路径点信息（road::element::Waypoint类型）。
    */
    Waypoint(SharedPtr<const Map> parent, road::element::Waypoint waypoint);
    /**
    * @brief 指向父Map对象的共享指针。
    */
    SharedPtr<const Map> _parent;
    /**
     * @brief 存储OpenDrive中的路径点信息。
     */
    road::element::Waypoint _waypoint;
    /**
    * @brief 路径点的几何变换信息。
    */
    geom::Transform _transform;
    /**
         * @brief 分别指向右侧和左侧标记记录的指针对。
         */
    std::pair<
        const road::element::RoadInfoMarkRecord *,
        const road::element::RoadInfoMarkRecord *> _mark_record;
  };

} // namespace client
} // namespace carla
