// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/opendrive/parser/ProfilesParser.h"

#include "carla/road/MapBuilder.h"

#include <pugixml/pugixml.hpp>

namespace carla {
namespace opendrive {
namespace parser {

  struct ElevationProfile {
    carla::road::Road *road { nullptr };
    double s            { 0.0 };
    double a            { 0.0 };
    double b            { 0.0 };
    double c            { 0.0 };
    double d            { 0.0 };
  };

  struct LateralCrossfall {
    std::string side      { "both" };
  };

  struct LateralShape {
    double t      { 0.0 };
  };

  struct LateralProfile {
    carla::road::Road *road { nullptr };
    double s            { 0.0 };
    double a            { 0.0 };
    double b            { 0.0 };
    double c            { 0.0 };
    double d            { 0.0 };
    std::string type    { "superelevation" };
    LateralCrossfall cross;
    LateralShape shape;
  };

  void ProfilesParser::Parse(
      const pugi::xml_document &xml,
      carla::road::MapBuilder &map_builder) {

    std::vector<ElevationProfile> elevation_profile;
    std::vector<LateralProfile> lateral_profile;

    for (pugi::xml_node node_road : xml.child("OpenDRIVE").children("road")) {

      // 解析高程剖面
      pugi::xml_node node_profile = node_road.child("elevationProfile");
      uint64_t number_profiles = 0;
      if (node_profile) {
        // 所有的几何
        for (pugi::xml_node node_elevation : node_profile.children("elevation")) {
          ElevationProfile elev;

          // 获取道路 id
          road::RoadId road_id = node_road.attribute("id").as_uint();
          elev.road = map_builder.GetRoad(road_id);

          // 获取常用属性
          elev.s = node_elevation.attribute("s").as_double();
          elev.a = node_elevation.attribute("a").as_double();
          elev.b = node_elevation.attribute("b").as_double();
          elev.c = node_elevation.attribute("c").as_double();
          elev.d = node_elevation.attribute("d").as_double();

          // 添加它
          elevation_profile.emplace_back(elev);
          number_profiles++;
        }
      }
      // 如果没有找到，则添加默认配置
      if(number_profiles == 0){
        ElevationProfile elev;
        road::RoadId road_id = node_road.attribute("id").as_uint();
        elev.road = map_builder.GetRoad(road_id);

        // 获取常用属性
        elev.s = 0;
        elev.a = 0;
        elev.b = 0;
        elev.c = 0;
        elev.d = 0;

        // 添加它
        elevation_profile.emplace_back(elev);
      }

      // 解析横向剖面
      node_profile = node_road.child("lateralProfile");
      if (node_profile) {
        for (pugi::xml_node node : node_profile.children()) {
          LateralProfile lateral;

          // 获取路 id
          road::RoadId road_id = node_road.attribute("id").as_uint();
          lateral.road = map_builder.GetRoad(road_id);

          // 获取常用属性
          lateral.s = node.attribute("s").as_double();
          lateral.a = node.attribute("a").as_double();
          lateral.b = node.attribute("b").as_double();
          lateral.c = node.attribute("c").as_double();
          lateral.d = node.attribute("d").as_double();

          // 处理不同类型
          lateral.type = node.name();
          if (lateral.type == "crossfall") {
            lateral.cross.side = node.attribute("side").value();
          } else if (lateral.type == "shape") {
            lateral.shape.t = node.attribute("t").as_double();
          }

          // add it
          lateral_profile.emplace_back(lateral);
        }
      }
    }

    // map_builder calls
    for (auto const pro : elevation_profile) {
      map_builder.AddRoadElevationProfile(pro.road, pro.s, pro.a, pro.b, pro.c, pro.d);
    }
    /// @todo: RoadInfo classes must be created to fit this information
    // for (auto const pro : lateral_profile) {
    //   if (pro.type == "superelevation")
    //     map_builder.AddRoadLateralSuperElevation(pro.road, pro.s, pro.a,
    // pro.b, pro.c, pro.d);
    //   else if (pro.type == "crossfall")
    //     map_builder.AddRoadLateralCrossfall(pro.road, pro.s, pro.a, pro.b,
    // pro.c, pro.d, pro.cross.side);
    //   else if (pro.type == "shape")
    //     map_builder.AddRoadLateralShape(pro.road, pro.s, pro.a, pro.b, pro.c,
    // pro.d, pro.shape.t);
    // }
  }

} // namespace parser
} // namespace opendrive
} // namespace carla
