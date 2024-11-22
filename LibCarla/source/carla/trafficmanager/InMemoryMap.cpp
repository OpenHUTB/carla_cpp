// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/Logging.h"

#include "carla/trafficmanager/Constants.h"
#include "carla/trafficmanager/InMemoryMap.h"
#include <boost/geometry/geometries/box.hpp>

namespace carla {
namespace traffic_manager {

  namespace cg = carla::geom;
  using namespace constants::Map;

  using TopologyList = std::vector<std::pair<WaypointPtr, WaypointPtr>>;
  using RawNodeList = std::vector<WaypointPtr>;

  InMemoryMap::InMemoryMap(WorldMap world_map) : _world_map(world_map) {}
  InMemoryMap::~InMemoryMap() {}

  SegmentId InMemoryMap::GetSegmentId(const WaypointPtr &wp) const {
    return std::make_tuple(wp->GetRoadId(), wp->GetLaneId(), wp->GetSectionId());
  }

  SegmentId InMemoryMap::GetSegmentId(const SimpleWaypointPtr &swp) const {
    return GetSegmentId(swp->GetWaypoint());
  }

  NodeList InMemoryMap::GetSuccessors(const SegmentId segment_id,
                                      const SegmentTopology &segment_topology,
                                      const SegmentMap &segment_map) {
    NodeList result;
    if (segment_topology.find(segment_id) == segment_topology.end()) {
      return result;
    }

    for (const auto &successor_segment_id : segment_topology.at(segment_id).second) {
      if (segment_map.find(successor_segment_id) == segment_map.end()) {
        auto successors = GetSuccessors(successor_segment_id, segment_topology, segment_map);
        result.insert(result.end(), successors.begin(), successors.end());
      } else {
        result.emplace_back(segment_map.at(successor_segment_id).front());
      }
    }
    return result;
  }

  NodeList InMemoryMap::GetPredecessors(const SegmentId segment_id,
                                        const SegmentTopology &segment_topology,
                                        const SegmentMap &segment_map) {
    NodeList result;
    if (segment_topology.find(segment_id) == segment_topology.end()) {
      return result;
    }

    for (const auto &predecessor_segment_id : segment_topology.at(segment_id).first) {
      if (segment_map.find(predecessor_segment_id) == segment_map.end()) {
        auto predecessors = GetPredecessors(predecessor_segment_id, segment_topology, segment_map);
        result.insert(result.end(), predecessors.begin(), predecessors.end());
      } else {
        result.emplace_back(segment_map.at(predecessor_segment_id).back());
      }
    }
    return result;
  }

  void InMemoryMap::Cook(WorldMap world_map, const std::string& path) {
    InMemoryMap local_map(world_map);
    local_map.SetUp();
    local_map.Save(path);
  }

  void InMemoryMap::Save(const std::string& path) {
    std::string filename;
    if (path.empty()) {
      filename = this->GetMapName() + ".bin";
    } else {
      filename = path;
    }

    std::ofstream out_file;
    out_file.open(filename, std::ios::binary);
    if (!out_file.is_open()) {
      log_error("Could not open binary file");
      return;
    }

    // 将所有记录的总数写入文件中
    uint32_t total = static_cast<uint32_t>(dense_topology.size());
    out_file.write(reinterpret_cast<const char *>(&total), sizeof(uint32_t));

    // 创建或记录一些基本的导航点
    std::unordered_set<uint64_t> used_ids;
    for (auto& wp: dense_topology) {
      if (used_ids.find(wp->GetId()) != used_ids.end()) {
        log_error("Could not generate the binary file. There are repeated waypoints");
      }
      CachedSimpleWaypoint cached_wp(wp);
      cached_wp.Write(out_file);

      used_ids.insert(wp->GetId());
    }

    out_file.close();
    return;
  }

