// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// 参考：http://hdmap.geomatics.ncku.edu.tw/assets/docs/20200612_TAICS%20TS-0024%20v1.01-%E9%AB%98%E7%B2%BE%E5%9C%B0%E5%9C%96%E5%9C%96%E8%B3%87%E5%85%A7%E5%AE%B9%E5%8F%8A%E6%A0%BC%E5%BC%8F%E6%A8%99%E6%BA%96.pdf
// 
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/opendrive/parser/RoadParser.h"

#include "carla/Logging.h"
#include "carla/StringUtil.h"
#include "carla/road/MapBuilder.h"
#include "carla/road/RoadTypes.h"

#include <pugixml/pugixml.hpp>

namespace carla {
namespace opendrive {
namespace parser {

  using RoadId = road::RoadId;
  using LaneId = road::LaneId;
  using JuncId = road::JuncId;

  // 三次多项式的线性 v_{local} = a + b * du + c * du^2 + d * du^3
  struct Polynomial {
    double s;
    double a, b, c, d;
  };

  struct Lane {
    LaneId id;
    road::Lane::LaneType type;
    bool level;
    LaneId predecessor;
    LaneId successor;
  };

  struct LaneOffset {
    double s;
    double a, b, c, d;
  };

  struct LaneSection {
    double s;
    std::vector<Lane> lanes;
  };

  struct RoadTypeSpeed {
    double s;
    std::string type;
    double max;
    std::string unit;
  };

  struct Road {
    RoadId id;
    std::string name;
    double length;
    JuncId junction_id;
    RoadId predecessor;
    RoadId successor;
    std::vector<RoadTypeSpeed> speed;
    std::vector<LaneOffset> section_offsets;
    std::vector<LaneSection> sections;
  };

  // 字符串转车道类型
  static road::Lane::LaneType StringToLaneType(std::string &&str) {
    StringUtil::ToLower(str);
    if (str == "driving") {                     // 车道
      return road::Lane::LaneType::Driving;
    } else if (str == "stop") {                 // 禁止进入
      return road::Lane::LaneType::Stop;
    } else if (str == "shoulder") {             // 路肩
      return road::Lane::LaneType::Shoulder;
    } else if (str == "biking") {               // 自行车专用道
      return road::Lane::LaneType::Biking;
    } else if (str == "sidewalk") {             // 人行道
      return road::Lane::LaneType::Sidewalk;
    } else if (str == "border") {               // 边界，车道之间的界限
      return road::Lane::LaneType::Border;
    } else if (str == "restricted") {           // 限制
      return road::Lane::LaneType::Restricted;
    } else if (str == "parking") {              // 路边停车带
      return road::Lane::LaneType::Parking;
    } else if (str == "bidirectional") {        // 双向行驶的车道，通常为狭窄道路的情况
      return road::Lane::LaneType::Bidirectional;
    } else if (str == "median") {               // 中央分隔带
      return road::Lane::LaneType::Median;
    } else if (str == "special1") {             // 特殊1
      return road::Lane::LaneType::Special1;
    } else if (str == "special2") {             // 特殊2
      return road::Lane::LaneType::Special2;
    } else if (str == "special3") {             // 特殊3
      return road::Lane::LaneType::Special3;
    } else if (str == "roadworks") {            // 道路施工
      return road::Lane::LaneType::RoadWorks;
    } else if (str == "tram") {                 // 轻轨电车专用道
      return road::Lane::LaneType::Tram;
    } else if (str == "rail") {                 // 铁路
      return road::Lane::LaneType::Rail;
    } else if (str == "entry") {                // 入口
      return road::Lane::LaneType::Entry;
    } else if (str == "exit") {                 // 出口
      return road::Lane::LaneType::Exit;
    } else if (str == "offramp") {              // 出口匝道
      return road::Lane::LaneType::OffRamp;
    } else if (str == "onramp") {               // 入口匝道
      return road::Lane::LaneType::OnRamp;
    } else {
      return road::Lane::LaneType::None;
    }
  }

