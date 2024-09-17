// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Memory.h"
#include "carla/NonCopyable.h"
#include "carla/geom/Transform.h"
#include "carla/road/element/LaneMarking.h"
#include "carla/road/element/RoadInfoMarkRecord.h"
#include "carla/road/element/Waypoint.h"
#include "carla/road/Lane.h"
#include "carla/road/RoadTypes.h"

#include <boost/optional.hpp>

namespace carla {
namespace client {

  class Map;
  class Junction;
  class Landmark;

  class Waypoint
    : public EnableSharedFromThis<Waypoint>,
    private NonCopyable {
  public:

    ~Waypoint();

    /// 返回标识此路径点的唯一Id.
    ///
    /// 这个Id综合了OpenDrive的道路Id、车道Id和s距离
    /// 其路段精度可达半厘米.
    uint64_t GetId() const {
      return std::hash<road::element::Waypoint>()(_waypoint);
    }

    auto GetRoadId() const {
      return _waypoint.road_id;
    }

    auto GetSectionId() const {
      return _waypoint.section_id;
    }

    auto GetLaneId() const {
      return _waypoint.lane_id;
    }

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