  bool InMemoryMap::Load(const std::vector<uint8_t>& content) {
    unsigned long pos = 0;
    std::vector<CachedSimpleWaypoint> cached_waypoints;
    std::unordered_map<uint64_t, uint32_t> id2index;

    // 读取总记录数
    uint32_t total;
    memcpy(&total, &content[pos], sizeof(total));
    pos += sizeof(total);

    // 读取简单航点
    for (uint32_t i=0; i < total; i++) {
      CachedSimpleWaypoint cached_wp;
      cached_wp.Read(content, pos);
      cached_waypoints.push_back(cached_wp);
      id2index.insert({cached_wp.waypoint_id, i});

      WaypointPtr waypoint_ptr = _world_map->GetWaypointXODR(cached_wp.road_id, cached_wp.lane_id, cached_wp.s);
      SimpleWaypointPtr wp = std::make_shared<SimpleWaypoint>(waypoint_ptr);
      wp->SetGeodesicGridId(cached_wp.geodesic_grid_id);
      wp->SetIsJunction(cached_wp.is_junction);
      wp->SetRoadOption(static_cast<RoadOption>(cached_wp.road_option));
      dense_topology.push_back(wp);
    }

    // 连接航点
    for (uint32_t i=0; i < dense_topology.size(); i++) {
      auto wp = dense_topology.at(i);
      auto cached_wp = cached_waypoints.at(i);

      std::vector<SimpleWaypointPtr> next_waypoints;
      for (auto id : cached_wp.next_waypoints) {
        next_waypoints.push_back(dense_topology.at(id2index.at(id)));
      }
      std::vector<SimpleWaypointPtr> previous_waypoints;
      for (auto id : cached_wp.previous_waypoints) {
        previous_waypoints.push_back(dense_topology.at(id2index.at(id)));
      }
      wp->SetNextWaypoint(next_waypoints);
      wp->SetPreviousWaypoint(previous_waypoints);
      if (cached_wp.next_left_waypoint > 0) {
        wp->SetLeftWaypoint(dense_topology.at(id2index.at(cached_wp.next_left_waypoint)));
      }
      if (cached_wp.next_right_waypoint > 0) {
        wp->SetRightWaypoint(dense_topology.at(id2index.at(cached_wp.next_right_waypoint)));
      }
    }

    // 创建空间树
    SetUpSpatialTree();

    return true;
  }