  void RoadParser::Parse(
      const pugi::xml_document &xml,
      carla::road::MapBuilder &map_builder) {

    std::vector<Road> roads;

    for (pugi::xml_node node_road : xml.child("OpenDRIVE").children("road")) {
      Road road { 0, "", 0.0, -1, 0, 0, {}, {}, {} };

      // 属性
      road.id = node_road.attribute("id").as_uint();
      road.name = node_road.attribute("name").value();
      road.length = node_road.attribute("length").as_double();
      road.junction_id = node_road.attribute("junction").as_int();

      // 连接
      pugi::xml_node link = node_road.child("link");
      if (link) {
        if (link.child("predecessor")) {
          road.predecessor = link.child("predecessor").attribute("elementId").as_uint();
        }
        if (link.child("successor")) {
          road.successor = link.child("successor").attribute("elementId").as_uint();
        }
      }

      // 类别
      for (pugi::xml_node node_type : node_road.children("type")) {
        RoadTypeSpeed type { 0.0, "", 0.0, "" };

        type.s = node_type.attribute("s").as_double();
        type.type = node_type.attribute("type").value();

        // 速度类别
        pugi::xml_node speed = node_type.child("speed");
        if (speed) {
          type.max = speed.attribute("max").as_double();
          type.unit = speed.attribute("unit").value();
        }

        // 添加它
        road.speed.emplace_back(type);
      }

      // section offsets
      for (pugi::xml_node node_offset : node_road.child("lanes").children("laneOffset")) {
        LaneOffset offset { 0.0, 0.0, 0.0, 0.0, 0.0 };
        offset.s = node_offset.attribute("s").as_double();
        offset.a = node_offset.attribute("a").as_double();
        offset.b = node_offset.attribute("b").as_double();
        offset.c = node_offset.attribute("c").as_double();
        offset.d = node_offset.attribute("d").as_double();
        road.section_offsets.emplace_back(offset);
      }
      // 如果没有找到，则添加默认的车道偏移量
      if(road.section_offsets.size() == 0) {
        LaneOffset offset { 0.0, 0.0, 0.0, 0.0, 0.0 };
        road.section_offsets.emplace_back(offset);
      }

      // lane sections
      for (pugi::xml_node node_section : node_road.child("lanes").children("laneSection")) {
        LaneSection section { 0.0, {} };

        section.s = node_section.attribute("s").as_double();

        // left lanes
        for (pugi::xml_node node_lane : node_section.child("left").children("lane")) {
          Lane lane { 0, road::Lane::LaneType::None, false, 0, 0 };

          lane.id = node_lane.attribute("id").as_int();
          lane.type = StringToLaneType(node_lane.attribute("type").value());
          lane.level = node_lane.attribute("level").as_bool();

          // link
          pugi::xml_node link2 = node_lane.child("link");
          if (link2) {
            if (link2.child("predecessor")) {
              lane.predecessor = link2.child("predecessor").attribute("id").as_int();
            }
            if (link2.child("successor")) {
              lane.successor = link2.child("successor").attribute("id").as_int();
            }
          }

          // add it
          section.lanes.emplace_back(lane);
        }

        // 中央车道
        for (pugi::xml_node node_lane : node_section.child("center").children("lane")) {
          Lane lane { 0, road::Lane::LaneType::None, false, 0, 0 };

          lane.id = node_lane.attribute("id").as_int();
          lane.type = StringToLaneType(node_lane.attribute("type").value());
          lane.level = node_lane.attribute("level").as_bool();

          // 连接(可能根本不存在)
          pugi::xml_node link2 = node_lane.child("link");
          if (link2) {
            if (link2.child("predecessor")) {
              lane.predecessor = link2.child("predecessor").attribute("id").as_int();
            }
            if (link2.child("successor")) {
              lane.successor = link2.child("successor").attribute("id").as_int();
            }
          }

          // 添加它
          section.lanes.emplace_back(lane);
        }

        // right lane
        for (pugi::xml_node node_lane : node_section.child("right").children("lane")) {
          Lane lane { 0, road::Lane::LaneType::None, false, 0, 0 };

          lane.id = node_lane.attribute("id").as_int();
          lane.type = StringToLaneType(node_lane.attribute("type").value());
          lane.level = node_lane.attribute("level").as_bool();

          // link
          pugi::xml_node link2 = node_lane.child("link");
          if (link2) {
            if (link2.child("predecessor")) {
              lane.predecessor = link2.child("predecessor").attribute("id").as_int();
            }
            if (link2.child("successor")) {
              lane.successor = link2.child("successor").attribute("id").as_int();
            }
          }

          // 添加它
          section.lanes.emplace_back(lane);
        }

        // add section
        road.sections.emplace_back(section);
      }

      // 添加路
      roads.emplace_back(road);
    }

    // test print
    /*
       printf("Roads: %d\n", roads.size());
       for (auto const r : roads) {
       printf("Road: %d\n", r.id);
       printf("  Name: %s\n", r.name.c_str());
       printf("  Length: %e\n", r.length);
       printf("  JunctionId: %d\n", r.junction_id);
       printf("  Predecessor: %d\n", r.predecessor);
       printf("  Successor: %d\n", r.successor);
       printf("  Speed: %d\n", r.speed.size());
       for (auto const s : r.speed) {
        printf("    S offset: %e\n", s.s);
        printf("    Type: %s\n", s.type.c_str());
        printf("    Max: %e\n", s.max);
        printf("    Unit: %s\n", s.unit.c_str());
       }
       printf("LaneSections: %d\n", r.sections.size());
       for (auto const s : r.sections) {
        printf("    S offset: %e\n", s.s);
        printf("    a,b,c,d: %e,%e,%e,%e\n", s.a, s.b, s.c, s.d);
        printf("    Lanes: %d\n", s.lanes.size());
        for (auto const l : s.lanes) {
          printf("      Id: %d\n", l.id);
          printf("      Type: %s\n", l.type.c_str());
          printf("      Level: %d\n", l.level);
          printf("      Predecessor: %d\n", l.predecessor);
          printf("      Successor: %d\n", l.successor);
        }
       }
       }
     */

    // 调用地图构建器 map_builder 
    for (auto const r : roads) {
      carla::road::Road *road = map_builder.AddRoad(r.id,
          r.name,
          r.length,
          r.junction_id,
          r.predecessor,
          r.successor);

      // type speed
      for (auto const s : r.speed) {
        map_builder.CreateRoadSpeed(road, s.s, s.type, s.max, s.unit);
      }

      // section offsets
      for (auto const s : r.section_offsets) {
        map_builder.CreateSectionOffset(road, s.s, s.a, s.b, s.c, s.d);
      }

      // lane sections
      road::SectionId i = 0;
      for (auto const s : r.sections) {
        carla::road::LaneSection *section = map_builder.AddRoadSection(road, i++, s.s);

        // lanes
        for (auto const l : s.lanes) {
          /*carla::road::Lane *lane = */ map_builder.AddRoadSectionLane(section, l.id,
              static_cast<uint32_t>(l.type), l.level, l.predecessor, l.successor);
        }
      }
    }
  }

} // namespace parser
} // namespace opendrive
} // namespace carla