  void InMemoryMap::SetUp() {

    // 1. 构建路段拓扑（即，定义路段的前驱和后继集合）
    assert(_world_map != nullptr && "No map reference found.");
    auto waypoint_topology = _world_map->GetTopology();

    SegmentTopology segment_topology;
    std::unordered_map<int64_t, std::pair<std::set<crd::RoadId>, std::set<crd::RoadId>>> std_road_connectivity;
    std::unordered_map<crd::RoadId, bool> is_real_junction;

    for (auto &connection : waypoint_topology) {
      auto &waypoint = connection.first;
      auto &successor = connection.second;

      // 设置段落的前驱和后继
      SegmentId waypoint_segment_id = GetSegmentId(connection.first);
      SegmentId successor_segment_id = GetSegmentId(connection.second);
      if (waypoint_segment_id == successor_segment_id){
        // 如果两个拓扑航点位于同一段，则忽略它们
        // 这种情况发生在没有后继或前驱连接的车道上
        continue;
      }
      using SegIdVectorPair = std::pair<std::vector<SegmentId>, std::vector<SegmentId>>;
      SegIdVectorPair &connection_first = segment_topology[waypoint_segment_id];
      SegIdVectorPair &connection_second = segment_topology[successor_segment_id];
      connection_first.second.push_back(successor_segment_id);
      connection_second.first.push_back(waypoint_segment_id);

      // 从路径到标准道路
      bool waypoint_is_junction = waypoint->IsJunction();
      bool successor_is_junction = successor->IsJunction();
      if (waypoint_is_junction && !successor_is_junction) {
        crd::RoadId path_id = waypoint->GetRoadId();
        int64_t std_road_id = static_cast<int64_t>(successor->GetRoadId());
        std_road_id = (successor->GetLaneId() < 0) ? -1 * std_road_id : std_road_id;

        std::set<crd::RoadId> &in_paths = std_road_connectivity[std_road_id].first;
        in_paths.insert(path_id);

        if (in_paths.size() >= 2) {
          for (auto &in_path_id: in_paths) {
            is_real_junction[in_path_id] = true;
          }
        }
      }

      // 从标准道路到小径
      if (!waypoint_is_junction && successor_is_junction) {
        crd::RoadId path_id = successor->GetRoadId();
        int64_t std_road_id = static_cast<int64_t>(waypoint->GetRoadId());
        std_road_id = (waypoint->GetLaneId() < 0) ? -1 * std_road_id : std_road_id;

        std::set<crd::RoadId> &out_paths = std_road_connectivity[std_road_id].second;
        out_paths.insert(path_id);

        if (out_paths.size() >= 2) {
          for (auto &out_path_id: out_paths) {
            is_real_junction[out_path_id] = true;
          }
        }
      }
    }

    // 2. 将cc::Map中的原始密集拓扑消费到SimpleWaypoints中
    SegmentMap segment_map;
    assert(_world_map != nullptr && "No map reference found.");
    auto raw_dense_topology = _world_map->GenerateWaypoints(MAP_RESOLUTION);
    for (auto &waypoint_ptr: raw_dense_topology) {
      if (waypoint_ptr->GetLaneWidth() > MIN_LANE_WIDTH){
        // 避免让车辆通过非常狭窄的车道
        segment_map[GetSegmentId(waypoint_ptr)].emplace_back(std::make_shared<SimpleWaypoint>(waypoint_ptr));
      }
    }

    // 3. 处理航点
    auto distance_squared = [](cg::Location l1, cg::Location l2) {
      return cg::Math::DistanceSquared(l1, l2);
    };
    auto square = [](float input) {return std::pow(input, 2);};
    auto compare_s = [](const SimpleWaypointPtr &swp1, const SimpleWaypointPtr &swp2) {
      return (swp1->GetWaypoint()->GetDistance() < swp2->GetWaypoint()->GetDistance());
    };
    auto wpt_angle = [](cg::Vector3D l1, cg::Vector3D l2) {
      return cg::Math::GetVectorAngle(l1, l2);
    };
    auto max = [](int16_t x, int16_t y) {
      return x ^ ((x ^ y) & -(x < y));
    };

    GeoGridId geodesic_grid_id_counter = -1;
    for (auto &segment: segment_map) {
      auto &segment_waypoints = segment.second;

      // 生成测地线网格ID
      ++geodesic_grid_id_counter;

      // 根据道路方向排序航点
      std::sort(segment_waypoints.begin(), segment_waypoints.end(), compare_s);
      auto lane_id = segment_waypoints.front()->GetWaypoint()->GetLaneId();
      if (lane_id > 0) {
        std::reverse(segment_waypoints.begin(), segment_waypoints.end());
      }

      // 如果角度太紧或航点之间的距离太远，则添加更多航点
      for (std::size_t i = 0; i < segment_waypoints.size() - 1; ++i) {
          double distance = std::abs(segment_waypoints.at(i)->GetWaypoint()->GetDistance() - segment_waypoints.at(i+1)->GetWaypoint()->GetDistance());
          double angle = wpt_angle(segment_waypoints.at(i)->GetTransform().GetForwardVector(), segment_waypoints.at(i+1)->GetTransform().GetForwardVector());
          int16_t angle_splits = static_cast<int16_t>(angle/MAX_WPT_RADIANS);
          int16_t distance_splits = static_cast<int16_t>((distance*distance)/MAX_WPT_DISTANCE);
          auto max_splits = max(angle_splits, distance_splits);
          if (max_splits >= 1) {
            // 计算我们需要生成多少个航点
            for (uint16_t j = 0; j < max_splits; ++j) {
              auto next_waypoints = segment_waypoints.at(i)->GetWaypoint()->GetNext(distance/(max_splits+1));
              if (next_waypoints.size() != 0) {
                auto new_waypoint = next_waypoints.front();
                i++;
                segment_waypoints.insert(segment_waypoints.begin()+static_cast<int64_t>(i), std::make_shared<SimpleWaypoint>(new_waypoint));
              } else {
                // 到达路的尽头
                break;
              }
            }
          }
        }

      // 放置段内连接
      cg::Location grid_edge_location = segment_waypoints.front()->GetLocation();
      for (std::size_t i = 0; i < segment_waypoints.size() - 1; ++i) {
        SimpleWaypointPtr current_waypoint = segment_waypoints.at(i);
        SimpleWaypointPtr next_waypoint = segment_waypoints.at(i+1);
        // 分配网格ID
        if (distance_squared(grid_edge_location, current_waypoint->GetLocation()) >
        square(MAX_GEODESIC_GRID_LENGTH)) {
          ++geodesic_grid_id_counter;
          grid_edge_location = current_waypoint->GetLocation();
        }
        current_waypoint->SetGeodesicGridId(geodesic_grid_id_counter);

        current_waypoint->SetNextWaypoint({next_waypoint});
        next_waypoint->SetPreviousWaypoint({current_waypoint});

      }
      segment_waypoints.back()->SetGeodesicGridId(geodesic_grid_id_counter);

      // 在处理后的密集拓扑中添加简单的航点
      for (auto swp: segment_waypoints) {
        // 检查航点是否位于实际的交叉路口
        auto wpt = swp->GetWaypoint();
        auto road_id = wpt->GetRoadId();
        if (wpt->IsJunction() && !is_real_junction.count(road_id)) {
          swp->SetIsJunction(false);
        } else {
          swp->SetIsJunction(swp->GetWaypoint()->IsJunction());
        }

        dense_topology.push_back(swp);
      }
    }

    SetUpSpatialTree();

    // 放置段间连接
    for (auto &segment : segment_map) {
      SegmentId segment_id = segment.first;
      auto &segment_waypoints = segment.second;

      auto successors = GetSuccessors(segment_id, segment_topology, segment_map);
      auto predecessors = GetPredecessors(segment_id, segment_topology, segment_map);

      segment_waypoints.front()->SetPreviousWaypoint(predecessors);
      segment_waypoints.back()->SetNextWaypoint(successors);
    }

    // 连接车道变更连接
    for (auto &swp : dense_topology) {
      if (!swp->CheckJunction()) {
        FindAndLinkLaneChange(swp);
      }
    }

    // 连接任何未连接的线段
    for (auto &swp : dense_topology) {
      if (swp->GetNextWaypoint().empty()) {
        auto neighbour = swp->GetRightWaypoint();
        if (!neighbour) {
          neighbour = swp->GetLeftWaypoint();
        }

        if (neighbour) {
          swp->SetNextWaypoint(neighbour->GetNextWaypoint());
          for (auto next_waypoint : neighbour->GetNextWaypoint()) {
            next_waypoint->SetPreviousWaypoint({swp}); 
          }
        }
      }
    }

    // 为每个 SimpleWaypoint 指定一个 RoadOption
    SetUpRoadOption();
  }

  void InMemoryMap::SetUpSpatialTree() {
    for (auto &simple_waypoint: dense_topology) {
      if (simple_waypoint != nullptr) {
        const cg::Location loc = simple_waypoint->GetLocation();
        Point3D point(loc.x, loc.y, loc.z);
        rtree.insert(std::make_pair(point, simple_waypoint));
      }
    }
  }

  void InMemoryMap::SetUpRoadOption() {
    for (auto &swp : dense_topology) {
      std::vector<SimpleWaypointPtr> next_waypoints = swp->GetNextWaypoint();
      std::size_t next_swp_size = next_waypoints.size();

      if (next_swp_size == 0) {
        // 没有下一个航点意味着这是路的尽头
        swp->SetRoadOption(RoadOption::RoadEnd);
      }

      else if (next_swp_size > 1 || (!swp->CheckJunction() && next_waypoints.front()->CheckJunction())) {
        // 为了确认我们是否真的在一个交叉路口，而不是在高速公路上，我们尝试查看
        // 如果附近有交通灯、停车标志或让行标志等类型的地标
        bool found_landmark= false;
        if (next_swp_size <= 1) {

          auto all_landmarks = swp->GetWaypoint()->GetAllLandmarksInDistance(15.0);

          if (all_landmarks.empty()) {
            // 未发现地标，这不是一个交叉口
            swp->SetRoadOption(RoadOption::LaneFollow);
          } else {
            for (auto &landmark : all_landmarks) {
              auto landmark_type = landmark->GetType();
              if (landmark_type == "1000001" || landmark_type == "206" || landmark_type == "205") {
                // 我们发现了一个地标
                found_landmark= true;
                break;
              }
            }
            if (!found_landmark) {
              swp->SetRoadOption(RoadOption::LaneFollow);
            }
          }
        }

        // 如果我们确实找到了一个地标，或者在另一种情况下，找到了所有的航点
        // 在交汇处并分配正确的道路选项
        if (found_landmark || next_swp_size > 1) {
          swp->SetRoadOption(RoadOption::LaneFollow);
          for (auto &next_swp : next_waypoints) {
            std::vector<SimpleWaypointPtr> traversed_waypoints;
            SimpleWaypointPtr junction_end_waypoint;

            if (next_swp_size > 1) {
              junction_end_waypoint = next_swp;
            } else {
              junction_end_waypoint = next_waypoints.front();
            }

            while (junction_end_waypoint->CheckJunction()){
              traversed_waypoints.push_back(junction_end_waypoint);
              std::vector<SimpleWaypointPtr> temp = junction_end_waypoint->GetNextWaypoint();
              if (temp.empty()) {
                break;
              }
              junction_end_waypoint = temp.front();
            }

            // 计算连接点首尾两点之间的夹角
            int16_t current_angle = static_cast<int16_t>(traversed_waypoints.front()->GetTransform().rotation.yaw);
            int16_t junction_end_angle = static_cast<int16_t>(traversed_waypoints.back()->GetTransform().rotation.yaw);
            int16_t diff_angle = (junction_end_angle - current_angle) % 360;
            bool straight = (diff_angle < STRAIGHT_DEG && diff_angle > -STRAIGHT_DEG) ||
                  (diff_angle > 360-STRAIGHT_DEG && diff_angle <= 360) ||
                  (diff_angle < -360+STRAIGHT_DEG && diff_angle >= -360);
            bool right = (diff_angle >= STRAIGHT_DEG && diff_angle <= 180) ||
                (diff_angle <= -180 && diff_angle >= -360+STRAIGHT_DEG);

            auto assign_option = [](RoadOption ro, std::vector<SimpleWaypointPtr> traversed_waypoints) {
              for (auto &twp : traversed_waypoints) {
                  twp->SetRoadOption(ro);
              }
            };

            // 根据角度分配道路选项
            if (straight) assign_option(RoadOption::Straight, traversed_waypoints);
            else if (right) assign_option(RoadOption::Right, traversed_waypoints);
            else assign_option(RoadOption::Left, traversed_waypoints);
          }
        }
      }
      else if (next_swp_size == 1 && swp->GetRoadOption() == RoadOption::Void) {
        swp->SetRoadOption(RoadOption::LaneFollow);
      }
    }
  }

  SimpleWaypointPtr InMemoryMap::GetWaypoint(const cg::Location loc) const {

    Point3D query_point(loc.x, loc.y, loc.z);
    std::vector<SpatialTreeEntry> result_1;

    rtree.query(bgi::nearest(query_point, 1), std::back_inserter(result_1));

    SpatialTreeEntry &closest_entry = result_1.front();
    SimpleWaypointPtr &closest_point = closest_entry.second;

    return closest_point;
  }

  NodeList InMemoryMap::GetWaypointsInDelta(const cg::Location loc, const uint16_t n_points, const float random_sample) const {
    Point3D query_point(loc.x, loc.y, loc.z);

    Point3D lower_p1(loc.x + random_sample, loc.y + random_sample, loc.z + Z_DELTA);
    Point3D lower_p2(loc.x - random_sample, loc.y - random_sample, loc.z - Z_DELTA);
    Point3D upper_p1(loc.x + random_sample + DELTA, loc.y + random_sample + DELTA, loc.z + Z_DELTA);
    Point3D upper_p2(loc.x - random_sample - DELTA, loc.y - random_sample - DELTA, loc.z - Z_DELTA);

    Box lower_query_box(lower_p2, lower_p1);
    Box upper_query_box(upper_p2, upper_p1);

    NodeList result;
    uint8_t x = 0;
    for (Rtree::const_query_iterator
        it = rtree.qbegin(bgi::within(upper_query_box)
        && !bgi::within(lower_query_box)
        && bgi::satisfies([&](SpatialTreeEntry const& v) { return !v.second->CheckJunction();}));
        it != rtree.qend();
        ++it) {
    x++;
    result.push_back(it->second);
    if (x >= n_points)
        break;
    }

    return result;
  }

  NodeList InMemoryMap::GetDenseTopology() const {
    return dense_topology;
  }

  void InMemoryMap::FindAndLinkLaneChange(SimpleWaypointPtr reference_waypoint) {

    const WaypointPtr raw_waypoint = reference_waypoint->GetWaypoint();
    const crd::element::LaneMarking::LaneChange lane_change = raw_waypoint->GetLaneChange();

    /// Cheack for transits
    switch(lane_change)
    {
      /// Left transit way point present only
      case crd::element::LaneMarking::LaneChange::Left:
      {
        const WaypointPtr left_waypoint = raw_waypoint->GetLeft();
        if (left_waypoint != nullptr &&
        left_waypoint->GetType() == crd::Lane::LaneType::Driving &&
        (left_waypoint->GetLaneId() * raw_waypoint->GetLaneId() > 0)) {

          SimpleWaypointPtr closest_simple_waypoint = GetWaypoint(left_waypoint->GetTransform().location);
          reference_waypoint->SetLeftWaypoint(closest_simple_waypoint);
        }
      }
      break;

      /// Right transit way point present only
      case crd::element::LaneMarking::LaneChange::Right:
      {
	    const WaypointPtr right_waypoint = raw_waypoint->GetRight();
	    if(right_waypoint != nullptr &&
	    right_waypoint->GetType() == crd::Lane::LaneType::Driving &&
	    (right_waypoint->GetLaneId() * raw_waypoint->GetLaneId() > 0)) {

	      SimpleWaypointPtr closest_simple_waypoint = GetWaypoint(right_waypoint->GetTransform().location);
	      reference_waypoint->SetRightWaypoint(closest_simple_waypoint);
	    }
      }
      break;

      /// Both left and right transit present
      case crd::element::LaneMarking::LaneChange::Both:
      {
        /// Right transit way point
        const WaypointPtr right_waypoint = raw_waypoint->GetRight();
        if (right_waypoint != nullptr &&
        right_waypoint->GetType() == crd::Lane::LaneType::Driving &&
        (right_waypoint->GetLaneId() * raw_waypoint->GetLaneId() > 0)) {

          SimpleWaypointPtr closest_simple_waypointR = GetWaypoint(right_waypoint->GetTransform().location);
          reference_waypoint->SetRightWaypoint(closest_simple_waypointR);
        }

        /// Left transit way point
        const WaypointPtr left_waypoint = raw_waypoint->GetLeft();
        if (left_waypoint != nullptr &&
        left_waypoint->GetType() == crd::Lane::LaneType::Driving &&
        (left_waypoint->GetLaneId() * raw_waypoint->GetLaneId() > 0)) {

          SimpleWaypointPtr closest_simple_waypointL = GetWaypoint(left_waypoint->GetTransform().location);
          reference_waypoint->SetLeftWaypoint(closest_simple_waypointL);
        }
      }
      break;

      /// For no transit waypoint (left or right)
      default: break;
    }
  }

  std::string InMemoryMap::GetMapName() {
    assert(_world_map != nullptr && "No map reference found.");
    return _world_map->GetName();
  }

  const cc::Map& InMemoryMap::GetMap() const {
    return *_world_map;
  }


} // namespace traffic_manager
} // namespace carla
